#include "stencil.h"
#include "util.h"
#include "schedule.h"

int main(int argc, char ** argv)
{
  Halide::Image<float> input = load_image("../images/rgb.png");
  Func image = readImage(input, true);

  Func blur = gaussianFilter(image, 3.0, 3);

  blur.compile_to_lowered_stmt("gaussian.html", std::vector<Argument>(), HTML);
  profile(blur, input.width(), input.height(), input.channels());
}
