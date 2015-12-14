#include <stdio.h>
#include "stencil.h"
#include "util.h"
#include "schedule.h"
#include "point.h"
#include "apps.h"

int main(int argc, char ** argv)
{
  if (argc < 3)
  {
    printf("USAGE: ./perfStyleChange input_image model_image");
  }
  Image<float> input_image = load_image(argv[1]);
  Image<float> model_image = load_image(argv[2]);
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
  schedule_compute_root(model);
  schedule_compute_root(input);
  schedule_compute_root(output);
  Target t = get_jit_target_from_environment().with_feature(Target::Profile);
  output.compile_jit();
  Image<float> output_buf = clamp(output,0,1).realize(input_image.width(), input_image.height(), t);
  save_image(output_buf, "output.png");
}
