#pragma leco dll

import dotz;
import gerby;
import pocpoc;
import traits;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;
using namespace pocpoc;

// Max to fit a 18650 vruzend 2x3 kit: 70x40. 45x40 also good to be between poles
static constexpr const auto board_w = 45.0_mm;
static constexpr const auto board_h = 30.0_mm;
  
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

// C601092
// https://jlcpcb.com/api/file/downloadByFileSystemAccessId/8588937407022436352
struct r1206 : point {
  static constexpr const auto l = 3.3_mm + 0.2_mm;
  static constexpr const auto w = 1.7_mm + 0.2_mm;
  static constexpr const auto a = 1.2_mm + 0.3_mm;

  static constexpr const point pad { a + def_copper_margin, w + def_copper_margin };

  point pin(int n) const {
    static constexpr const auto px = (l - a / 2) / 2;
    if (n == 1) return point { x + px, y };
    if (n == 2) return point { x - px, y };
    return point {};
  }

  void copper(cnc::pen & p, d::inch margin) const {
    p.aperture(pad.x + margin, pad.y + margin, false);
    p.flash(pin(1));
    p.flash(pin(2));
  };

  void silk(cnc::pen & p) const {
    box(p, x, y, l, w);
  }
};

struct rcem : point {
  // Basing values on their maximum dimensions
  static constexpr const auto l = 22.0_mm + 1.5_mm;
  static constexpr const auto w = 9.5_mm + 1.0_mm;

  // Max diameter + margin
  static constexpr const auto hole = 0.68_mm + 0.05_mm + 0.05_mm;


  point pin(int n) const {
    static constexpr const auto px = (l + 3.0_mm) / 2;
    if (n == 1) return point { x + px, y };
    if (n == 2) return point { x - px, y };
    return point {};
  }

  void copper(cnc::pen & p, d::inch margin) const {
    p.aperture(hole + margin + 1.0_mm);
    p.flash(pin(1));
    p.flash(pin(2));
  }

  void drill(cnc::pen & p) const {
    p.aperture(hole);
    p.flash(pin(1));
    p.flash(pin(2));
  }

  void silk(cnc::pen & p) const {
    box(p, x, y, l, w);
  }
};

const esop8 ic1 {};
enum {
  ic1_temp = 1,
  ic1_prog,
  ic1_gnd,
  ic1_vcc,
  ic1_bat,
  ic1_n_stby,
  ic1_n_chrg,
  ic1_n_ce,
};

// 10nF
const r0603 c_in { ic1.pin(ic1_vcc).plus(1.0_mm, 0) };
// 10nF
const r0603 c_bat { ic1.pin(ic1_bat).plus(1.0_mm, 0) };

const d0603 d_chrg {};
const d0603 d_stby {};

// 0.5 ohms (1W) for heat dissipation
const r1206 r_heat { ic1.plus(0, -8.0_mm) };
// 1.1k for 1A
const r0603 r_prog {};
// 1k
const r0603 r_chrg {};
const r0603 r_stby {};

const header<2> hdr_vcc {};
const header<2> hdr_bat {};

struct compos : generic_layers<compos>, cs<
  ic1,
  c_in, c_bat,
  d_chrg, d_stby,
  r_prog, r_chrg, r_stby, r_heat,
  hdr_vcc, hdr_bat>
{
  static void border(cnc::pen & p, d::inch margin) {
    box(p, 0, 0, board_w, board_h, margin);
  }
};

#ifdef LECO_TARGET_WINDOWS
#define A __declspec(dllexport)
#else
#define A
#endif

extern "C" A void draw(cnc::builder * b, cnc::grb_layer l) {
  compos::draw(b, l);
}

extern "C" A void cpl(cpl::builder * b) {
  b->part({ "R1", r_heat, true, 0 });
  b->part({ "R2", r_prog, true, 0 });
  b->part({ "R3", r_chrg, true, 0 });
  b->part({ "R4", r_stby, true, 0 });
  b->part({ "C1", c_in, true, 0 });
  b->part({ "C2", c_bat, true, 0 });
  b->part({ "D1", d_chrg, true, 0 });
  b->part({ "D2", d_stby, true, 0 });

  b->bom({ "500m", "R1", "1206_R", "C27777" });
  b->bom({ "1.1k", "R2", "0603_R", "" });
  b->bom({ "1k", "R3-4", "0603_R", "" });
  b->bom({ "10uF", "C1-2", "0603_C", "" });
  b->bom({ "RED", "D1", "0603_R", "" });
  b->bom({ "BLUE", "D2", "0603_R", "" });
}
