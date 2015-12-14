#include "Halide.h"
#include "point.h"
#include "stencil.h"
#include "util.h"
#include "schedule.h"
using namespace Halide;
using namespace Halide::Tools;

void TestScale()
{
  Var x, y, c;
  Func image;
  image(x,y,c) = (0.2f*x+0.3f*y+0.5f*c)%1.0f;

  Func scaledNN = clamp(scale(image, 25, 25, INTERPOLATE_METHOD::NN), 0.0f, 1.0f);

  Halide::Image<float> output(100, 100, 3);
  output.set_min(25, 25);
  scaledNN.realize(output);
  save_image(output, "output/testScaleNN.png");

  Func scaledLin = clamp(scale(image, 25, 25, INTERPOLATE_METHOD::BILIN), 0.0f, 1.0f);
  scaledLin.realize(output);
  save_image(output, "output/testScaleLin.png");

  Func scaledCubic = clamp(scale(image, 25, 25, INTERPOLATE_METHOD::BICUBIC), 0.0f, 1.0f);
  scaledCubic.realize(output);
  save_image(output, "output/testScaleCubic.png");
}

void TestGradient()
{
  Var x, y, c;
  Func image;
  image(x,y,c) = x+2.0f*y;

  Func gradientSobel = gradient(image, GRADIENT_KERNEL::SOBEL);
  Func gradientPrewitt = gradient(image, GRADIENT_KERNEL::PREWITT);

  Realization rSobel = gradientSobel.realize(100, 100, 3);
  Realization rPrewitt = gradientPrewitt.realize(100, 100, 3);

  Image<float> sobelX = rSobel[0];
  Image<float> sobelY = rSobel[1];
  Image<float> prewittX = rPrewitt[0];
  Image<float> prewittY = rPrewitt[1];
  for (int x = 0; x < 100; x++)
  {
    for (int y = 0; y < 100; y++)
    {
      for (int c = 0; c < 3; c++)
      {
        assert(fabs(sobelX(x,y,c)-8) < 0.01);
        assert(fabs(sobelY(x,y,c)-16) < 0.01);
        assert(fabs(prewittX(x,y,c)-6) < 0.01);
        assert(fabs(prewittY(x,y,c)-12) < 0.01);
      }
    }
  }
}

void TestBoxBlur()
{
  Var x, y, c;
  Func image;

  int width = 1000;
  int height = 1000;
  image(x,y,c) = 1.0f;
  Func blurred = boxBlur(image, 1);

  verifyFunc(blurred, 1, width, height);
}

void TestConvolve()
{
  Var x, y, c;
  Func image;
  Func kernel;

  image(x,y,c) = cast<float>(x);
  kernel(x) = select(x==-1||x==1, cast<float>(x), 0);

  Func convolvedX = convolveX(image, kernel, 1);
  Func convolvedY = convolveY(image, kernel, 1);

  verifyFunc(convolvedX, 1, 100, 100);
  verifyFunc(convolvedY, 0, 100, 100);
}

void TestGaussian()
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

void TestLaplacian()
{
  Halide::Image<float> input = load_image("../images/rgb.png");
  Func image = readImage(input, true);
  Func filtered = laplacianFilter(image);
  apply_default_schedule(filtered);
  Halide::Image<float> output =
      filtered.realize(input.width(), input.height(), input.channels());

  save_image(output, "output/testLaplacian.png");
}

void TestBilateral()
{
  Halide::Image<float> input = load_image("../images/lens.png");
  Func image = readImage(input, true);
  Func filtered = clamp(bilateralFilter(image), 0, 1);
  Func filteredYUV = clamp(bilateralFilterYUV(image), 0, 1);
  apply_default_schedule(filtered);
  apply_default_schedule(filteredYUV);
  Halide::Image<float> output =
      filtered.realize(input.width(), input.height(), input.channels());

  save_image(output, "output/testBilateral.png");

  output = filteredYUV.realize(input.width(), input.height(), input.channels());
  save_image(output, "output/testBilateralYUV.png");

  Func gray = clampDomain(color2gray(image), input.width(), input.height());
  std::cout << gray.name() <<'\n';
  Func grayFilterd = clamp(bilateralFilter(gray), 0, 1);
  apply_default_schedule(grayFilterd);
  output = grayFilterd.realize(input.width(), input.height());
  save_image(output, "output/testBilateralGray.png");

  Func grayGrid = clamp(bilateralGrid(gray), 0, 1);
  apply_default_schedule(grayGrid);
  output = grayGrid.realize(input.width(), input.height());
  save_image(output, "output/testBilateralGrid.png");

}

int main(int argc, char ** argv)
{
  TestScale();
  TestGradient();
  TestBoxBlur();
  TestGaussian();
  TestLaplacian();
  TestBilateral();
}
