#pragma leco tool

import dl;
import gerby;
import print;

using namespace gerby;

int main(int argc, char ** argv) {
  if (argc != 2) {
    errfn("Usage: %s <libname>", argv[0]);
    return 1;
  }

  errfn("Preparing to generate with %s", argv[1]);

  auto dll = dl::open(argv[1]);
  if (!dll) {
    errfn("Failed to load library [%s]", argv[1]);
    return 1;
  }

  auto fn = dll->fn<gerby::out::lb_t>("draw");
  if (!fn) {
    errfn("Missing draw function in [%s]", argv[1]);
    return 1;
  }

  gerby::out::write(fn);

  auto cpl_fn = dll->fn<cpl::b_t>("cpl");
  if (!cpl_fn) {
    errfn("Missing pick-n-place function in [%s] - bailing out", argv[1]);
    return 0;
  }

  cpl::write(cpl_fn);

  return 0;
}
