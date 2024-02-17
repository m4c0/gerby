#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

enum rotation { CW0, CW90, CW180, CW270 };

class compo {
  gerby::d::inch m_x;
  gerby::d::inch m_y;
  rotation m_rot{};

protected:
  [[nodiscard]] constexpr auto x() const noexcept { return m_x; }
  [[nodiscard]] constexpr auto y() const noexcept { return m_y; }
  [[nodiscard]] constexpr auto rot() const noexcept { return m_rot; }

  void flash_pins(auto &p, unsigned max) const {
    for (auto i = 1; i <= max; i++) {
      p.flash(pin_x(i), pin_y(i));
    }
  }

  virtual gerby::d::inch pin_rel_x(unsigned) const = 0;
  virtual gerby::d::inch pin_rel_y(unsigned) const = 0;

public:
  constexpr compo(gerby::d::inch x, gerby::d::inch y, rotation r = CW0)
      : m_x{x}
      , m_y{y}
      , m_rot{r} {}

  virtual void copper(gerby::pen &, gerby::d::inch margin) const = 0;
  virtual void doc(gerby::pen &) const {};
  virtual void hole(gerby::pen &) const {};

  gerby::d::inch pin_x(unsigned i) const {
    switch (m_rot) {
    case CW0:
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
    case CW0:
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
  gerby::d::inch pin_rel_y(unsigned i) const override {
    return -0.1_in * (i - 1);
  }

public:
  constexpr pad(gerby::d::inch x, gerby::d::inch y, unsigned n,
                rotation r = CW0)
      : compo{x, y, r}
      , m_count{n} {}

  void copper(gerby::pen &p, gerby::d::inch m) const override {
    p.aperture(copper_d + m);
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
  gerby::d::inch pin_rel_x(unsigned i) const override {
    return (d - b) * (i - 1);
  }
  gerby::d::inch pin_rel_y(unsigned i) const override { return 0; }

public:
  using compo::compo;

  void copper(gerby::pen &p, gerby::d::inch m) const override {
    if (rot() == CW90 || rot() == CW270) {
      p.aperture(c + m, b + m, false);
    } else {
      p.aperture(b + m, c + m, false);
    }
    flash_pins(p, 2);
  }
};

class led : public r0805 {
public:
  using r0805::r0805;

  void doc(gerby::pen &p) const override {
    p.aperture(10.0_mil);

    auto cx = (pin_x(2) + pin_x(1)) * 0.5;
    auto cy = (pin_y(2) + pin_y(1)) * 0.5;
    auto dh = 6.0_mil * (pin_x(2) - pin_x(1)).sign();
    auto dw = 8.0_mil * (pin_y(2) - pin_y(1)).sign();

    if (rot() == CW0 || rot() == CW180) {
      p.move(cx - dh, cy - dw);
      p.draw_y(cy + dw);
      p.draw(cx + dh, cy);
      p.draw(cx - dh, cy - dw);
    } else {
      p.move(cx - dw, cy + dh);
      p.draw_x(cx + dw);
      p.draw(cx, cy - dh);
      p.draw(cx - dw, cy + dh);
    }
  }
};

class soic_8 : public compo {
  static constexpr const auto pw = 1.55_mm;
  static constexpr const auto ph = 0.60_mm;
  static constexpr const auto dx = 5.40_mm;
  static constexpr const auto dy = 1.27_mm;

  gerby::d::inch pin_rel_x(unsigned i) const override {
    return dx * ((i - 1) / 4);
  }
  gerby::d::inch pin_rel_y(unsigned i) const override {
    if (i < 5) {
      return -dy * (i - 1);
    } else {
      return -dy * (8 - i);
    }
  }

public:
  using compo::compo;

  void copper(gerby::pen &p, gerby::d::inch m) const override {
    p.aperture(pw + m, ph + m, false);
    flash_pins(p, 8);
  }
  void doc(gerby::pen &p) const override {
    p.aperture(10.0_mil, 30.0_mil, true);
    p.flash(x() + pw, y());
  }
};

class turtle {
  gerby::cnc::pen *m_pen;

  gerby::d::inch m_x{};
  gerby::d::inch m_y{};

public:
  explicit constexpr turtle(gerby::cnc::pen *p) : m_pen{p} {}

  void move(const compo &c, unsigned pin) {
    m_x = c.pin_x(pin);
    m_y = c.pin_y(pin);
    m_pen->move(m_x, m_y);
  }

  void draw(const compo &c, unsigned pin) {
    auto nx = c.pin_x(pin);
    auto ny = c.pin_y(pin);

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
class minmax_pen : public gerby::cnc::pen {
  gerby::d::inch m_min_x{1e10f};
  gerby::d::inch m_min_y{1e10f};
  gerby::d::inch m_max_x{-1e10f};
  gerby::d::inch m_max_y{-1e10f};

  void update_x(gerby::d::inch x) {
    m_min_x = m_min_x < x ? m_min_x : x;
    m_max_x = m_max_x > x ? m_max_x : x;
  }
  void update_y(gerby::d::inch y) {
    m_min_y = m_min_y < y ? m_min_y : y;
    m_max_y = m_max_y > y ? m_max_y : y;
  }

public:
  void draw(gerby::d::inch x, gerby::d::inch y) override {
    update_x(x);
    update_y(y);
  }
  void draw_x(gerby::d::inch x) override { update_x(x); }
  void draw_y(gerby::d::inch y) override { update_y(y); }
  void move(gerby::d::inch x, gerby::d::inch y) override {
    update_x(x);
    update_y(y);
  }
  void move_x(gerby::d::inch x) override { update_x(x); }
  void move_y(gerby::d::inch y) override { update_y(y); }

  [[nodiscard]] gerby::d::inch center_x() const noexcept {
    return (m_max_x + m_min_x) * 0.5f;
  }
  [[nodiscard]] gerby::d::inch center_y() const noexcept {
    return (m_max_y + m_min_y) * 0.5f;
  }
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  // https://datasheet.lcsc.com/lcsc/2205311800_UNI-ROYAL-Uniroyal-Elec-0805W8F1001T5E_C17513.pdf
  // https://datasheet.lcsc.com/lcsc/2304140030_FH--Guangdong-Fenghua-Advanced-Tech-0805B471K500NT_C1743.pdf
  // https://datasheet.lcsc.com/lcsc/1806151129_Hubei-KENTO-Elec-KT-0805Y_C2296.pdf
  // https://www.ti.com/lit/ds/symlink/lm555.pdf
  static constexpr const r0805 r1{8.0_mm, 0.0_mm, CW270};
  static constexpr const r0805 r2{4.0_mm, 2.0_mm, CW180};
  static constexpr const r0805 r3{-2.5_mm, -3.0_mm, CW270};
  static constexpr const r0805 c1{-2.5_mm, -1.2_mm, CW90};
  static constexpr const r0805 c2{8.0_mm, -4.0_mm, CW270};
  static constexpr const led l1{-5.0_mm, -5.3_mm, CW90};
  static constexpr const pad bat{10.0_mm, 3.0_mm, 2, CW90};
  static constexpr const soic_8 ne555{0.0_mm, 0.0_mm};

  static constexpr const auto nets = [](turtle &t) {
    t.move(ne555, 2);
    t.draw_x(2.0_mm);
    t.draw(ne555, 6);

    t.move(r1, 1);
    t.draw_x(1.0_mm);
    t.draw(1.0_mm, -1.0_mm);
    t.draw_y(-3.0_mm);
    t.draw(-1.0_mm, -1.0_mm);
    t.draw_x(-6.0_mm);
    t.draw(ne555, 4);

    t.move(ne555, 8);
    t.draw(r1, 1);

    t.move(r1, 2);
    t.draw(ne555, 7);

    t.move(ne555, 7);
    t.draw_x(-0.6_mm);
    t.draw(r2, 1);

    t.move(r2, 2);
    t.draw_y(-3.0_mm);
    t.draw(ne555, 2);
    t.draw(c1, 1);

    t.move(ne555, 5);
    t.draw(c2, 1);

    t.move(ne555, 3);
    t.draw(r3, 1);

    t.move(r3, 2);
    t.draw(l1, 1);
  };
  static constexpr const auto pads = [](auto &p, gerby::d::inch m) {
    r1.copper(p, m);
    r2.copper(p, m);
    r3.copper(p, m);
    c1.copper(p, m);
    c2.copper(p, m);
    l1.copper(p, m);
    ne555.copper(p, m);
    bat.copper(p, m);
  };
  static constexpr const auto copper = [](auto &p, gerby::d::inch m) {
    p.aperture(15.0_mil + m);

    turtle t{&p};
    nets(t);

    pads(p, m);
  };

  static constexpr const auto plane = [](auto &f) {
    minmax_pen mmp{};
    turtle trt{&mmp};
    nets(trt);

    auto cx = mmp.center_x();
    auto cy = mmp.center_y();

    constexpr const auto w = 20.0_mm;
    constexpr const auto h = 20.0_mm;

    const auto l = cx - w * 0.5;
    const auto r = l + w;
    const auto b = cy - h * 0.5;
    const auto t = b + h;
    f.move(l, b);
    f.draw_x(r);
    f.draw_y(t);
    f.draw_x(l);
    f.draw_y(b);
  };

  static gerby::thread t{[](auto b) {
    b->add_region([](auto &f) { plane(f); }, red);
    b->add_lines([](auto &p) { copper(p, 15.0_mil); }, black);
    b->add_lines([](auto &p) { copper(p, 0.0); }, red);
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
