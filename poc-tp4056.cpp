#pragma leco dll

import dotz;
import gerby;
import pocpoc;
import traits;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;
using namespace pocpoc;

// https://lcsc.com/datasheet/lcsc_datasheet_2410121619_TOPPOWER-Nanjing-Extension-Microelectronics-TP4056-42-ESOP8_C16581.pdf
struct esop8 : point {
  static constexpr const auto N = 8;

  static constexpr const auto h = N / 4.0 - 0.5;

  dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;

  point pin(int n) const {
    auto [dx, dy] = pin_fn(n, N);
    return { x + (3.1_mm - 0.45_mm) * dx, y + 1.27_mm * (dy - h) };
  }

  void copper(cnc::pen & p, d::inch m) const {
    p.aperture(0.9_mm + m, 0.5_mm + m, false);
    for (auto i = 0; i < N; i++) p.flash(pin(i + 1));

    p.aperture(2.5_mm + def_copper_margin, 3.4_mm + def_copper_margin, false);
    p.flash(*this);
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
