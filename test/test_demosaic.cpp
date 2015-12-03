#include "Halide.h"
#include "util.h"
#include "apps.h"
#include "schedule.h"
using namespace Halide;
using namespace Halide::Tools;

int main(int argc, char ** argv)
{
  Halide::Image<float> input = load_image("../images/signs-small.png");
  Func image = readImage(input, true);
  Func full = clamp(demosaic(image), 0, 1);
  apply_default_schedule(full);
  Halide::Image<float> output(input.width()-2, input.height()-2, 3);
  output.set_min(1,1);
  full.realize(output);
  save_image(output, "output/testDemosaic.png");
}
