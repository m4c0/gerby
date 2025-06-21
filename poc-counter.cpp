#pragma leco dll
import gerby;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;

//static constexpr const auto copper_margin = 15.0_mil;

// https://jlcpcb.com/partdetail/23933-0603WAF5602T5E/C23206
struct r0603 {
  d::inch x;
  d::inch y;
};
static void r0603_copper(cnc::pen & p, r0603 r) {
  static constexpr const auto a = 0.90_mm;
  static constexpr const auto b = 0.65_mm;
  static constexpr const auto c = 0.80_mm;

  static constexpr const auto px = (a + b) / 2;

  p.aperture(b, c, false);
  p.flash(r.x + px, r.y);
  p.flash(r.x - px, r.y);
};
static void r0603_silk(cnc::pen & p, r0603 r) {
  static constexpr const auto l = 1.6_mm;
  static constexpr const auto w = 0.8_mm;

  p.aperture(6.0_mil);
  p.move(r.x - l / 2, r.y - w / 2);
  p.draw_x(r.x + l / 2);
  p.draw_y(r.y + w / 2);
  p.draw_x(r.x - l / 2);
  p.draw_y(r.y - w / 2);
};

// 100k
const auto r1 = r0603(0.0_mm, 0.0_mm);
const auto r2 = r0603(0.0_mm, 0.0_mm);
const auto r3 = r0603(0.0_mm, 0.0_mm);
const auto r4 = r0603(0.0_mm, 0.0_mm);
// 68
const auto r5 = r0603(0.0_mm, 0.0_mm);
const auto r6 = r0603(0.0_mm, 0.0_mm);
const auto r7 = r0603(0.0_mm, 0.0_mm);
const auto r8 = r0603(0.0_mm, 0.0_mm);
const auto r9 = r0603(0.0_mm, 0.0_mm);
const auto r10 = r0603(0.0_mm, 0.0_mm);
const auto r11 = r0603(0.0_mm, 0.0_mm);

// 1nF
const auto c1 = r0603(0.0_mm, 0.0_mm);
// 10nF
const auto c2 = r0603(0.0_mm, 0.0_mm);

void penny(cnc::pen & p, void (*fn)(cnc::pen &, r0603)) {
  fn(p, r1);
  fn(p, r2);
  fn(p, r3);
  fn(p, r4);
  fn(p, r5);
  fn(p, r6);
  fn(p, r7);
  fn(p, r8);
  fn(p, r9);
  fn(p, r10);
  fn(p, r11);
  fn(p, c1);
  fn(p, c2);
}

void top_copper(cnc::pen & p) {
  penny(p, r0603_copper);
}
void top_silk(cnc::pen & p) {
  penny(p, r0603_silk);
}

extern "C" void draw(cnc::builder * b, cnc::grb_layer l) {
  switch (l) {
    case cnc::gl_top_copper:
      b->add_lines(top_copper, red);
      break;
    case cnc::gl_top_silk:
      b->add_lines(top_silk, white);
      break;
    default: break;
  }
}
