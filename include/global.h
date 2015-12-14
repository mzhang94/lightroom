#include "Halide.h"
using namespace Halide;

Func sum(Func image, int width, int height, bool channelwise=false);
Func min(Func image, int width, int height, bool channelwise=false);
Func max(Func image, int width, int height, bool channelwise=false);
Func minMax(Func image, int width, int height, bool channelwise=false);
Expr findBin(Expr val, float minVal, float maxVal, int nbins);
Func histogram(Func image, int width, int height, std::string name, float minVal=0.0f, float maxVal=1.0f, int nbins=256);
Func cumulate(Func hist, int nbins, std::string name);
Func dot(Func im1, Func im2, int width, int height);
