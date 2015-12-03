#include "Halide.h"
using namespace Halide;

Func demosaic(Func raw, int offsetGreen=1, int offsetRedX=1, int offsetRedY=1, int offsetBlueX=0, int offsetBlueY=0);
Func computeWeight(Func images, float epsilonMin, float epsilonMax, int k);
Func computeFactors(Func images, Func weights, int width, int height);
Func mkhdr(Func images, float epsilonMin, float epsilonMax, int width, int height, int k);
Func toneMap(Func image, int height, int width, float initContrast, float targetContrast, float detailAmp, float sigmaRange, bool useGrid=true);
