#include "stencil.h"
#include "util.h"
#include "schedule.h"

int main(int argc, char ** argv)
{
  Halide::Image<float> input = load_image("../images/rgb.png");
  Func image = readImage(input, true);

  Func kernel = gaussianKernel2d(3.0, 3);
  Func blur1 = convolve2d(image, kernel, 3, 3);
  Func blur2 = gaussianFilter(image, 3.0, 3);

  apply_default_schedule(blur1);
  apply_default_schedule(blur2);
  verifyFunc(blur1, blur2, input.width(), input.height());
  Halide::Image<float> output =
      blur2.realize(input.width(), input.height(), input.channels());

  save_image(output, "output/testGaussian.png");
}
