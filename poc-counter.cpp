#pragma leco dll
import gerby;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;

//static constexpr const auto copper_margin = 15.0_mil;

void box(cnc::pen & p, d::inch cx, d::inch cy, d::inch w, d::inch h) {
  p.aperture(6.0_mil);
  p.move(cx - w / 2, cy - h / 2);
  p.draw_x(cx + w / 2);
  p.draw_y(cy + h / 2);
  p.draw_x(cx - w / 2);
  p.draw_y(cy - h / 2);
}

namespace l {
  struct copper {};
  struct holes {};
  struct silk {};
}
template<typename L, typename T> void penpen(cnc::pen & p, L, T t) {
}

struct point {
  // Aligned at the center of the compo
  d::inch x;
  d::inch y;
};

// https://jlcpcb.com/partdetail/23933-0603WAF5602T5E/C23206
struct r0603 : point {};
template<> void penpen(cnc::pen & p, l::copper, r0603 r) {
  static constexpr const auto a = 0.90_mm;
  static constexpr const auto b = 0.65_mm;
  static constexpr const auto c = 0.80_mm;

  static constexpr const auto px = (a + b) / 2;

  p.aperture(b, c, false);
  p.flash(r.x + px, r.y);
  p.flash(r.x - px, r.y);
};
template<> void penpen(cnc::pen & p, l::silk, r0603 r) {
  static constexpr const auto l = 1.6_mm;
  static constexpr const auto w = 0.8_mm;
  box(p, r.x, r.y, l, w);
};

// https://jlcpcb.com/partdetail/2503-S8050_J3Y_RANGE_200_350/C2146
struct sot23 : point {};
template<> void penpen(cnc::pen & p, l::copper, sot23 r) {
  static constexpr const auto h = 2.02_mm / 2;
  static constexpr const auto w = 1.20_mm / 2;

  p.aperture(0.6_mm, 0.8_mm, false);
  p.flash(r.x, r.y - h);
  p.flash(r.x - w, r.y + h);
  p.flash(r.x + w, r.y + h);
}
template<> void penpen(cnc::pen & p, l::silk, sot23 r) {
  static constexpr const auto l = 2.9_mm;
  static constexpr const auto w = 1.3_mm;
  box(p, r.x, r.y, l, w);
}

struct dip14 : point {};
void dip14_copper(cnc::pen & p, dip14 r, d::inch margin) {
  static constexpr const auto pin = 0.5_mm;
  static constexpr const auto hole = pin + 0.2_mm;

  static constexpr const auto w = 0.3_in / 2;
  static constexpr const auto h = 0.1_in * 3.0;

  p.aperture(hole + margin);
  for (auto i = 0; i < 7; i++) {
    p.flash(r.x - w, r.y - h + 0.1_in * i);
    p.flash(r.x + w, r.y - h + 0.1_in * i);
  }
}
template<> void penpen(cnc::pen & p, l::copper, dip14 r) {
  dip14_copper(p, r, 0.6_mm);
}
template<> void penpen(cnc::pen & p, l::holes, dip14 r) {
  dip14_copper(p, r, 0.0_mm);
}
template<> void penpen(cnc::pen & p, l::silk, dip14 r) {
  box(p, r.x, r.y, 0.3_in, 0.1_in * 7);
}

// TODO: unify dip14/16
struct dip16 : point {};
void dip16_copper(cnc::pen & p, dip16 r, d::inch margin) {
  static constexpr const auto pin = 0.5_mm;
  static constexpr const auto hole = pin + 0.2_mm;

  static constexpr const auto w = 0.3_in / 2;
  static constexpr const auto h = 0.1_in * 3.5;

  p.aperture(hole + margin);
  for (auto i = 0; i < 8; i++) {
    p.flash(r.x - w, r.y - h + 0.1_in * i);
    p.flash(r.x + w, r.y - h + 0.1_in * i);
  }
}
template<> void penpen(cnc::pen & p, l::copper, dip16 r) {
  dip16_copper(p, r, 0.6_mm);
}
template<> void penpen(cnc::pen & p, l::holes, dip16 r) {
  dip16_copper(p, r, 0.0_mm);
}
template<> void penpen(cnc::pen & p, l::silk, dip16 r) {
  box(p, r.x, r.y, 0.3_in, 0.1_in * 8);
}

// 100k
const auto r1 = r0603({0.0_mm, 0.0_mm});
const auto r2 = r0603({0.0_mm, 0.0_mm});
const auto r3 = r0603({0.0_mm, 0.0_mm});
const auto r4 = r0603({0.0_mm, 0.0_mm});
// 68
const auto r5 = r0603({0.0_mm, 0.0_mm});
const auto r6 = r0603({0.0_mm, 0.0_mm});
const auto r7 = r0603({0.0_mm, 0.0_mm});
const auto r8 = r0603({0.0_mm, 0.0_mm});
const auto r9 = r0603({0.0_mm, 0.0_mm});
const auto r10 = r0603({0.0_mm, 0.0_mm});
const auto r11 = r0603({0.0_mm, 0.0_mm});

// 1nF
const auto c1 = r0603({0.0_mm, 0.0_mm});
// 10nF
const auto c2 = r0603({0.0_mm, 0.0_mm});

// BJT NPN
const auto q1 = sot23({0.0_mm, 3.0_mm});
const auto q2 = sot23({0.0_mm, 3.0_mm});
const auto q3 = sot23({0.0_mm, 3.0_mm});

// 7-digit displays
const auto msd = dip14({-12.0_mm, -10.0_mm});
const auto nsd = dip14({  0.0_mm, -10.0_mm});
const auto lsd = dip14({ 12.0_mm, -10.0_mm});

// MC14553 - 3-digit BCD counter
const auto ic1 = dip16({-6.0_mm, 10.0_mm});
// MC14511 - BCD to 7 segments
const auto ic2 = dip16({ 6.0_mm, 10.0_mm});

template<typename T>
void pennies(cnc::pen & p, T t, auto... cs) {
  (penpen(p, t, cs), ...);
}
template<typename T>
void penny(cnc::pen & p, T t) {
  pennies(p, t,
      r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11,
      c1, c2,
      q1, q2, q3,
      msd, nsd, lsd,
      ic1, ic2);
}

void top_copper(cnc::pen & p) {
  penny(p, l::copper {});
}
void top_silk(cnc::pen & p) {
  penny(p, l::silk {});
}
void holes(cnc::pen & p) {
  penny(p, l::holes {});
}

void border_margin(cnc::pen & p) {
  box(p, 0, 0, 40.0_mm + 25.0_mil, 40.0_mm + 25.0_mil);
}
void border(cnc::pen & p) {
  box(p, 0, 0, 40.0_mm, 40.0_mm);
}

extern "C" void draw(cnc::builder * b, cnc::grb_layer l) {
  switch (l) {
    case cnc::gl_top_copper:
      b->add_lines(top_copper, red);
      b->add_lines(holes, black);
      break;
    case cnc::gl_top_silk:
      b->add_lines(top_silk, white);
      break;
    case cnc::gl_border:
      b->add_lines(border_margin, black);
      b->add_lines(border, purple);
      break;
    case cnc::gl_drill_holes:
      b->add_lines(holes, white);
      break;
    default: break;
  }
}
