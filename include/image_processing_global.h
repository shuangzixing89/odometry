// Created by Yu Wang on 04.12.18.
// The header file contains global function for different kinds of image processing, such as:
// image smoothing, image gradient, image pyramids, warping, etc.

// The functions can be used by all other classes and functions, unless you need to
// access private data members or member functions from some class, you need to explicitly declare the function as
// a global friend function within the class although it is highly unrecommended.

#ifndef ODOMETRY_IMAGE_PROCESSING_GLOBAL_H
#define ODOMETRY_IMAGE_PROCESSING_GLOBAL_H

#include <Eigen/Core>
#include <opencv2/core.hpp>
#include <data_types.h>
#include <camera.h>
#include <memory>

namespace odometry
{

// inlined function to re-project pixel-coord to current camera's 3d coord, assuming a valid depth value!
inline void ReprojectToCameraFrame(const Vector4f& kIn_coord, const std::shared_ptr<Camera>& kCameraPtr, Vector4f& out_3d){
  out_3d(0) = kIn_coord(2) * (kIn_coord(0) - kCameraPtr->cx_) / kCameraPtr->fx_;
  out_3d(1) = kIn_coord(2) * (kIn_coord(1) - kCameraPtr->cy_) / kCameraPtr->fy_;
  out_3d(2) = kIn_coord(2);
  out_3d(3) = 1.0;
}

// inlined function to warp a single pixel, use Vector4X for sake of vectorization
inline GlobalStatus WarpPixel(const Vector4f& kIn_3d, const Matrix44f& kTranform, int Height, int Width, const std::shared_ptr<Camera>& kCameraPtr, Vector4f& out_coord){
  Vector4f tmp = kTranform * kIn_3d;
  out_coord(0) = kCameraPtr->fx_ * tmp(0) / tmp(2) + kCameraPtr->cx_;
  out_coord(1) = kCameraPtr->fy_ * tmp(1) / tmp(2) + kCameraPtr->cy_;
  out_coord(2) = tmp(2);
  out_coord(3) = 1.0;
  if (std::floor(out_coord(0)) >= float(Width) || std::floor(out_coord(1)) >= float(Height)
      || std::floor(out_coord(0)) < 0.0 || std::floor(out_coord(1)) < 0.0) // out of image boundary
    return -1;
  else
    return 0;
}
// TODO: using native c++ for loop combined with openmp to warp entire image
void WarpImageNative(const cv::Mat& img_in, const Matrix44f& kTransMat, cv::Mat& warped_img);
// using sse to warp entire image
void WarpImageSse(const cv::Mat& img_in, const Matrix44f& kTransMat, cv::Mat& warped_img);

// TODO: compute gaussian pyramid and save the value, the out_pyramids is not initialised. NOTE NOTE NOTE smoothing on all pyramid levels
// return status: -1 failed, otherwise success
GlobalStatus GaussianImagePyramid(int num_levels, const cv::Mat& in_img, std::vector<cv::Mat>& out_pyramids);

// compute pixel gradient given image and coordinate using central difference
inline void ComputePixelGradient(const cv::Mat& kImg, int Height, int Width, int y, int x, RowVector2f& grad){
  int pre_x = (x-1 >= 0) ? (x-1) : 0;
  int next_x = (x+1 < Width) ? x+1 : Width-1;
  int pre_y = (y-1 >= 0) ? (y-1) : 0;
  int next_y = (y+1 < Height) ? y+1 : Height-1;
  grad(0) = 0.5f * (kImg.at<float>(next_x, y)- kImg.at<float>(pre_x, y));
  grad(1) = 0.5f * (kImg.at<float>(x, next_y) - kImg.at<float>(x, pre_y));
}


} // namespace odometry

#endif //ODOMETRY_IMAGE_PROCESSING_GLOBAL_H
