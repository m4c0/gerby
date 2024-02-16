#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

enum rotation { NONE, CW90, CW180, CW270 };

class compo {
  gerby::d::inch m_x;
  gerby::d::inch m_y;
  rotation m_rot{};

protected:
  [[nodiscard]] constexpr auto x() const noexcept { return m_x; }
  [[nodiscard]] constexpr auto y() const noexcept { return m_y; }
  [[nodiscard]] constexpr auto rot() const noexcept { return m_rot; }

  void flash_pins(auto &p, unsigned max) const {
    for (auto i = 0; i < max; i++) {
      p.flash(pin_x(i), pin_y(i));
    }
  }

  virtual gerby::d::inch pin_rel_x(unsigned) const = 0;
  virtual gerby::d::inch pin_rel_y(unsigned) const = 0;

public:
  constexpr compo(gerby::d::inch x, gerby::d::inch y, rotation r = NONE)
      : m_x{x}
      , m_y{y}
      , m_rot{r} {}

  virtual void copper(gerby::pen &) const = 0;
  virtual void doc(gerby::pen &) const {};
  virtual void hole(gerby::pen &) const {};

  gerby::d::inch pin_x(unsigned i) const {
    switch (m_rot) {
    case NONE:
      return x() + pin_rel_x(i);
    case CW90:
      return x() + pin_rel_y(i);
    case CW180:
      return x() - pin_rel_x(i);
    case CW270:
      return x() - pin_rel_y(i);
    }
  }
  gerby::d::inch pin_y(unsigned i) const {
    switch (m_rot) {
    case NONE:
      return y() + pin_rel_y(i);
    case CW90:
      return y() + pin_rel_x(i);
    case CW180:
      return y() - pin_rel_y(i);
    case CW270:
      return y() - pin_rel_x(i);
    }
  }
};

class pad : public compo {
  static constexpr const auto diam = 0.021_in;
  static constexpr const auto hole_d = diam + 0.2_mm;
  static constexpr const auto copper_d = hole_d + 0.6_mm;

  unsigned m_count;

  gerby::d::inch pin_rel_x(unsigned i) const override { return 0; }
  gerby::d::inch pin_rel_y(unsigned i) const override { return -0.1_in * i; }

public:
  constexpr pad(gerby::d::inch x, gerby::d::inch y, unsigned n)
      : compo{x, y}
      , m_count{n} {}

  void copper(gerby::pen &p) const override {
    p.aperture(copper_d);
    flash_pins(p, m_count);
  }
  void hole(gerby::pen &p) const override {
    p.aperture(hole_d);
    flash_pins(p, m_count);
  }
};

class r0805 : public compo {
  static constexpr const auto a = 1.0_mm;
  static constexpr const auto b = 1.0_mm;
  static constexpr const auto c = 1.3_mm;
  static constexpr const auto d = 3.0_mm;

protected:
  auto center_x() const { return x() + (d - b) * 0.5f; }

  gerby::d::inch pin_rel_x(unsigned i) const override { return (d - b) * i; }
  gerby::d::inch pin_rel_y(unsigned i) const override { return 0; }

public:
  using compo::compo;

  void copper(gerby::pen &p) const override {
    if (rot() == CW90 || rot() == CW270) {
      p.aperture(c, b, false);
    } else {
      p.aperture(b, c, false);
    }
    flash_pins(p, 2);
  }
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

  gerby::d::inch pin_rel_x(unsigned i) const override { return dx * (i / 4); }
  gerby::d::inch pin_rel_y(unsigned i) const override {
    if (i < 4) {
      return -dy * i;
    } else {
      return -dy * (7 - i);
    }
  }

public:
  using compo::compo;

  void copper(gerby::pen &p) const override {
    p.aperture(pw, ph, false);
    flash_pins(p, 8);
  }
  void doc(gerby::pen &p) const override {
    p.aperture(10.0_mil, 30.0_mil, true);
    p.flash(x() + pw, y());
  }
};

class turtle {
  gerby::pen *m_pen;

  gerby::d::inch m_x{};
  gerby::d::inch m_y{};

public:
  explicit constexpr turtle(gerby::pen *p) : m_pen{p} {}

  void move(const compo &c, unsigned pin) {
    m_x = c.pin_x(pin - 1);
    m_y = c.pin_y(pin - 1);
    m_pen->move(m_x, m_y);
  }

  void draw(const compo &c, unsigned pin) {
    auto nx = c.pin_x(pin - 1);
    auto ny = c.pin_y(pin - 1);

    auto dx = nx - m_x;
    auto dy = ny - m_y;
    if (dx.abs() > dy.abs()) {
      m_pen->draw(m_x + dy.abs() * dx.sign(), ny);
    } else {
      m_pen->draw(nx, m_y + dx.abs() * dy.sign());
    }

    m_x = nx;
    m_y = ny;
    m_pen->draw(m_x, m_y);
  }
  void draw(gerby::d::inch dx, gerby::d::inch dy) {
    m_x = m_x + dx;
    m_y = m_y + dy;
    m_pen->draw(m_x, m_y);
  }
  void draw_x(gerby::d::inch dx) {
    m_x = m_x + dx;
    m_pen->draw_x(m_x);
  }
  void draw_y(gerby::d::inch dy) {
    m_y = m_y + dy;
    m_pen->draw_y(m_y);
  }
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  // https://datasheet.lcsc.com/lcsc/2205311800_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.pdf
  // https://datasheet.lcsc.com/lcsc/2304140030_FH--Guangdong-Fenghua-Advanced-Tech-0805B471K500NT_C1743.pdf
  // https://datasheet.lcsc.com/lcsc/1806151129_Hubei-KENTO-Elec-KT-0805Y_C2296.pdf
  // https://www.ti.com/lit/ds/symlink/lm555.pdf
  static constexpr const r0805 r1{10.0_mm, 0.0_mm, CW270};
  static constexpr const r0805 r2{10.0_mm, 2.0_mm};
  static constexpr const r0805 r3{10.0_mm, 4.0_mm};
  static constexpr const r0805 c1{10.0_mm, 6.0_mm};
  static constexpr const r0805 c2{10.0_mm, 8.0_mm};
  static constexpr const led l1{10.0_mm, 10.0_mm};
  static constexpr const pad bat{10.0_mm, 12.0_mm, 2};
  static constexpr const soic_8 ne555{0.0_mm, 0.0_mm};

  static gerby::thread t{[](auto b) {
    b->add_lines(
        [](auto &p) {
          p.aperture(15.0_mil);

          turtle t{&p};
          t.move(ne555, 2);
          t.draw_x(2.0_mm);
          t.draw(ne555, 6);

          t.move(ne555, 8);
          t.draw_x(1.0_mm);
          t.draw(1.0_mm, -1.0_mm);
          t.draw_y(-3.0_mm);
          t.draw(-1.0_mm, -1.0_mm);
          t.draw_x(-3.0_mm);
          t.draw(ne555, 4);

          t.move(ne555, 8);
          t.draw(r1, 1);

          t.move(r1, 2);
          t.draw(ne555, 7);

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
