#include "Halide.h"
using namespace Halide;

Func sum(Func image, int width, int height, bool channelwise=false);
Func min(Func image, int width, int height, bool channelwise=false);
Func max(Func image, int width, int height, bool channelwise=false);
Func minMax(Func image, int width, int height, bool channelwise=false);
Func substract(Func image1, Func image2);
Func add(Func image1, Func image2);
Func multiply(Func image1, Func image2);
Func divide(Func image1, Func image2);
Func add(Func image, Expr a);
Func multiply(Func image, Expr a);
// Func median(Func image, int width, int height);
Func changeGamma(Func image, float oldGamma, float newGamma);
Func exposure(Func image, float factor, float gamma=2.2);
Func brightness(Func image, float factor);
Func contrast(Func image, float factor, float midpoint = 0.5);
Func color2gray(Func image, float wr = 0.299, float wg = 0.587, float wb0 = 0.114);
Func rgb2yuv(Func image);
Func yuv2rgb(Func image);
Func saturate(Func image, float factor);
std::vector<Func> lumiChromi(Func image, float wr = 0.299, float wg = 0.587, float wb0 = 0.114);
