#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

class compo {
  gerby::d::inch m_x;
  gerby::d::inch m_y;

protected:
  [[nodiscard]] constexpr auto x() const noexcept { return m_x; }
  [[nodiscard]] constexpr auto y() const noexcept { return m_y; }

public:
  constexpr compo(gerby::d::inch x, gerby::d::inch y) : m_x{x}, m_y{y} {}

  virtual void copper(gerby::pen &) const = 0;
};

class r0805 : public compo {
  static constexpr const auto a = 1.0_mm;
  static constexpr const auto b = 1.0_mm;
  static constexpr const auto c = 1.3_mm;
  static constexpr const auto d = 3.0_mm;

  void pads(auto &p) const {
    p.aperture(b, c, false);
    p.flash(x(), y());
    p.flash_x(x() + d - b);
  }

public:
  using compo::compo;

  void copper(gerby::pen &p) const override { pads(p); }
};

class soic_8 : public compo {
  static constexpr const auto pw = 1.55_mm;
  static constexpr const auto ph = 0.60_mm;
  static constexpr const auto dx = 5.40_mm;
  static constexpr const auto dy = 1.27_mm;

  void pads(auto &p) const {
    p.aperture(pw, ph, false);
    for (auto i = 0; i < 4; i++) {
      p.flash(x(), y() - dy * i);
      p.flash_x(x() + dx);
    }
  }

public:
  using compo::compo;

  void copper(gerby::pen &p) const override { pads(p); }
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  // https://datasheet.lcsc.com/lcsc/2205311800_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.pdf
  // https://datasheet.lcsc.com/lcsc/2304140030_FH--Guangdong-Fenghua-Advanced-Tech-0805B471K500NT_C1743.pdf
  // https://datasheet.lcsc.com/lcsc/1806151129_Hubei-KENTO-Elec-KT-0805Y_C2296.pdf
  // https://www.ti.com/lit/ds/symlink/lm555.pdf
  static constexpr const r0805 r1{0, 0};
  static constexpr const r0805 r2{0, 2.0_mm};
  static constexpr const r0805 r3{0, 4.0_mm};
  static constexpr const r0805 c1{7.0_mm, 1.0_mm};
  static constexpr const r0805 c2{7.0_mm, 3.0_mm};
  static constexpr const r0805 l1{7.0_mm, 6.0_mm};
  static constexpr const soic_8 ne555{-10.0_mm, 0};

  static gerby::thread t{[](auto b) {
    b->add_lines(
        [](auto &p) {
          r1.copper(p);
          r2.copper(p);
          r3.copper(p);
          c1.copper(p);
          c2.copper(p);
          l1.copper(p);
          ne555.copper(p);
        },
        red);
  }};
  t.handle(e);
}
