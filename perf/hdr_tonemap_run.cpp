#include <stdio.h>
#include <iostream>
#include "benchmark.h"
#include "hdr_tonemap.h"
#include "halide_image.h"
#include "halide_image_io.h"

using namespace Halide::Tools;

int main(int argc, char **argv) {
  if (argc < 4) {
      printf("Usage: .hdr_tonemap images numOfImages timing_iterations output\n");
      return 0;
  }

  int numOfImages = atoi(argv[2]);
  Image<float> input = load_image(std::string(argv[1]) + "-1.png");
  Image<float> images(input.width(), input.height(), 3, numOfImages);
  Image<float> output(input.width(), input.height(), 3);

  printf("Usage: %d %d\n", input.width(), input.height());
  for (int i=0; i < numOfImages; i++)
  {
    Image<uint16_t> input = load_image(std::string(argv[1]) + "-" + std::to_string(i+1) + ".png");
    for (int x=0; x < input.width(); x++)
      for (int y=0; y < input.height(); y++)
        for (int c=0; c < input.channels(); c++)
            images(x,y,c,i) = input(x,y,c);
  }

  printf("call\n");
  // int timing = atoi(argv[3]);
  // // Timing code
  // double best = benchmark(timing, 1, [&]() {
  //     local_laplacian(levels, alpha/(levels-1), beta, input, output);
  // });
  // printf("%gus\n", best * 1e6);
  //

  double min_t = benchmark(10, 10, [&]() {
      hdr_tonemap(images, output);
  });
  printf("Time: %gms\n", min_t * 1e3);

  hdr_tonemap(images, output);

  printf("done call\n");
  printf("%f", output(0,0,0));
  save_image(output, argv[4]);

  return 0;
}
