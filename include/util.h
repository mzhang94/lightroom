#include "Halide.h"

#ifdef HALIDE_IMAGE_IO_H
#include "halide_image_io.h"
#else
#include "image_io.h"
#define load_image load<float>
#define save_image save<float>
#endif

using namespace Halide;
using namespace Halide::Tools;

#define N_TIMES 10

Func clamp(Func image, float min = 0.0f, float max = 1.0f);
Func clampDomain(Func image, int xMax, int yMax);
Func readImage(Halide::Image<float> input, bool clamp=true);
void writeImage(Func func, std::string filename, int width, int height);

void verifyFunc(Func f, float val, int xRange, int yRange);
void verifyFunc(Func output, Func expected, int xRange, int yRange);
void verifyFunc(Func output, Image<float> expected);

unsigned long millisecond_timer(void);
float profile(Func myFunc, int w, int h);
float profile(Func myFunc, int w, int h, int c);
float profile(Func myFunc, int w, int h, int c, int i);
