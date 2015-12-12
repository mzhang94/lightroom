#include "stencil.h"
#include "util.h"
#include "schedule.h"
#include "point.h"
#include "apps.h"

int main(int argc, char ** argv)
{
  int numOfImages = atoi(argv[2]);
  Func images("images");
  int width;
  int height;
  Var x, y, c, i;
  images(x,y,c,i) = 0.0f;
  for (int i = 0; i < numOfImages; i++){
    Image<float> image = load_image("../images/hdr_input/" + std::string(argv[1]) + "-" + std::to_string(i+1) + ".png");
    width = image.width();
    height = image.height();
    images(x,y,c,i) = image(x,y,c);
  }

  Func hdr = mkhdr(images, 0.002, 0.99, width, height, numOfImages);
  // Halide::Image<float> input = load_image("../images/rgb.png");
  // Func image = readImage(input, true);
  // int width = input.width();
  // int height = input.height();

  Func toneMapped= clamp(toneMap(hdr, width, height, pow(20, numOfImages), 100, 3, 0.1), 0, 1);
  // toneMapped.compute_at(output, y);
  // output.compute_root().reorder(c, x, y).parallel(y).vectorize(x,16);
  apply_default_schedule(toneMapped);

  // apply_default_schedule(hdr);
  // profile(hdr, width, height, 3);
  Target t = get_jit_target_from_environment().with_feature(Target::Profile);
  Image<float> output_buf = toneMapped.realize(width, height, 3, t);
  toneMapped.compile_to_lowered_stmt("tonemap.html", {}, HTML);
  save_image(output_buf, "output.png");
}
