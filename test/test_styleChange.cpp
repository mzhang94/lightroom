#include "Halide.h"
#include "halide_image_io.h"
#include "util.h"
#include "stencil.h"
#include "schedule.h"
#include "apps.h"
#include "point.h"
#include "global.h"
#include <stdio.h>
using namespace Halide;
using namespace Halide::Tools;

void TestHistgram()
{
  Image<float> input_image = load_image("../images/rock.png");
  Func input = color2gray(readImage(input_image, false));
  Func hist = histogram(input, input_image.width(), input_image.height(), "hist");

  Image<float> output = hist.realize(256);

  for (int i = 0; i < 256; i++)
  {
    std::cout << output(i) << ' ';
  }
    std::cout << '\n';
}
void TestHistMatch()
{
  Image<float> model_image = load_image("../images/cws.png");
  Image<float> input_image = load_image("../images/rock.png");

  Func model = color2gray(readImage(model_image, false));
  Func input = color2gray(readImage(input_image, false));

  Func output = histMatch(model, input, model_image.width(), model_image.height(),
                          input_image.width(), input_image.height(), "output", 0, 2, 1000);
  writeImage(output, "output/testHistMatch.png", input_image.width(), input_image.height());
}


void TestGetDetail()
{
  Image<float> input_image = load_image("../images/rock.png");
  Func input = color2gray(readImage(input_image));

  float sigmaRange;
  int sigmaDomain;
  getSigma(input, input_image.width(), input_image.height(), sigmaRange, sigmaDomain);
  Func detail = getCorrectedDetail(input, input_image.width(), input_image.height(), sigmaRange*2, sigmaDomain, "corrected_detail");
  apply_default_schedule(detail);
  detail.compile_to_lowered_stmt("correctted_detail.html", {}, HTML);
  writeImage(detail, "output/testGetDetail.png", input_image.width(), input_image.height());
}

void TestTextureness()
{
  Image<float> input_image = load_image("../images/rock.png");
  Func input = color2gray(readImage(input_image));
  float sigmaRange;
  int sigmaDomain;
  getSigma(input, input_image.width(), input_image.height(), sigmaRange, sigmaDomain);
  Func base = bilateralGrid(input, sigmaRange, sigmaDomain);
  Func detail = input - base;

  std::cout << input_image.width() << " " << input_image.height() << "\n";
  Func texture = textureness(detail, sigmaRange, sigmaDomain);
  writeImage(Expr(5)*texture, "output/testTextureness.png", input_image.width(), input_image.height());
}

void TestStyleChange()
{
  Image<float> input_image = load_image("../images/flower.png");
  Image<float> model_image = load_image("../images/train.png");
  Func input = color2gray(readImage(input_image));
  Func model = color2gray(readImage(model_image));
  float inputSigmaRange;
  int inputSigmaDomain;
  float modelSigmaRange;
  int modelSigmaDomain;
  getSigma(input, input_image.width(), input_image.height(), inputSigmaRange, inputSigmaDomain);
  getSigma(model, model_image.width(), model_image.height(), modelSigmaRange, modelSigmaDomain);

  Func output = styleChange(model, input, model_image.width(), model_image.height(),
                            input_image.width(), input_image.height(), inputSigmaRange, inputSigmaDomain,
                            modelSigmaRange, modelSigmaDomain, "output");
  apply_default_schedule(output);
  // Target t = get_jit_target_from_environment().with_feature(Target::Profile);
  Image<float> output_buf = clamp(output,0,1).realize(input_image.width(), input_image.height());
  save_image(output_buf, "output/testStyleChange.png");
}

int main(int argc, char ** argv)
{
  // TestHistgram();
  // TestHistMatch();
  // TestTextureness();
  // TestGetDetail();
  TestStyleChange();
}
