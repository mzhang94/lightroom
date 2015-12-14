#include "Halide.h"
using namespace Halide;

void apply_default_schedule(Func F) {
    std::map<std::string,Internal::Function> flist = Internal::find_transitive_calls(F.function());
    flist.insert(std::make_pair(F.name(), F.function()));
    std::map<std::string,Internal::Function>::iterator fit;
    for (fit=flist.begin(); fit!=flist.end(); fit++) {
        Func f(fit->second);
        f.compute_root();
        std::cout << "Warning: applying default schedule to " << f.name() << std::endl;
    }
    std::cout << std::endl;
}

void schedule_compute_root(Func f)
{
  Var yi, yo;
  // f.compute_root().split(f.args()[1], yo, yi, 32).parallel(yo).vectorize(f.args()[0], 8);
  // f.compute_root().split(f.args()[1], yo, yi, 64).parallel(yo).vectorize(f.args()[0], 8);
  f.compute_root().parallel(f.args()[1]).vectorize(f.args()[0], 8);
}
