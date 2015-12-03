#include "Halide.h"
#include "point.h"
#include "stencil.h"
#include "util.h"
#include "apps.h"
using namespace Halide;

Func demosaic(Func raw, int offsetGreen, int offsetRedX, int offsetRedY, int offsetBlueX, int offsetBlueY)
{
  Var x,y,c;
  Func output;
  Func green;

  green(x,y) = select((x+y)%2 == offsetGreen, raw(x,y),
                      select(abs(raw(x+1, y) - raw(x-1,y)) < abs(raw(x, y+1) - raw(x,y-1)),
                             (raw(x+1,y)+raw(x-1,y))/2,
                             (raw(x,y+1)+raw(x,y-1))/2));

  Func rawGreenDiff;
  rawGreenDiff(x,y) = raw(x,y)-green(x,y);
  Func redGreenDiff;
  redGreenDiff(x,y) = select(x%2==offsetRedX,
                      select(y%2==offsetRedY, rawGreenDiff(x,y), (rawGreenDiff(x,y-1)+rawGreenDiff(x,y+1))/2),
                      select(y%2==offsetRedY, (rawGreenDiff(x-1,y)+rawGreenDiff(x+1,y))/2,
                              (rawGreenDiff(x-1,y-1)+rawGreenDiff(x-1,y+1)+rawGreenDiff(x+1,y-1)+rawGreenDiff(x+1,y+1))/4));
  Func blueGreenDiff;
  blueGreenDiff(x,y) = select(x%2==offsetBlueX,
                      select(y%2==offsetBlueY, rawGreenDiff(x,y), (rawGreenDiff(x,y-1)+rawGreenDiff(x,y+1))/2),
                      select(y%2==offsetBlueY, (rawGreenDiff(x-1,y)+rawGreenDiff(x+1,y))/2,
                              (rawGreenDiff(x-1,y-1)+rawGreenDiff(x-1,y+1)+rawGreenDiff(x+1,y-1)+rawGreenDiff(x+1,y+1))/4));

  output(x,y,c) = select(c==0, redGreenDiff(x,y)+green(x,y), select(c==1, green(x,y), blueGreenDiff(x,y)+green(x,y)));
  return output;
}
