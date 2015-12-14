#include "Halide.h"
#include "point.h"
#include "util.h"
#include "global.h"
using namespace Halide;
using namespace Halide::Tools;

void TestChangeGamma()
{
  Halide::Image<float> input = load_image("../images/rgb.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = clamp(changeGamma(image, 2.2, 1), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/rgb_changeGamma.png");
}

void TestExposure()
{
  Halide::Image<float> input = load_image("../images/Boston_underexposed.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = clamp(exposure(image, 3), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/boston_testExposure.png");
}

void TestBrightness()
{
  Halide::Image<float> input = load_image("../images/Boston_underexposed.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = clamp(brightness(image, 2), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/boston_testBrightness.png");
}

void TestContrast()
{
  Halide::Image<float> input = load_image("../images/Boston_underexposed.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = clamp(contrast(image, 2, 0.5f), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/boston_testContrast.png");
}

void TestColor2Gray()
{
  Halide::Image<float> input = load_image("../images/rgb.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = color2gray(image);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height());
  save_image(output, "output/testGray.png");
}

void TestYuvRgb()
{
  Halide::Image<float> input = load_image("../images/rgb.png");

  Var x,y,c;
  Func image;

  image(x,y,c) = input(x,y,c);

  Func image_yuv = rgb2yuv(image);
  Func new_image = clamp(yuv2rgb(image_yuv), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/testYuvRgb.png");
}

void TestSaturate()
{
  Halide::Image<float> input = load_image("../images/rgb.png");

  Var x, y, c;
  Func image;
  image(x,y,c) = input(x,y,c);

  Func new_image = clamp(saturate(image, 2), 0.0f, 1.0f);

  Halide::Image<float> output =
      new_image.realize(input.width(), input.height(), input.channels());

  save_image(output, "output/testSaturate.png");
}

void TestSum()
{
  Var x, y, c;
  Func image;
  image(x,y,c) = 1;

  Func s = sum(image, 100, 100, true);
  Halide::Image<float> output = s.realize(3);
  assert(fabs(output(0)-10000)<0.01);
  assert(fabs(output(1)-10000)<0.01);
  assert(fabs(output(2)-10000)<0.01);

  s = sum(image, 100, 100, false);
  output = s.realize(1);
  assert(fabs(output(0)-30000)<0.01);
}

void TestMin()
{
  Var x, y, c;
  Func image;
  image(x,y,c) = c;

  Func s = min(image, 100, 100, true);
  Halide::Image<int> output = s.realize(3);
  assert(fabs(output(0)-0)<0.01);
  assert(fabs(output(1)-1)<0.01);
  assert(fabs(output(2)-2)<0.01);

  s = min(image, 100, 100, false);
  output = s.realize(1);
  assert(fabs(output(0)-0)<0.01);
}

void TestLumiChromi()
{
  Halide::Image<float> input = load_image("../images/rgb.png");
  Func image = readImage(input, true);
  std::vector<Func> lumiChromi_ = lumiChromi(image);
  Halide::Image<float> output =
      lumiChromi_[0].realize(input.width(), input.height());

  save_image(output, "output/testLumi.png");

  output = lumiChromi_[1].realize(input.width(), input.height(), input.channels());
  save_image(output, "output/testChromi.png");
}

int main(int argc, char** argv)
{
  TestChangeGamma();
  TestExposure();
  TestBrightness();
  TestContrast();
  TestColor2Gray();
  TestYuvRgb();
  TestSaturate();
  TestSum();
  TestMin();
  TestLumiChromi();
}
