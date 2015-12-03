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

class ImageWrapper {
 public:

  // Constructor to initialize an image of size width_*height_*channels_
  // If channels_ are zero, the image will be one dimensional
  // If channels_ is zero, the image will be two dimensional
  ImageWrapper(int width_, int height_, int channels_ = 0, const std::string &name="");

  // Constructor to create an image from a file.
  ImageWrapper(const std::string & filename);

  ImageWrapper(Halide::Image<float> image, const std::string &name="");

  // Destructor. Because there is no explicit memory management here, this doesn't do anything
  ~ImageWrapper();

  // The number of dimensions in the image (1,2 or 3)
  int dimensions() const { return dims; }
  int extent(int i) const {return dim_values[i]; }

  int width() const {   return dim_values[0];   }//Extent of dimension 0
  int height() const {  return dim_values[1];  }//Extent of dimension 1
  int channels() const {return dim_values[2]; }//Extent of dimension 2

  // realize the Func and write to file
  void write(const std::string & filename);

  // Accessors of the pixel values
  Expr operator()(Expr x, Expr y) const;
  Expr operator()(Expr x, Expr y, Expr c) const;

  Func func;

  // The following are functions and variables that are not accessible from outside the class
 private:
  unsigned int dims; // Number of dimensions
  unsigned int dim_values[3]; // Size of each dimension
  // Common code shared between constructors
  // This does not allocate the image; it only initializes image metadata -
  // image name, width, height, number of channels and number of pixels
  void initialize_image_metadata(int x, int y, int z, const std::string &name_);
  void read_image_data(Halide::Image<float> image);
};

void compareDimensions(const ImageWrapper & im1, const ImageWrapper & im2);
ImageWrapper operator+ (const ImageWrapper & im1, const ImageWrapper & im2);
ImageWrapper operator- (const ImageWrapper & im1, const ImageWrapper & im2);
ImageWrapper operator* (const ImageWrapper & im1, const ImageWrapper & im2);
ImageWrapper operator/ (const ImageWrapper & im1, const ImageWrapper & im2);

ImageWrapper operator+ (const ImageWrapper & im1, const Expr & c);
ImageWrapper operator- (const ImageWrapper & im1, const Expr & c);
ImageWrapper operator* (const ImageWrapper & im1, const Expr & c);
ImageWrapper operator/ (const ImageWrapper & im1, const Expr & c);

ImageWrapper operator+ (const Expr & c, const ImageWrapper & im1);
ImageWrapper operator- (const Expr & c, const ImageWrapper & im1);
ImageWrapper operator* (const Expr & c, const ImageWrapper & im1);
ImageWrapper operator/ (const Expr & c, const ImageWrapper & im1);
