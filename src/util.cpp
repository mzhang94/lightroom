#include "util.h"
#include "schedule.h"

using namespace Halide;
using namespace Halide::Tools;

Func clampDomain(Func image, int xMax, int yMax)
{
  Func clamped;
  Var x, y, c;

  if (image.dimensions()==2)
  {
    clamped(x, y) = image(clamp(x, 0, xMax), clamp(y, 0, yMax));
  }
  else
  {
    clamped(x, y, c) = image(clamp(x, 0, xMax), clamp(y, 0, yMax), c);
  }
  return clamped;
}

Func readImage(Halide::Image<float> input, bool clamp)
{
  Var x, y, c;
  Func image;
  if (input.dimensions()==2)
  {
    image(x,y) = input(x,y);
  }
  else
  {
    image(x,y,c) = input(x,y,c);
  }

  if (clamp)
  {
    return clampDomain(image, input.width()-1, input.height()-1);
  }
  return image;
}

void writeImage(Func func, std::string filename, int width, int height)
{
  Func clamped = clamp(func, 0, 1);
  apply_default_schedule(func);
  if (func.dimensions()==3)
  {
    Halide::Image<float> x(width, height, 3);
    clamped.realize(x);
    save_image(x, filename);
  }
  else
  {
    Halide::Image<float> x(width, height);
    clamped.realize(x);
    save_image(x, filename);
  }
}

void verifyFunc(Func f, float val, int xRange, int yRange)
{
  Image<float> image = f.dimensions()==3 ? f.realize(xRange, yRange, 3) : f.realize(xRange, yRange);
  for (int x = 0; x < xRange; x++)
  {
    for (int y = 0; y < yRange; y++)
    {
      if (f.dimensions() == 3)
      {
        for (int c = 0; c < 3; c++)
        {
          assert(fabs(image(x, y, c)-val)<0.01);
        }
      }
      else
      {
        assert(fabs(image(x, y)-val)<0.01);
      }
    }
  }
}

void verifyFunc(Func output, Func expected, int xRange, int yRange)
{
  Var x, y, c;
  Func diff;
  diff(x,y,c) = output.dimensions()==3 ? expected(x,y,c) - output(x,y,c) : expected(x,y) - output(x,y);
  verifyFunc(diff, 0, xRange, yRange);
}

void verifyFunc(Func output, Image<float> expected)
{
  Var x, y, c;
  Func diff;
  diff(x,y,c) = output.dimensions()==3 ? expected(x,y,c) - output(x,y,c) : expected(x,y) - output(x,y);
  verifyFunc(diff, 0, expected.width(), expected.height());
}

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
unsigned long millisecond_timer(void) {
    static SYSTEMTIME t;
    GetSystemTime(&t);
    return (unsigned long)((unsigned long)t.wMilliseconds
            + 1000*((unsigned long)t.wSecond
            + 60*((unsigned long)t.wMinute
            + 60*((unsigned long)t.wHour
            + 24*(unsigned long)t.wDay))));

#elif defined(_APPLE_) || defined(__APPLE__) || \
    defined(APPLE)   || defined(_APPLE)    || defined(__APPLE) || \
defined(unix)    || defined(__unix__)  || defined(__unix)
#include <unistd.h>
#include <sys/time.h>
unsigned long millisecond_timer(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (unsigned long)(t.tv_usec/1000 + t.tv_sec*1000);
}
#else
unsigned long millisecond_timer(void) {
    std::cout << "Warning: no timer implementation available" << std::endl;
    return 0;
}
#endif


/**
 * Print the runtime and throughout and return the runtime.
 */
float profile(Func myFunc, int w, int h) {
    myFunc.compile_jit();

    unsigned long s = millisecond_timer();
    for (int i=0; i<N_TIMES; i++) {
        myFunc.realize(w,h);
    }
    float total_time = float(millisecond_timer()-s);

    float mpixels = float(w*h)/1e6;
    std::cout << "runtime " << total_time/N_TIMES << " ms "
        << " throughput " << (mpixels*N_TIMES)/(total_time/1000) << " megapixels/sec" << std::endl;

    return total_time/N_TIMES;
}


/**
 * Print the runtime and throughout and return the runtime.
 */
float profile(Func myFunc, int w, int h, int c) {
    myFunc.compile_jit();

    unsigned long s = millisecond_timer();
    for (int i=0; i<N_TIMES; i++) {
        myFunc.realize(w,h,c);
    }
    float total_time = float(millisecond_timer()-s);

    float mpixels = float(w*h)/1e6;
    std::cout << " runtime " << total_time/N_TIMES << " ms "
        << " throughput " << (mpixels*N_TIMES)/(total_time/1000) << " megapixels/sec" << std::endl;

    return total_time/N_TIMES;
}
