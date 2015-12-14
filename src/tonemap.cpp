#include "Halide.h"
#include "point.h"
#include "stencil.h"
#include "util.h"
#include "schedule.h"
#include "global.h"

using namespace Halide;
Func nonzeroMin(Func image, int height, int width)
{
  Var x;
  RDom r(0, height, 0, width);
  Func min;
  min(x) = image(0,0);
  min(0) = select(image(r.x,r.y)>0 && image(r.x,r.y)<min(0), image(r.x,r.y), min(0));
  return min;
}

Func logImage(Func image, int height, int width)
{
  Var x,y;
  Func min = nonzeroMin(image, height, width);
  Func output;
  output(x,y) = select(image(x,y)==0, log(min(0)), log(image(x,y)));
  return output;
}

Func exp(Func image)
{
  Var x, y;
  Func output;
  output(x,y) = exp(image(x,y));
  return output;
}

Func toneMap(Func image, int width, int height, float initContrast, float targetContrast, float detailAmp, float sigmaRange, bool useGrid)
{
  Var x("x"),y("y"),c("c");
  Func lumi = color2gray(image, 0.4, 0.7, 0.01);
  // writeImage(lumi, "output/lumi.png", width, height);
  Func logLumi = clampDomain(logImage(lumi, width, height), width-1, height-1);
  int sigmaDomain = ceil(0.02*(width > height ? width : height));
  Func baseLumi;
  if (useGrid)
  {
    Func minMaxLog = minMax(logLumi, width, height);
    Func normalized;
    normalized(x,y) = (logLumi(x,y) - minMaxLog(0))/(minMaxLog(1) - minMaxLog(0));
    Func filtered = bilateralGrid(normalized, sigmaRange/log(initContrast), sigmaDomain);
    baseLumi(x,y) = filtered(x,y) * (minMaxLog(1) - minMaxLog(0)) + minMaxLog(0);
    // writeImage(exp(baseLumi), "output/hdr/baseLumiGrid.png", width, height);
  }
  else
  {
    int k = int(ceil(3.0*sigmaDomain));
    baseLumi = bilateralFilter(logLumi, sigmaRange, sigmaDomain);
    // writeImage(exp(baseLumi), "output/hdr/baseLumi.png", width, height);
  }

  Func detailLumi("detailLumi");
  detailLumi(x,y) = logLumi(x,y) - baseLumi(x,y);
  // writeImage(exp(detailLumi), "output/detailLumi.png", width, height);
  Func minMaxBase = minMax(baseLumi, width, height);
  Func scaleFactor;
  scaleFactor(x) = float(log(targetContrast))/(minMaxBase(1)-minMaxBase(0));
  // apply_default_schedule(scaleFactor);
  // Image<float> temp = scaleFactor.realize(1);
  // std::cout << temp(0) << "\n";
  Func newLogLumi("newLogLumi");
  newLogLumi(x,y) = detailLumi(x,y) * detailAmp + (baseLumi(x,y)-minMaxBase(1)) * scaleFactor(0);
  Func outLumi("outLumi");
  outLumi(x,y) = exp(newLogLumi(x,y));
  // writeImage(outLumi, "output/outLumi.png", width, height);
  Func output("output");
  output(x,y,c) = image(x,y,c)/lumi(x,y)*outLumi(x,y);
  return changeGamma(output, 1, 1/2.2f);
}
