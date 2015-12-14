#include "global.h"

Func sum(Func image, int width, int height, bool channelwise)
{
  Var c;
  Func s;
  s(c) = 0.0f;
  if (image.dimensions() == 3)
  {
    if (channelwise)
    {
      RDom r(0, width, 0, height);
      s(c) += image(r.x,r.y,c);
    }
    else
    {
      RDom r(0, width, 0, height, 0, 3);
      s(0) += image(r.x,r.y,r.z);
    }
  }
  else
  {
    RDom r(0, width, 0, height);
    assert(image.dimensions()==2);
    s(0) += image(r.x,r.y);
  }
  return s;
}

Func min(Func image, int width, int height, bool channelwise)
{
  Var c;
  Func s;
  if (image.dimensions() == 3)
  {
    if (channelwise)
    {
      s(c) = image(0,0,c);
      RDom r(0, width, 0, height);
      s(c) = select(image(r.x,r.y,c) < s(c), image(r.x,r.y,c), s(c));
    }
    else
    {
      s(c) = image(0,0,0);
      RDom r(0, width, 0, height, 0, 3);
      s(0) = select(image(r.x,r.y,r.z) < s(0), image(r.x,r.y,r.z), s(0));
    }
  }
  else
  {
    s(c) = image(0,0);
    RDom r(0, width, 0, height);
    s(0) = select(image(r.x,r.y) < s(0), image(r.x,r.y), s(0));
  }
  return s;
}

Func max(Func image, int width, int height, bool channelwise)
{
  Var c;
  Func s;
  if (image.dimensions() == 3)
  {
    if (channelwise)
    {
      s(c) = image(0,0,c);
      RDom r(0, width, 0, height);
      s(c) = select(image(r.x,r.y,c) > s(c), image(r.x,r.y,c), s(c));
    }
    else
    {
      s(c) = image(0,0,0);
      RDom r(0, width, 0, height, 0, 3);
      s(0) = select(image(r.x,r.y,r.z) > s(0), image(r.x,r.y,r.z), s(0));
    }
  }
  else
  {
    s(c) = image(0,0);
    RDom r(0, width, 0, height);
    s(0) = select(image(r.x,r.y) > s(0), image(r.x,r.y), s(0));
  }
  return s;
}

//output(0)=min, output(1) = max
Func minMax(Func image, int width, int height, bool channelwise)
{
  Var c;
  Func s("minmax");
  Expr pixel;
  if (image.dimensions() == 3)
  {
    s(c) = image(0,0,0);
    if (channelwise)
    {
      RDom r(0, width, 0, height);
      pixel = image(r.x,r.y,c);
    }
    else
    {
      RDom r(0, width, 0, height, 0, 3);
      pixel = image(r.x, r.y, r.z);
    }
  }
  else
  {
    s(c) = image(0,0);
    RDom r(0, width, 0, height);
    pixel = image(r.x, r.y);
  }
  s(c) = select((pixel > s(c)) ^ (c == 0), pixel, s(c));

  //schedule
  s.compute_root().unroll(c);
  return s;
}

Expr findBin(Expr val, float minVal, float maxVal, int nbins)
{
  return cast<int>((clamp(val, minVal, maxVal) - minVal)/ (maxVal-minVal) * (nbins-1));
}

Func histogram(Func image, int width, int height, std::string name, float minVal, float maxVal, int nbins)
{
  Var x("x");
  Func hist(name);
  float w = 1.0/width/height;
  hist(x) = 0.0f;
  RDom r(0, width, 0, height);
  hist(findBin(image(r.x,r.y), minVal, maxVal, nbins)) += w;
  return hist;
}

Func cumulate(Func hist, int nbins, std::string name)
{
  Var x("x");
  RDom r(1,nbins-1);
  Func cumulative(name);

  cumulative(x) = 0.0f;
  cumulative(0) = hist(0);
  cumulative(r) = cumulative(r-1) + hist(r);
  return cumulative;
}

Func dot(Func im1, Func im2, int width, int height)
{
  Var x("x");
  Func s("dotProd");
  s(x) = 0.0f;
  if (im1.dimensions() == 3)
  {
    RDom r(0, width, 0, height, 0, 3);
    s(0) += im1(r.x,r.y,r.z) * im2(r.x,r.y,r.z);
  }
  else
  {
    RDom r(0, width, 0, height);
    s(0) += im1(r.x,r.y) * im2(r.x,r.y);
  }
  return s;
}
