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
  virtual void doc(gerby::pen &) const {};
  virtual void hole(gerby::pen &) const {};
};

class pad : public compo {
  static constexpr const auto diam = 0.021_in;
  static constexpr const auto hole_d = diam + 0.2_mm;
  static constexpr const auto copper_d = hole_d + 0.6_mm;

  unsigned m_count;

  void pads(auto &p) const {
    for (auto i = 0; i < m_count; i++) {
      p.flash(x(), y() - 0.1_in * i);
    }
  }

public:
  constexpr pad(gerby::d::inch x, gerby::d::inch y, unsigned n)
      : compo{x, y}
      , m_count{n} {}

  void copper(gerby::pen &p) const override {
    p.aperture(copper_d);
    pads(p);
  }
  void hole(gerby::pen &p) const override {
    p.aperture(hole_d);
    pads(p);
  }
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

protected:
  auto center_x() const { return x() + (d - b) * 0.5f; }

public:
  using compo::compo;

  void copper(gerby::pen &p) const override { pads(p); }
};
class led : public r0805 {
public:
  using r0805::r0805;

  void doc(gerby::pen &p) const override {
    p.aperture(10.0_mil);
    p.move(center_x() - 6.0_mil, y() - 8.0_mil);
    p.draw_y(y() + 8.0_mil);
    p.draw(center_x() + 6.0_mil, y());
    p.draw(center_x() - 6.0_mil, y() - 8.0_mil);
  }
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
  void doc(gerby::pen &p) const override {
    p.aperture(10.0_mil, 30.0_mil, true);
    p.flash(x() + pw, y());
  }
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
  static constexpr const led l1{7.0_mm, 6.0_mm};
  static constexpr const soic_8 ne555{-10.0_mm, 0};
  static constexpr const pad bat{-10.0_mm, 5.0_mm, 2};

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
          bat.copper(p);
        },
        red);
    b->add_lines([](auto &p) { bat.hole(p); }, black);
    b->add_lines(
        [](auto &p) {
          ne555.doc(p);
          l1.doc(p);
        },
        white);
  }};
  t.handle(e);
}
