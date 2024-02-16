#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

// https://datasheet.lcsc.com/lcsc/2205311800_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.pdf
class r0805 {
  static constexpr const auto a = (1.0_mm).value();
  static constexpr const auto b = (1.0_mm).value();
  static constexpr const auto c = (1.3_mm).value();
  static constexpr const auto d = (3.0_mm).value();

  float m_x;
  float m_y;

  void pads(auto &p) const {
    p.aperture(b, c, false);
    p.flash(m_x, m_y);
    p.flash_x(d - b + m_x);
  }

public:
  constexpr r0805(float x, float y) : m_x{x}, m_y{y} {}

  void copper(auto &p) const { pads(p); }
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  static constexpr const r0805 r1{0, 0};
  static constexpr const r0805 r2{0, (2.0_mm).value()};
  static constexpr const r0805 r3{0, (4.0_mm).value()};

  static gerby::thread t{[](auto b) {
    b->add_lines(
        [](auto &p) {
          r1.copper(p);
          r2.copper(p);
          r3.copper(p);
        },
        red);
  }};
  t.handle(e);
}
