#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

// https://datasheet.lcsc.com/lcsc/2205311800_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.pdf
// https://datasheet.lcsc.com/lcsc/2304140030_FH--Guangdong-Fenghua-Advanced-Tech-0805B471K500NT_C1743.pdf
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

// https://www.ti.com/lit/ds/symlink/lm555.pdf
class soic_8 {
  static constexpr const auto pw = (1.55_mm).value();
  static constexpr const auto ph = (0.60_mm).value();
  static constexpr const auto dx = (5.40_mm).value();
  static constexpr const auto dy = (1.27_mm).value();

  float m_x;
  float m_y;

  void pads(auto &p) const {
    p.aperture(pw, ph, false);
    for (auto i = 0; i < 4; i++) {
      p.flash(m_x, m_y - dy * i);
      p.flash_x(m_x + dx);
    }
  }

public:
  constexpr soic_8(float x, float y) : m_x{x}, m_y{y} {}

  void copper(auto &p) const { pads(p); }
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  static constexpr const r0805 r1{0, 0};
  static constexpr const r0805 r2{0, (2.0_mm).value()};
  static constexpr const r0805 r3{0, (4.0_mm).value()};
  static constexpr const r0805 c1{(5.0_mm).value(), 0};
  static constexpr const r0805 c2{(5.0_mm).value(), 0};
  static constexpr const soic_8 ne555{(-10.0_mm).value(), 0};

  static gerby::thread t{[](auto b) {
    b->add_lines(
        [](auto &p) {
          r1.copper(p);
          r2.copper(p);
          r3.copper(p);
          c1.copper(p);
          c2.copper(p);
          ne555.copper(p);
        },
        red);
  }};
  t.handle(e);
}
