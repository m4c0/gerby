#pragma leco dll

import dotz;
import gerby;
import pocpoc;
import traits;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;
using namespace pocpoc;

struct esop8 : point {
  static constexpr const auto N = 8;

  static constexpr const auto w = 0.3_in / 2;
  static constexpr const auto h = 0.1_in * (N / 4.0 - 0.5);

  static constexpr const auto b = 0.65_mm;
  static constexpr const auto c = 0.80_mm;

  dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;

  point pin(int n) const {
    auto [dx, dy] = pin_fn(n, N);
    return { x + w * dx, y - h + 0.1_in * dy };
  }

  void copper(cnc::pen & p, d::inch m) const {
    p.aperture(b + m, c + m, false);
    for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
  }
};

const esop8 ic1 { {} };

struct compos : generic_layers<compos>, cs<
  ic1>
{};

#ifdef LECO_TARGET_WINDOWS
#define A __declspec(dllexport)
#else
#define A
#endif

extern "C" A void draw(cnc::builder * b, cnc::grb_layer l) {
  compos::draw(b, l);
}

extern "C" A void cpl(cpl::builder * b) {
}
