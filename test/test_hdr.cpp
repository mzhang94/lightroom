#include "apps.h"
#include "util.h"
#include "point.h"
#include "schedule.h"

using namespace Halide;
using namespace Halide::Tools;

Func loadImages(std::string name, int k, int & width, int & height)
{
  std::vector<Image<float>> inputs;
  for (int i = 0; i < k; i++){
    inputs.push_back(load_image("../images/hdr_input/" + name + "-" + std::to_string(i+1) + ".png"));
  }
  width = inputs[0].width();
  height = inputs[0].height();
  Func images;
  Var x, y, c, i;
  images(x,y,c,i) = 0.0f;
  for (int i = 0; i < k; i++)
  {
    images(x,y,c,i) = inputs[i](x,y,c);
  }
  return images;
}

void saveImages(Func images, std::string name, int k, int width, int height)
{
  Halide::Image<float> output(width, height, 3, k);
  images.realize(output);
  for (int i = 0; i < k; i++)
  {
    Halide::Image<float> image(width, height, 3);
    for (int x = 0; x < width; x++)
    {
      for (int y = 0; y < height; y++)
      {
        for (int c = 0; c < 3; c++)
        {
          image(x,y,c) = output(x,y,c,i);
        }
      }
    }
    save_image(image, "output/hdr/" + name + "-" + std::to_string(i+1) + ".png");
  }

}
void testComputeWeight()
{
  int width, height;
  Func images = loadImages("ante2", 2, width, height);
  Func gammaCorrected;
  Var x, y, c, i;
  gammaCorrected(x,y,c,i) = pow(images(x,y,c,i), 2.2f);
  Func weights = computeWeight(gammaCorrected, 0.002f, 0.99f, 2);
  saveImages(weights, "testComputeWeight", 2, width, height);
}

void testComputeFactors()
{
  int width, height;
  Func images = loadImages("ante2", 2, width, height);
  Func gammaCorrected;
  Var x, y, c, i;
  gammaCorrected(x,y,c,i) = pow(images(x,y,c,i), 2.2f);
  Func weights = computeWeight(gammaCorrected, 0.002f, 0.99f, 2);
  Func factors = computeFactors(gammaCorrected, weights, width, height);

  Image<float> output(1);
  factors.realize(output);
  std::cout << output(0) << "\n";
}

void testMakeHDR()
{
  int width, height;
  Func images = loadImages("design", 7, width, height);
  Func hdr = mkhdr(images, 0.002, 0.99, width, height, 7);
  Func maxVal = max(hdr, width, height);
  Func scaled;
  Var x, y, c, i;
  scaled(x,y,c,i) = clamp(hdr(x,y,c) * 2 * pow(10, 2*i) / maxVal(0), 0.0f, 1.0f);
  apply_default_schedule(scaled);
  saveImages(scaled, "testHDR", 5, width, height);
}

void testToneMapping_(std::string name, int numOfImages)
{
  int width, height;
  Func images = loadImages(name, numOfImages, width, height);
  Func hdr = mkhdr(images, 0.002, 0.99, width, height, numOfImages);

  writeImage(hdr, "output/hdr/hdr.png", width, height);

  Func toneMapped = clamp(toneMap(hdr, width, height, pow(20, numOfImages), 100, 3, 0.1, false), 0, 1);
  apply_default_schedule(toneMapped);
  Image<float> output = toneMapped.realize(width, height, 3);
  save_image(output, "output/hdr/testToneMapping_" + name + ".png");

  Func toneMappedGrid = clamp(toneMap(hdr, width, height, pow(20, numOfImages), 100, 3, 0.1), 0, 1);
  apply_default_schedule(toneMappedGrid);
  output = toneMappedGrid.realize(width, height, 3);
  save_image(output, "output/hdr/testToneMappingGrid_" + name + ".png");

}
void testToneMapping()
{
  testToneMapping_("ante2", 2);
  testToneMapping_("ante3", 4);
  testToneMapping_("boston", 3);
}

int main(int argc, char ** argv)
{
  // testComputeWeight();
  // testComputeFactors();
  // testMakeHDR();
  testToneMapping();
}
