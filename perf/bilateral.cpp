#include "stencil.h"
#include "util.h"
#include "schedule.h"
#include "point.h"

int main(int argc, char ** argv)
{
  Halide::Image<float> input = load_image("../images/lens.png");
  Func image = readImage(input, true);
  Func gray = clampDomain(color2gray(image), input.width(), input.height());

  Func grayGrid = clamp(bilateralGrid(gray), 0, 1);
  //apply_default_schedule(grayGrid);
  //output = grayGrid.realize(input.width(), input.height());
  //save_image(output, "output/testBilateralGrid.png");

  grayGrid.compile_to_lowered_stmt("bilateral.html", std::vector<Argument>(), HTML);
  profile(grayGrid, input.width(), input.height());
}
