#include "image.h"

using namespace Halide;
using namespace Halide::Tools;

void TestImageIO()
{
  ImageWrapper image("../images/rgb.png");
  image.write("output/rgb.png");
}

int main(int argc, char** argv)
{
  TestImageIO();
}
