#include "Halide.h"
using namespace Halide;


Func operator-(Func image1, Func image2);
Func operator+(Func image1, Func image2);
Func operator*(Func image1, Func image2);
Func operator/(Func image1, Func image2);
Func operator+(Func image, Expr a);
Func operator-(Func image, Expr a);
Func operator*(Func image, Expr a);
Func operator/(Func image, Expr a);
Func operator+(Expr a, Func image);
Func operator-(Expr a, Func image);
Func operator*(Expr a, Func image);
Func operator/(Expr a, Func image);
// Func median(Func image, int width, int height);
Func abs(Func image);
Func changeGamma(Func image, float oldGamma, float newGamma);
Func exposure(Func image, float factor, float gamma=2.2);
Func brightness(Func image, float factor);
Func contrast(Func image, float factor, float midpoint = 0.5);
Func color2gray(Func image, float wr = 0.299, float wg = 0.587, float wb0 = 0.114);
Func rgb2yuv(Func image);
Func yuv2rgb(Func image);
Func saturate(Func image, float factor);
std::vector<Func> lumiChromi(Func image, float wr = 0.299, float wg = 0.587, float wb0 = 0.114);
