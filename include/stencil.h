#include "Halide.h"
using namespace Halide;

enum INTERPOLATE_METHOD
{
  NN, BILIN, BICUBIC
};

enum GRADIENT_KERNEL
{
  SOBEL, PREWITT
};

Expr interpolate(Func image, Expr x, Expr y, Expr x1, Expr y1, Expr x2, Expr y2, Expr c, INTERPOLATE_METHOD method);
Func scale(Func image, float factorX, float factorY, INTERPOLATE_METHOD method);
Func gradient(Func image, GRADIENT_KERNEL kernel);
Func convolveX(Func image, Func kernel, int halfSize);
Func convolveY(Func image, Func kernel, int halfSize);
Func convolve2d(Func image, Func kernel, int halfWidth, int halfHeight);
Func boxBlur(Func image, int halfWindowSize);
Expr gaussian(Expr x, Expr y, float sigma);
Func gaussianKernel1d(float sigma, int halfSize = 0);
Func gaussianKernel2d(float sigma, int halfSize = 0);
Func gaussianFilter(Func image, float sigma, int halfWindowSize);
Func laplacianFilter(Func image);
Func bilateralFilter(Func image, float sigmaRange=0.1f, float sigmaDomain=1.0f, int hSize=3);
Func bilateralFilterYUV(Func image, float sigmaRange=0.1f, float sigmaY=1.0f, float sigmaUV=4.0f, float truncateDomain=3.0f);
Func bilateralGrid(Func image, float sigmaRange=0.1f, int sigmaDomain=1.0, float minVal=0.0f, float maxVal=1.0f);
Func crossBilateral(Func gridIm, Func filterIm, float sigmaRange, int sigmaDomain, float minVal=0.0f, float maxVal=1.0f);
Func demosaic(Func image);
