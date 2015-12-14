#include "stencil.h"
#include "global.h"
#include "util.h"
#include "point.h"
#include "schedule.h"

//debug helper function
template<typename T>
void printFunc(Func f, int range)
{
  std::cout << f.name() << '\n';
  apply_default_schedule(f);
  Image<T> output = f.realize(range);

  for (int i = 0; i < range; i++)
  {
    std::cout << output(i) << " ";
  }
  std::cout << "\n";
}

void getSigma(Func input, int width, int height, float& sigmaRange, int& sigmaDomain)
{
  sigmaDomain = ceil(0.05*(width < height ? width : height));
  Func gradI = gradient(input, SOBEL);
  Var x("x"), y("y");
  Func gradMag("gradMag");
  gradMag(x,y) = sqrt(pow(gradI(x,y)[0],2) + pow(gradI(x,y)[1],2))/4;
  Func gradHist = histogram(gradMag, width, height, "gradHist", 0, float(sqrt(2)), 256);
  Func cumuGradHist = cumulate(gradHist, 256, "cumulate");
  Image<float> cumuGradHist_buf = cumuGradHist.realize(256);
  for (int i = 255; i >= 0; i--)
  {
    if (cumuGradHist_buf(i) <= 0.9)
    {
      sigmaRange = i*sqrt(2)/255;
      break;
    }
  }
  printf("sigmaRange %f sigmaDomain %d\n", sigmaRange, sigmaDomain);
}

Func histMatch(Func model, Func input, int modelW, int modelH, int inputW, int inputH, std::string name, float minVal, float maxVal, int nbins)
{
  Var x("x"), y("y");

  Func model_hist = histogram(model, modelW, modelH, "model_hist", minVal, maxVal, nbins);
  Func input_hist = histogram(input, inputW, inputH, "input_hist", minVal, maxVal, nbins);

  Func model_cumu = cumulate(model_hist, nbins, "model_cumu");
  Func input_cumu = cumulate(input_hist, nbins, "input_cumu");

  Func model_equalize("model_equalize");
  Func input_equalize("input_equalize");
  model_equalize(x) = findBin(model_cumu(x), 0, 1, nbins);
  input_equalize(x) = findBin(input_cumu(x), 0, 1, nbins);

  // printFunc<float>(model_cumu, nbins);
  // printFunc<float>(input_cumu, nbins);
  // printFunc<int>(model_equalize, nbins);
  // printFunc<int>(input_equalize, nbins);

  Func model_inv("model_inv");
  model_inv(x) = 0;
  //resolve this ambiguity by choosing the smallest value for the inverse mapping
  RDom r(0, nbins);
  model_inv(model_equalize(nbins-1-r)) = nbins-1-r;
  RDom r1(1, nbins-1);
  model_inv(r1) = select(model_inv(r1) < model_inv(r1-1), model_inv(r1-1), model_inv(r1));

  // printFunc<int>(model_inv, nbins);

  Func match("match");
  match(x) = model_inv(input_equalize(x));

  Func output(name);
  output(x,y) = match(findBin(input(x,y), minVal, maxVal, nbins))/(nbins-1.0f) * (maxVal-minVal) + minVal;
  return output;
}

Func textureness(Func input, float sigmaRange, int sigmaDomain)
{
  Var x("x"), y("y");
  Func lowpass = boxBlur(input, sigmaDomain);
  Func highpassMag("highpassMag");
  highpassMag(x,y) = abs(input(x,y) - lowpass(x,y));

  return crossBilateral(input, highpassMag, sigmaRange, 8*sigmaDomain);
}

Func poisson(Func input, Func guidanceField, int width, int height, int niter)
{
  writeImage(clamp(contrast(input,3,-0.2),0,1), "detail.png", width, height);
  Func div("div");
  Var x("x"), y("y");
  div(x,y) = -(guidanceField(x,y)[0] - guidanceField(x-1,y)[0] + guidanceField(x,y)[1] - guidanceField(x,y-1)[1]);
  for (int i = 0; i < niter; i++)
  {
    Func update("update-" + std::to_string(i));
    //laplacian filter is atually negative
    Func delta = div - laplacianFilter(input);
    // writeImage(clamp(Expr(5)*delta,0,1), "delta-" + std::to_string(i+1) + ".png", width, height);
    Func steppingFactor = dot(delta, delta, width, height)/dot(delta, laplacianFilter(delta), width, height);
    update(x,y) = input(x,y) + steppingFactor(0) * delta(x,y);
    // writeImage(clamp(Expr(5)*update,0,1), "detail_corrected-" + std::to_string(i+1) + ".png", width, height);
    input = update;
  }
  writeImage(clamp(contrast(input, 3,-0.2),0,1), "detail_corrected.png", width, height);
  return input;
}

Func getCorrectedDetail(Func input, int width, int height, float sigmaRange, int sigmaDomain, std::string name)
{
  Var x("x"), y("y");
  Func base = bilateralGrid(input, sigmaRange, sigmaDomain);
  Func detail("detail");
  detail(x,y) = input(x,y) - base(x,y);

  Func gradient_corrected("gradient_correct");
  Expr dddx = detail(x+1,y) - detail(x,y);
  Expr dddy = detail(x,y+1) - detail(x,y);
  Expr didx = input(x+1,y) - input(x,y);
  Expr didy = input(x,y+1) - input(x,y-1);
  gradient_corrected(x,y) = {
                            select(didx*dddx<0, 0,                // if different sign
                            select(abs(dddx) > abs(didx), didx,   // if dD/dx > dI/dx
                            dddx)),                               // otherwise
                            select(didy*dddy<0, 0,                // if different sign
                            select(abs(dddy) > abs(didy), didy,   // if dD/dy > dI/dy
                            dddy))};                              // otherwise
  return poisson(detail, gradient_corrected, width, height, 200);
}

Func styleChange(Func model, Func input,
                 int modelW, int modelH, int inputW, int inputH,
                 float inputSigmaRange, int inputSigmaDomain, float modelSigmaRange, int modelSigmaDomain,
                 std::string name)
{
  Var x("x"), y("y");
  Func base = bilateralGrid(input, inputSigmaRange, inputSigmaDomain);
  Func detail = input - base;
  Func modelBase = bilateralGrid(model, modelSigmaRange, modelSigmaDomain);

  //tonal balance
  Func modifiedBase = histMatch(modelBase, base, modelW, modelH, inputW, inputH, "modifies_base", 0, 1, 256);

  //detail
  Func modelTextureness = textureness(model, inputSigmaRange, inputSigmaDomain);
  Func inputTextureness = textureness(input, inputSigmaRange, inputSigmaDomain);
  Func baseTextureness = textureness(modifiedBase, inputSigmaRange, inputSigmaDomain);
  Func detailTextureness = textureness(detail, inputSigmaRange, inputSigmaDomain);
  Func textureMap = histMatch(modelTextureness, inputTextureness, modelW, modelH, inputW, inputH, "textureMap", 0, 1, 5000);

  Func amplifyRatio("amplifyRatio");
  amplifyRatio(x,y) = max(0, (textureMap(x,y) - baseTextureness(x,y))/detailTextureness(x,y));
  Func output("before_postprocess");
  output(x,y) = modifiedBase(x,y) + amplifyRatio(x,y) * detail(x, y);

  //detail match the quick pass
  // Func detailMatch = histMatch(abs(modelDetail), abs(detail), modelW, modelH, inputW, inputH, "detail_match", 0, 0.5, 1000);
  // writeImage(detailMatch, "detailMatch.png", inputW, inputH);
  // writeImage(abs(modelDetail), "modelDetail.png", modelW, modelH);
  // writeImage(abs(detail), "detail.png", inputW, inputH);
  // Func modifiedDetail("modifiedDetail");
  // modifiedDetail(x,y) = select(detail(x,y) > 0, detailMatch(x,y), -detailMatch(x,y));
  // writeImage(modifiedDetail + Expr(0.1f), "modifiedDetail.png", inputW, inputH);
  // Func output(name);
  // output(x,y) = modifiedDetail(x,y) + modifiedBase(x,y);
  // writeImage(output, "output.png", inputW, inputH);
  return histMatch(model, output, modelW, modelH, inputW, inputH, name, 0, 1.1f, 256);
}
