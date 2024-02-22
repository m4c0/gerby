#pragma leco tool

import dl;
import gerby;
import silog;

int main(int argc, char ** argv) {
  if (argc != 2) {
    silog::log(silog::error, "Usage: %s <libname>", argv[0]);
    return 1;
  }

  silog::log(silog::info, "Preparing to generate with %s", argv[1]);

  auto dll = dl::open(argv[1]);
  if (!dll) {
    silog::log(silog::error, "Failed to load library [%s]", argv[1]);
    return 1;
  }

  auto fn = dll->fn<gerby::out::lb_t>("draw");
  if (!fn) {
    silog::log(silog::error, "Missing draw function in [%s]", argv[1]);
    return 1;
  }

  gerby::out::write(fn);

  return 0;
}
