#include "image.h"
#include "imageException.h"
using namespace Halide;
using namespace Halide::Tools;

// Accessors
Expr ImageWrapper::operator()(Expr x, Expr y) const {
  return func(x,y);
}

Expr ImageWrapper::operator()(Expr x, Expr y, Expr c) const {
  if (dimensions() == 3)
  {
    return func(x,y,c);
  }
  else
  {
    return func(x,y);
  }
}

ImageWrapper::ImageWrapper(int x, int y, int z, const std::string &name_) {
  initialize_image_metadata(x,y,z,name_);
}

void ImageWrapper::initialize_image_metadata(int x, int y, int z, const std::string &name_) {
  dim_values[0] = x;
  dim_values[1] = y;
  dim_values[2] = z;
  dims = z==0 ? 2 : 3;

  if ( x < 0 )
    throw NegativeDimensionException();
  if ( y< 0)
    throw NegativeDimensionException();
  if (z < 0 )
    throw NegativeDimensionException();

  if (!name_.empty())
  {
    func = Func(name_);
  }
  else
  {
    func = Func();
  }
}

void ImageWrapper::read_image_data(Halide::Image<float> image) {
  Var x, y, c;
  if (dimensions()==3)
  {
    func(x,y,c) = image(x,y,c);
  }
  else
  {
    func(x,y) = image(x,y);
  }
}

ImageWrapper::ImageWrapper(Halide::Image<float> image, const std::string & filename) {
  initialize_image_metadata(image.width(), image.height(), image.channels(), filename);
  read_image_data(image);
}

ImageWrapper::ImageWrapper(const std::string & filename) {
  Halide::Image<float> image = load_image(filename);
  initialize_image_metadata(image.width(), image.height(), image.channels(), "");
  read_image_data(image);
}

ImageWrapper::~ImageWrapper() { } // Nothing to clean up


void ImageWrapper::write(const std::string &filename) {
  if (dimensions()==3)
  {
    Halide::Image<float> x(width(), height(), channels());
    func.realize(x);
    save_image(x, filename);
  }
  else
  {
    Halide::Image<float> x(width(), height());
    func.realize(x);
    save_image(x, filename);
  }
}

void compareDimensions(const ImageWrapper & im1, const ImageWrapper & im2)  {
  if(im1.dimensions() != im2.dimensions())
    throw MismatchedDimensionsException();
  for (int i = 0; i < im1.dimensions(); i++ ) {
    if (im1.extent(i) != im2.extent(i))
      throw MismatchedDimensionsException();
  }
}

ImageWrapper operator+ (const ImageWrapper & im1, const ImageWrapper & im2) {
  compareDimensions(im1, im2);

  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, c;
  output(x,y,c) = im1(x,y,c) + im2(x,y,c);
  return output;
}

ImageWrapper operator- (const ImageWrapper & im1, const ImageWrapper & im2) {
  compareDimensions(im1, im2);
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, c;
  output(x,y,c) = im1(x,y,c) - im2(x,y,c);
  return output;
}

ImageWrapper operator* (const ImageWrapper & im1, const ImageWrapper & im2) {
  compareDimensions(im1, im2);
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, c;
  output(x,y,c) = im1(x,y,c) * im2(x,y,c);
  return output;
}

ImageWrapper operator/ (const ImageWrapper & im1, const ImageWrapper & im2) {
  compareDimensions(im1, im2);
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, c;
  output(x,y,c) = im1(x,y,c) / im2(x,y,c);
  return output;
}

ImageWrapper operator+ (const ImageWrapper & im1, const float & c) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = im1(x,y,z) + c;
  return output;
}

ImageWrapper operator- (const ImageWrapper & im1, const float & c) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = im1(x,y,z) - c;
  return output;
}
ImageWrapper operator* (const ImageWrapper & im1, const float & c) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = im1(x,y,z) * c;
  return output;
}
ImageWrapper operator/ (const ImageWrapper & im1, const float & c) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = im1(x,y,z) / c;
  return output;
}

ImageWrapper operator+(const float & c, const ImageWrapper & im1) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = c + im1(x,y,z);
  return output;
}

ImageWrapper operator- (const float & c, const ImageWrapper & im1) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = c - im1(x,y,z);
  return output;
}

ImageWrapper operator* (const float & c, const ImageWrapper & im1) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = c * im1(x,y,z);
  return output;
}

ImageWrapper operator/ (const float & c, const ImageWrapper & im1) {
  ImageWrapper output(im1.extent(0), im1.extent(1), im1.extent(2));
  Var x, y, z;
  output(x,y,z) = c / im1(x,y,z);
  return output;
}
