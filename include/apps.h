#include "Halide.h"
using namespace Halide;

Func demosaic(Func raw, int offsetGreen=1, int offsetRedX=1, int offsetRedY=1, int offsetBlueX=0, int offsetBlueY=0);

//mkhdr
Func computeWeight(Func images, float epsilonMin, float epsilonMax, int k);
Func computeFactors(Func images, Func weights, int width, int height);
Func mkhdr(Func images, float epsilonMin, float epsilonMax, int width, int height, int k);
//toneMap
Func toneMap(Func image, int height, int width, float initContrast, float targetContrast, float detailAmp, float sigmaRange, bool useGrid=true);

//styleChange
void getSigma(Func input, int width, int height, float& sigmaRange, int& sigmaDomain);
Func textureness(Func input, float sigmaRange, int sigmaDomain);
Func histMatch(Func model, Func input, int modelW, int modelH, int inputW, int inputH, std::string name, float minVal=0.0f, float maxVal=1.0f, int nbins=256);
Func getCorrectedDetail(Func input, int width, int height, float sigmaRange, int sigmaDomain, std::string name);
Func styleChange(Func model, Func input, int modelW, int modelH, int inputW, int inputH,
  float inputSigmaRange, int inputSigmaDomain, float modelSigmaRange, int modelSigmaDomain, std::string name);
