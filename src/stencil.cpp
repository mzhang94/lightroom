#include <cmath>
#include "point.h"
#include "stencil.h"
#include "schedule.h"

using namespace Halide;

Expr inline cubic(Expr p0, Expr p1, Expr p2, Expr p3, Expr x)
{
  return (-0.5f*p0+1.5f*p1-1.5f*p2+0.5f*p3)*x*x*x + (p0-2.5f*p1+2*p2-0.5f*p3)*x*x +
          (-0.5f*p0+0.5f*p2)*x +p1;
}

Expr interpolate(Func image, Expr x, Expr y, Expr x1, Expr y1, Expr x2, Expr y2, Expr c, INTERPOLATE_METHOD method)
{
  switch (method) {
    case INTERPOLATE_METHOD::NN:
    {
      Expr x0 = select(x-x1 < x2-x, x1, x2);
      Expr y0 = select(y-y1 < y2-y, y1, y2);
      return image(x0,y0,c);
    }
    case INTERPOLATE_METHOD::BILIN:
    {
      Expr a = (x-x1)/cast<float>(x2-x1);
      Expr b = (y-y1)/cast<float>(y2-y1);
      return (1-a)*(1-b)*image(x1, y1,c) + (1-a)*b*image(x1, y2, c) +
              a*(1-b)*image(x2, y1,c) + a*b*image(x2, y2,c);
    }
    case INTERPOLATE_METHOD::BICUBIC:
    {
      Expr xstep = x2-x1;
      Expr ystep = y2-y1;
      Expr x0 = x1-xstep;
      Expr x3 = x2+xstep;

      Expr y0 = y1-ystep;
      Expr y3 = y2+ystep;

      Expr a0 = cubic(image(x0,y0,c), image(x0,y1,c), image(x0,y2,c), image(x0,y3,c), y-y1);
      Expr a1 = cubic(image(x1,y0,c), image(x1,y1,c), image(x1,y2,c), image(x1,y3,c), y-y1);
      Expr a2 = cubic(image(x2,y0,c), image(x2,y1,c), image(x2,y2,c), image(x2,y3,c), y-y1);
      Expr a3 = cubic(image(x3,y0,c), image(x3,y1,c), image(x3,y2,c), image(x3,y3,c), y-y1);

      return cubic(a0, a1, a2, a3, x-x1);
    }
    default:
    {
      assert(false);
    }
  }
}

Func scale(Func image, float factorX, float factorY, INTERPOLATE_METHOD method)
{
  Var x,y,c;
  Func output;
  Expr x1 = cast<int>(floor(x/factorX));
  Expr y1 = cast<int>(floor(y/factorY));

  output(x,y,c) = interpolate(image, x/factorX, y/factorY, x1, y1, x1+1, y1+1, c, method);
  return output;
}

Func gradient(Func image, GRADIENT_KERNEL kernel)
{
  Var x,y,c;

  Func gradient("gradient");
  if (image.dimensions() == 3)
  {
    switch(kernel){
      case SOBEL:
      {
        gradient(x,y,c) =
        {
          -image(x-1,y-1,c) + image(x+1,y-1,c) - 2.0f*image(x-1,y,c) + 2.0f*image(x+1,y,c) - image(x-1,y+1,c) + image(x+1,y+1,c),
          -image(x-1,y-1,c) + image(x-1,y+1,c) - 2.0f*image(x,y-1,c) + 2.0f*image(x,y+1,c) - image(x+1,y-1,c) + image(x+1,y+1,c)
        };
        break;
      }
      case PREWITT:
      {
        gradient(x,y,c) =
        {
          -image(x-1,y-1,c) + image(x+1,y-1,c) - image(x-1,y,c) + image(x+1,y,c) - image(x-1,y+1,c) + image(x+1,y+1,c),
          -image(x-1,y-1,c) + image(x-1,y+1,c) - image(x,y-1,c) + image(x,y+1,c) - image(x+1,y-1,c) + image(x+1,y+1,c)
        };
        break;
      }
      default:
      {
        assert(false);
      }
    }
  }
  else
  {
    switch(kernel){
      case SOBEL:
      {
        gradient(x,y) =
        {
          -image(x-1,y-1) + image(x+1,y-1) - 2.0f*image(x-1,y) + 2.0f*image(x+1,y) - image(x-1,y+1) + image(x+1,y+1),
          -image(x-1,y-1) + image(x-1,y+1) - 2.0f*image(x,y-1) + 2.0f*image(x,y+1) - image(x+1,y-1) + image(x+1,y+1)
        };
        break;
      }
      case PREWITT:
      {
        gradient(x,y) =
        {
          -image(x-1,y-1) + image(x+1,y-1) - image(x-1,y) + image(x+1,y) - image(x-1,y+1) + image(x+1,y+1),
          -image(x-1,y-1) + image(x-1,y+1) - image(x,y-1) + image(x,y+1) - image(x+1,y-1) + image(x+1,y+1)
        };
        break;
      }
      default:
      {
        assert(false);
      }
    }
  }
  return gradient;
}

// image.compute_root() or image.compute_at(blur_y, xo);
// output can't be inlined
Func boxBlur(Func image, int halfWindowSize)
{
  Var x,y,c;

  RDom r(-halfWindowSize, 2*halfWindowSize+1);
  Func blurx, blury;
  if (image.dimensions() == 3)
  {
    blurx(x,y,c) = sum(image(x+r,y,c))/(2*halfWindowSize+1);
    blury(x,y,c) = sum(blurx(x,y+r,c))/(2*halfWindowSize+1);
  }
  else
  {
    blurx(x,y) = sum(image(x+r,y))/(2*halfWindowSize+1);
    blury(x,y) = sum(blurx(x,y+r))/(2*halfWindowSize+1);
  }

  /************** Schedule ****************/
  Var xo, yo, xi, yi;
  blury.tile(x, y, xo, yo, xi, yi, 256, 32).parallel(yo).vectorize(xi, 8);
  blurx.compute_at(blury, xo);
  blurx.vectorize(x, 8);

  return blury;
}

Func convolveX(Func image, Func kernel, int halfSize)
{
  Var x, y, c;

  RDom r(-halfSize, 2*halfSize+1);

  Func output;
  if (image.dimensions() == 3)
  {
    output(x,y,c) = sum(image(x+r,y,c) * kernel(r));
  }
  else
  {
    output(x,y) = sum(image(x+r,y) * kernel(r));
  }
  return output;
}

Func convolveY(Func image, Func kernel, int halfSize)
{
  Var x, y, c;

  RDom r(-halfSize, 2*halfSize+1);

  Func output;
  if (image.dimensions() == 3)
  {
    output(x,y,c) = sum(image(x,y+r,c) * kernel(r));
  }
  else
  {
    output(x,y) = sum(image(x,y+r) * kernel(r));
  }
  return output;
}

Func convolve2d(Func image, Func kernel, int halfWidth, int halfHeight)
{
  Var x, y, c;

  RDom r(-halfWidth, 2*halfWidth+1, -halfHeight, 2*halfHeight+1);

  Func output;
  output(x,y,c) = sum(image(x+r.x,y+r.y,c) * kernel(r.x, r.y));
  return output;
}

Expr gaussian(Expr x, float sigma)
{
  float sigma2 = sigma*sigma;
  return exp(-x*x/(2*sigma2))/(2*float(M_PI)*sigma2);
}

Expr gaussian(Expr x, Expr y, float sigma)
{
  float sigma2 = sigma*sigma;
  return exp(-(x*x+y*y)/(2*sigma2))/(2*float(M_PI)*sigma2);
}

Expr gaussian(Expr x, Expr y, Expr z, float sigma)
{
  float sigma2 = sigma*sigma;
  return exp(-(x*x+y*y+z*z)/(2*sigma2))/(2*float(M_PI)*sigma2);
}

Func gaussianKernel1d(float sigma, int halfSize)
{
  Var x;

  Func kernel;
  kernel(x) = gaussian(x, sigma);
  if (halfSize > 0)
  {
    Func s, output;
    RDom r(-halfSize, 2*halfSize+1);
    s(x) = sum(kernel(r));

    output(x) = kernel(x)/s(0);
    return output;
  }
  return kernel;
}

Func gaussianKernel2d(float sigma, int halfSize)
{
  Var x,y;

  Func kernel;
  kernel(x,y) = gaussian(x, y, sigma);

  if (halfSize > 0)
  {
    Func s, output;
    RDom r(-halfSize, 2*halfSize+1, -halfSize, 2*halfSize+1);
    s(x) = sum(kernel(r.x, r.y));

    output(x,y) = kernel(x,y)/s(0);
    return output;
  }
  return kernel;
}

Func gaussianFilter(Func image, float sigma, int halfWindowSize)
{
  Func kernel = gaussianKernel1d(sigma, halfWindowSize);
  Func blurx = convolveX(image, kernel, halfWindowSize);
  Func blury = convolveY(blurx, kernel, halfWindowSize);

  return blury;
}

Func laplacianFilter(Func image)
{
  Var x, y, c;
  Func output;
  if (image.dimensions() == 3)
  {
    output(x,y,c) = 4*image(x,y,c) - image(x-1,y,c) - image(x+1,y,c) - image(x,y-1,c) - image(x,y+1,c);
  }
  else
  {
    output(x,y) = 4*image(x,y) - image(x-1,y) - image(x+1,y) - image(x,y-1) - image(x,y+1);
  }
  return output;
}

Func bilateralFilter(Func image, float sigmaRange, float sigmaDomain, int hSize)
{
  Var x, y, c;
  Func gaussianDomain = gaussianKernel2d(sigmaDomain);

  RDom r(-hSize, 2*hSize+1, -hSize, 2*hSize+1);
  Expr gDomain = gaussianDomain(r.x,r.y);
  Func output;
  if (image.dimensions()==3)
  {
    Expr gRange = gaussian(image(x,y,0)-image(x+r.x,y+r.y,0), image(x,y,1)-image(x+r.x,y+r.y,1), image(x,y,2)-image(x+r.x,y+r.y,2), sigmaRange);
    Func filtered, k;
    filtered(x,y,c) = sum(gDomain*gRange*image(x+r.x,y+r.y,c));
    k(x,y) = sum(gDomain*gRange);
    output(x,y,c) = filtered(x,y,c) / k(x,y);
  }
  else
  {
    Expr gRange = gaussian(image(x,y)-image(x+r.x,y+r.y), sigmaRange);
    Func filtered, k;
    filtered(x,y) = sum(gDomain*gRange*image(x+r.x,y+r.y));
    k(x,y) = sum(gDomain*gRange);
    output(x,y) = filtered(x,y) / k(x,y);
  }
  return output;
}

Func bilateralFilterYUV(Func image, float sigmaRange, float sigmaY, float sigmaUV, float truncateDomain)
{
  Var x, y, c;
  Func yuv = rgb2yuv(image);

  Func gaussianDomainY = gaussianKernel2d(sigmaY);
  Func gaussianDomainUV = gaussianKernel2d(sigmaUV);


  int hSizeY = int(ceil(sigmaY * truncateDomain));
  RDom ry(-hSizeY, 2*hSizeY+1, -hSizeY, 2*hSizeY+1);
  Expr gDomainY = gaussianDomainY(ry.x, ry.y);
  int hSizeUV = int(ceil(sigmaUV * truncateDomain));
  RDom ruv(-hSizeUV, 2*hSizeUV+1, -hSizeUV, 2*hSizeUV+1);
  Expr gDomainUV = gaussianDomainUV(ruv.x, ruv.y);

  Expr gRangeY = gaussian(yuv(x,y,0)-yuv(x+ry.x,y+ry.y,0), yuv(x,y,1)-yuv(x+ry.x,y+ry.y,1), yuv(x,y,2)-yuv(x+ry.x,y+ry.y,2), sigmaRange);
  Expr gRangeUV = gaussian(yuv(x,y,0)-yuv(x+ruv.x,y+ruv.y,0), yuv(x,y,1)-yuv(x+ruv.x,y+ruv.y,1), yuv(x,y,2)-yuv(x+ruv.x,y+ruv.y,2), sigmaRange);

  Func filtered, ky, kuv;
  filtered(x,y,c) = select(c==0, sum(gDomainY*gRangeY*yuv(x+ry.x,y+ry.y,0)),
                    select(c==1, sum(gDomainUV*gRangeUV*yuv(x+ruv.x,y+ruv.y,1)),
                                 sum(gDomainUV*gRangeUV*yuv(x+ruv.x,y+ruv.y,2))));
  ky(x,y) = sum(gDomainY*gRangeY);
  kuv(x,y) = sum(gDomainUV*gRangeUV);

  Func normalized;
  normalized(x,y,c) = select(c==0, filtered(x,y,c)/ky(x,y), filtered(x,y,c)/kuv(x,y));

  return yuv2rgb(normalized);
}

Func bilateralGrid(Func image, float sigmaRange, int sigmaDomain, float minVal, float maxVal)
{
  return crossBilateral(image, image, sigmaRange, sigmaDomain, minVal, maxVal);
}

// gridIm.compute_root(), filterVal.compute_at(blur_z, y)
// bilateralGrid stencil
Func crossBilateral(Func gridIm, Func filterIm, float sigmaRange, int sigmaDomain, float minVal, float maxVal)
{
  Var x, y, z, c;
  // Construct the bilateral grid
  RDom r(0, sigmaDomain, 0, sigmaDomain);
  Expr gridVal = gridIm(x * sigmaDomain + r.x, y * sigmaDomain + r.y);
  Expr filterVal = filterIm(x * sigmaDomain + r.x, y * sigmaDomain + r.y);
  gridVal = clamp(gridVal, minVal, maxVal);
  Expr zi = cast<int>(floor(gridVal * (1.0f/sigmaRange)));
  Func histogram("histogram");
  histogram(x, y, z, c) = 0.0f;
  histogram(x, y, zi, c) += select(c == 0, filterVal, 1.0f);

  // Blur the grid using a five-tap filter
  Func blurx("blurx"), blury("blury"), blurz("blurz");
  blurz(x, y, z, c) = (histogram(x, y, z-2, c) +
                       histogram(x, y, z-1, c)*4 +
                       histogram(x, y, z  , c)*6 +
                       histogram(x, y, z+1, c)*4 +
                       histogram(x, y, z+2, c));
  blurx(x, y, z, c) = (blurz(x-2, y, z, c) +
                       blurz(x-1, y, z, c)*4 +
                       blurz(x  , y, z, c)*6 +
                       blurz(x+1, y, z, c)*4 +
                       blurz(x+2, y, z, c));
  blury(x, y, z, c) = (blurx(x, y-2, z, c) +
                       blurx(x, y-1, z, c)*4 +
                       blurx(x, y  , z, c)*6 +
                       blurx(x, y+1, z, c)*4 +
                       blurx(x, y+2, z, c));

  // Take trilinear samples to compute the output
  Expr zv = clamp(gridIm(x,y), minVal, maxVal) * (1.0f/sigmaRange);
  zi = cast<int>(floor(zv));
  Expr zf = zv - zi;
  Expr xf = cast<float>(x % sigmaDomain) / sigmaDomain;
  Expr yf = cast<float>(y % sigmaDomain) / sigmaDomain;
  Expr xi = x/sigmaDomain;
  Expr yi = y/sigmaDomain;
  Func interpolated("interpolated");
  interpolated(x, y, c) =
      lerp(lerp(lerp(blury(xi, yi, zi, c), blury(xi+1, yi, zi, c), xf),
                lerp(blury(xi, yi+1, zi, c), blury(xi+1, yi+1, zi, c), xf), yf),
           lerp(lerp(blury(xi, yi, zi+1, c), blury(xi+1, yi, zi+1, c), xf),
                lerp(blury(xi, yi+1, zi+1, c), blury(xi+1, yi+1, zi+1, c), xf), yf), zf);

  // Normalize
  Func bilateral_grid("bilateral_grid");
  bilateral_grid(x, y) = interpolated(x, y, 0)/interpolated(x, y, 1);

  /**************** Schedule ******************/
  blurz.compute_root().reorder(c, z, x, y).parallel(y).vectorize(x, 8).unroll(c);
  histogram.compute_at(blurz, y);
  histogram.update().reorder(c, r.x, r.y, x, y).unroll(c);
  blurx.compute_root().reorder(c, x, y, z).parallel(z).vectorize(x, 8).unroll(c);
  blury.compute_root().reorder(c, x, y, z).parallel(z).vectorize(x, 8).unroll(c);
  return bilateral_grid;
}
