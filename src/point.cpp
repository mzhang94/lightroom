#include "point.h"
using namespace Halide;

Func clamp(Func image, float min, float max)
{
  Var x,y,c;
  Func output("clamp");

  if (image.dimensions() == 3)
  {
    output(x,y,c) = clamp(image(x,y,c), min, max);
  }
  else
  {
    output(x,y) = clamp(image(x,y), min, max);
  }
  return output;
}

Func changeGamma(Func image, float oldGamma, float newGamma)
{
  Var x,y,c;
  Func output;

  output(x,y,c) = pow(image(x,y,c), newGamma/oldGamma);

  return output;
}

Func brightness(Func image, float factor)
{
  Var x,y,c;
  Func output;

  output(x,y,c) = image(x,y,c) * factor;
  return output;
}

Func contrast(Func image, float factor, float midpoint)
{
  Var x,y,c;
  Func output;

  output(x,y,c) = (image(x,y,c) - midpoint) * factor + midpoint;
  return output;
}

Func exposure(Func image, float factor, float gamma)
{
  Var x,y,c;
  Func output;

  output(x,y,c) = image(x,y,c) * Halide::pow(factor, 1.0f/gamma);
  return output;
}

Func color2gray(Func image, float wr, float wg, float wb)
{
  Var x, y, c;
  Func output("lumi");

  output(x,y) = image(x,y,0) * wr + image(x,y,1) * wg + image(x,y,2) * wb;
  return output;
}

Func rgb2yuv(Func image)
{
  Var x, y, c;
  Func output;

  output(x,y,c) = 0.0f;
  output(x,y,0) = 0.299f * image(x,y,0) + 0.587f * image(x,y,1) + 0.114f * image(x,y,2);
  output(x,y,1) = -0.147f * image(x,y,0) - 0.289f * image(x,y,1) + 0.436f * image(x,y,2);
  output(x,y,2) = 0.615f * image(x,y,0) - 0.515f * image(x,y,1) - 0.100f * image(x,y,2);
  return output;
}

Func yuv2rgb(Func image)
{
  Var x,y,c;
  Func output;

  output(x,y,c) = 0.0f;
  output(x,y,0) = image(x,y,0) + 1.14f * image(x,y,2);
  output(x,y,1) = image(x,y,0) - 0.395f * image(x,y,1) - 0.581f * image(x,y,2);
  output(x,y,2) = image(x,y,0) + 2.032f * image(x,y,1);

  return output;
}

Func saturate(Func image, float factor)
{
  Var x,y,c;
  Func yuv = rgb2yuv(image);

  Func yuv1;
  yuv1(x,y,c) = 0.0f;
  yuv1(x,y,0) = yuv(x,y,0);
  yuv1(x,y,1) = yuv(x,y,1) * factor;
  yuv1(x,y,2) = yuv(x,y,2) * factor;
  return yuv2rgb(yuv1);
}

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

std::vector<Func> lumiChromi(Func image, float wr, float wg, float wb)
{
  Var x, y,c;
  Func lumi, chromi;
  lumi(x,y) = image(x,y,0)*wr + image(x,y,1)*wg + image(x,y,2)*wb;
  chromi(x,y,c) = image(x,y,c)/lumi(x,y);
  std::vector<Func> output;
  output.push_back(lumi);
  output.push_back(chromi);
  return output;
}

Func substract(Func image1, Func image2)
{
  Func output;
  Var x,y,c;
  if (image1.dimensions()==2)
  {
    output(x,y) = image1(x,y) - image2(x,y);
  }
  else
  {
    output(x,y,c) = image1(x,y,c) - image2(x,y,c);
  }
  return output;
}

Func add(Func image1, Func image2)
{
  Func output;
  Var x,y,c;
  if (image1.dimensions()==2)
  {
    output(x,y) = image1(x,y) + image2(x,y);
  }
  else
  {
    output(x,y,c) = image1(x,y,c) + image2(x,y,c);
  }
  return output;
}

Func multiply(Func image1, Func image2)
{
  Func output;
  Var x,y,c;
  if (image1.dimensions()==2)
  {
    output(x,y) = image1(x,y) * image2(x,y);
  }
  else
  {
    output(x,y,c) = image1(x,y,c) * image2(x,y,c);
  }
  return output;
}

Func divide(Func image1, Func image2)
{
  Func output;
  Var x,y,c;
  if (image1.dimensions()==2)
  {
    output(x,y) = image1(x,y) / image2(x,y);
  }
  else
  {
    output(x,y,c) = image1(x,y,c) / image2(x,y,c);
  }
  return output;
}

Func add(Func image, Expr a)
{
  Func output;
  Var x,y,c;
  if (image.dimensions()==2)
  {
    output(x,y) = image(x,y) + a;
  }
  else
  {
    output(x,y,c) = image(x,y,c) + a;
  }
  return output;
}

Func multiply(Func image, Expr a)
{
  Func output;
  Var x,y,c;
  if (image.dimensions()==2)
  {
    output(x,y) = image(x,y) * a;
  }
  else
  {
    output(x,y,c) = image(x,y,c) * a;
  }
  return output;

}
