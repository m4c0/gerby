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

  static constexpr const point pad { 0.9_mm, 0.5_mm };

  dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;

  point pin(int n) const {
    auto [dx, dy] = pin_fn(n, N);
    return { x + (3.1_mm - 0.45_mm) * dx, y + 1.27_mm * (dy - h) };
  }

  void copper(cnc::pen & p, d::inch m) const {
    p.aperture(0.9_mm + m, 0.5_mm + m, false);
    for (auto i = 0; i < N; i++) p.flash(pin(i + 1));

    p.aperture(2.5_mm + m, 3.4_mm + m, false);
    p.flash(*this);
  }

  void silk(cnc::pen & p) const {
    box(p, x, y, 3.9_mm, 4.9_mm);

    p.aperture(0.3_mm);
    p.flash(pin(1).plus(1.2_mm, 0));
  }
};

struct sot23_6 : point {
  static constexpr const auto N = 6;

  static constexpr const auto h = N / 4.0 - 0.5;

  static constexpr const point pad { 0.9_mm, 0.5_mm };

  dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;

  point pin(int n) const {
    auto [dx, dy] = pin_fn(n, N);
    return { x + (0.8_mm + 0.3_mm) * dx, y + 0.95_mm * (dy - h) };
  }

  void copper(cnc::pen & p, d::inch m) const {
    p.aperture(0.7_mm + m, 0.4_mm + m, false);
    for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
  }

  void silk(cnc::pen & p) const {
    box(p, x, y, 1.6_mm, 2.9_mm);

    p.aperture(0.3_mm);
    p.flash(pin(1).plus(1.2_mm, 0));
  }
};

// https://jlcpcb.com/partdetail/Hubei_KENTOElec-KT0603R/C2286
struct d0603 : r0603 {
  void silk(cnc::pen & p) const {
    p.aperture(0.25_mm);

    turtle t { &p };
    t.move(pin(1).plus(-0.6_mm, -0.4_mm));
    t.draw(pin(1).plus(-0.6_mm, 0.4_mm));
    t.move(pin(1).plus(-0.6_mm, 0));
    t.draw(pin(2).plus(0.6_mm, 0));
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

// TP4056
const esop8 ic1 {{ -10.0_mm, 0 }};
enum {
  ic1_temp = 1,
  ic1_prog,
  ic1_gnd,
  ic1_vcc,
  ic1_bat,
  ic1_n_stby,
  ic1_n_chrg,
  ic1_ce,
};

// DW01A - Battery Protection
const sot23_6 ic2 { ic1.plus(15.0_mm, 0) };
enum {
  ic2_od = 1,
  ic2_cs,
  ic2_oc,
  ic2_vcc = 5,
  ic2_gnd,
};

// 10nF
const r0603 c_in { ic1.pin(ic1_vcc).plus(-2.5_mm, 0) };
// 10nF
const r0603 c_bat { ic1.pin(ic1_bat).plus(0.75_mm, -2.0_mm) };
// 100uF
const r0603 c_dw { ic2.pin(ic2_vcc).plus(2.5_mm, 0) };

// 0.5 ohms (1W) for heat dissipation
const r1206 r_heat { ic1.plus(0, -8.0_mm) };
// 1.1k for 1A
const r0603 r_prog { ic1.pin(ic1_prog).plus(-2.5_mm, 0) };
// 1k
const r0603 r_chrg { ic1.pin(ic1_n_chrg).plus(2.5_mm, 0) };
const r0603 r_stby { ic1.pin(ic1_n_stby).plus(2.5_mm, 0) };
// 2k
const r0603 r_dw_cs { ic2.pin(ic2_cs).plus(-2.0_mm, 0) };
// 470 ohms
const r0603 r_dw_vcc { c_dw.plus(0, 1.5_mm) };

const d0603 d_chrg { r_chrg.plus(3.0_mm, 0) };
const d0603 d_stby { r_stby.plus(3.0_mm, 0) };

const header<2> hdr_vcc { r_heat.plus(-6.0_mm, 0) };
const header<2> hdr_bat { ic1.plus(0, 8.0_mm) };

const via v_vcc_1 { d_stby.pin(1).plus(0, -1.0_mm) };
const via v_vcc_2 { r_heat.pin(1).plus(2.0_mm, 0) };

struct compos : generic_layers<compos>, cs<
  ic1, ic2,
  c_in, c_bat, c_dw,
  d_chrg, d_stby,
  r_prog, r_chrg, r_stby, r_heat, r_dw_vcc, r_dw_cs,
  hdr_vcc, hdr_bat,
  v_vcc_1, v_vcc_2>
{
  static void border(cnc::pen & p, d::inch margin) {
    box(p, 0, 0, board_w, board_h, margin);
  }

  static void top_nets(cnc::pen & p, d::inch m) {
    turtle t { &p };

    t.move(ic1.pin(ic1_vcc));
    t.draw(c_in.pin(1));

    t.move(ic1.pin(ic1_bat));
    t.draw(c_bat.pin(2));

    t.move(ic1.pin(ic1_prog));
    t.draw(r_prog.pin(1));

    t.move(ic1.pin(ic1_n_stby));
    t.draw(r_stby.pin(2));
    t.move(r_stby.pin(1));
    t.draw(d_stby.pin(2));

    t.move(ic1.pin(ic1_n_chrg));
    t.draw(r_chrg.pin(2));
    t.move(r_chrg.pin(1));
    t.draw(d_chrg.pin(2));

    t.move(ic1.pin(ic1_ce));
    t.draw_ld(d_chrg.pin(1));
    t.draw(d_stby.pin(1));
    t.draw(v_vcc_1);

    t.move(v_vcc_2);
    t.draw(r_heat.pin(1));

    t.move(ic2.pin(ic2_vcc));
    t.draw(c_dw.pin(2));
    t.draw(r_dw_vcc.pin(2));

    p.aperture(0.5_mm + m);
    t.move(hdr_vcc.pin(2));
    t.draw(r_heat.pin(2));

    t.move(r_heat.pin(1));
    t.draw(r_heat.pin(1).plus(0, 2.0_mm));
    t.draw(ic1.pin(ic1_vcc));
  }

  static void bottom_nets(cnc::pen & p, d::inch m) {
    turtle t { &p };

    t.move(v_vcc_1);
    t.draw(v_vcc_1.plus(0, -1.0_mm));
    t.draw(v_vcc_2);
  }

  static void top_thermals(cnc::pen & p) {
    thermal(p, hdr_bat, 1);
    thermal(p, hdr_vcc, 1);
    thermal(p, ic1, ic1_gnd);
    thermal(p, ic1, ic1_temp);
    thermal(p, r_prog, 2);
    thermal(p, r_dw_cs, 2);
    thermal(p, c_in, 2);
    thermal(p, c_bat, 1);
    thermal(p, c_dw, 1);
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
