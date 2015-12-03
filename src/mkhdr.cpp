#include "Halide.h"
#include "point.h"
#include "stencil.h"
#include "util.h"
#include "apps.h"
using namespace Halide;
using namespace Halide::Tools;

Func computeWeight(Func images, float epsilonMin, float epsilonMax, int k)
{
  Var x, y, c, i;
  Func weights("weights");
  Func min;
  Func max;
  min(i) = select(i==k-1, 0.0f, epsilonMin);
  max(i) = select(i==0, 1.0f, epsilonMax);
  weights(x,y,c,i) = select(images(x,y,c,i) >= min(i) && images(x,y,c,i) <= max(i), 1.0f, 0.0f);
  return weights;
}

Func computeFactors(Func images, Func weights, int width, int height)
{
  Var x,y,c,i;
  Func valid;
  valid(x,y,c,i) = weights(x,y,c,i)*weights(x,y,c,i+1);
  Func ratio;
  ratio(x,y,c,i) = select(valid(x,y,c,i)!=0, images(x,y,c,i+1)/(images(x,y,c,i)+0.0000001f), 0);
  Func sum("sum"), count("count"), factor("factor");
  sum(i) = 0.0f;
  count(i) = 0.0f;
  RDom r(0, width, 0, height, 0, 3);
  sum(i) += ratio(r.x,r.y,r.z,i);
  count(i) += valid(r.x,r.y,r.z,i);
  //use mean instead of median
  factor(i) = sum(i)/count(i);
  return factor;
}

Func mkhdr(Func images, float epsilonMin, float epsilonMax, int width, int height, int k)
{
  Var x, y, c, i;
  Func gammaCorrected("gammaCorrected");
  gammaCorrected(x,y,c,i) = pow(images(x,y,c,i), 2.2f);

  Func weights = computeWeight(gammaCorrected, epsilonMin, epsilonMax, k);
  Func factor_ratio = computeFactors(gammaCorrected, weights, width, height);

  Func factors("factors");
  factors(i) = 1.0f;
  RDom r(1,k-1);
  factors(r.x) = factor_ratio(r.x-1) * factors(r.x-1);
  Image<float> factors_ = factors.realize(k);
  for (int i = 0; i < k; i++)
  {
    std::cout << factors_(i) << "\n";
  }

  Func sumOfWeights;
  RDom r1(0,k);
  sumOfWeights(x,y,c) = sum(weights(x,y,c,r1.x));

  Func unnormalized("unnormalized"), output("output");
  unnormalized(x,y,c) = sum(weights(x,y,c,r1.x)/factors(r1.x)*gammaCorrected(x,y,c,r1.x));
  output(x,y,c) = select(sumOfWeights(x,y,c)>0, unnormalized(x,y,c) / sumOfWeights(x,y,c), gammaCorrected(x,y,c,0));
  return output;
}
