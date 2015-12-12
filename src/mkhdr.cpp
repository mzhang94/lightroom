#include "Halide.h"
#include "point.h"
#include "stencil.h"
#include "util.h"
#include "apps.h"
using namespace Halide;
using namespace Halide::Tools;

Func computeWeight(Func images, float epsilonMin, float epsilonMax, int k)
{
  Var x("x"), y("y"), c("c"), i("i");
  Func weights("weights");
  Func min("min");
  Func max("max");
  min(i) = select(i==k-1, 0.0f, epsilonMin);
  max(i) = select(i==0, 1.0f, epsilonMax);
  weights(x,y,c,i) = select(images(x,y,c,i) >= min(i) && images(x,y,c,i) <= max(i), 1.0f, 0.0f);

  //schedule
  min.compute_root().unroll(i);
  max.compute_root().unroll(i);
  weights.reorder(i, c, x, y).vectorize(x, 16).parallel(y).unroll(i);
  return weights;
}

Func computeFactors(Func images, Func weights, int width, int height)
{
  Var x,y,c,i;
  Func valid("valid");
  valid(x,y,c,i) = weights(x,y,c,i)*weights(x,y,c,i+1);
  Func ratio("ratio");
  ratio(x,y,c,i) = select(valid(x,y,c,i)!=0, images(x,y,c,i+1)/(images(x,y,c,i)+0.0000001f), 0);
  Func sum("sum"), count("count"), factor("factor");
  sum(i) = 0.0f;
  count(i) = 0.0f;
  RDom r(0, width, 0, height, 0, 3);
  sum(i) += ratio(r.x,r.y,r.z,i);
  count(i) += valid(r.x,r.y,r.z,i);
  //use mean instead of median
  factor(i) = sum(i)/count(i);

  //schedule
  sum.compute_root();
  count.compute_root();

  sum.update().unroll(i);
  count.update().unroll(i);
  return factor;
}

Func mkhdr(Func images, float epsilonMin, float epsilonMax, int width, int height, int k)
{
  Var x("x"), y("y"), c("c"), i("i");
  Func gammaCorrected("gammaCorrected");
  gammaCorrected(x,y,c,i) = pow(images(x,y,c,i), 2.2f);

  Func weights = computeWeight(gammaCorrected, epsilonMin, epsilonMax, k);
  Func factor_ratio = computeFactors(gammaCorrected, weights, width, height);

  Func factors("factors");
  factors(i) = 1.0f;
  RDom r(1,k-1);
  factors(r.x) = factor_ratio(r.x-1) * factors(r.x-1);

  Func sumOfWeights("sumOfWeights");
  RDom r1(0,k);
  sumOfWeights(x,y,c) = sum(weights(x,y,c,r1.x));

  Func unnormalized("unnormalized"), output("hdr");
  unnormalized(x,y,c) = sum(weights(x,y,c,r1.x)/factors(r1.x)*gammaCorrected(x,y,c,r1.x));
  output(x,y,c) = select(sumOfWeights(x,y,c)>0, unnormalized(x,y,c) / sumOfWeights(x,y,c), gammaCorrected(x,y,c,0));

  //schedule
  gammaCorrected.compute_root();
  gammaCorrected.reorder(i, c, x, y).unroll(i).vectorize(x, 16).parallel(y);
  weights.compute_root();
  factor_ratio.compute_root();
  factors.compute_root();

  sumOfWeights.compute_at(output,y).vectorize(x, 16);
  unnormalized.compute_at(output,y).vectorize(x, 16);
  output.compute_root().parallel(y).vectorize(x,16);
  return output;
}
