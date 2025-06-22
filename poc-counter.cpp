#pragma leco dll
import gerby;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;

static constexpr const auto def_copper_margin = 15.0_mil;

static constexpr const auto board_w = 50.0_mm;
static constexpr const auto board_h = 50.0_mm;

void box(cnc::pen & p, d::inch cx, d::inch cy, d::inch w, d::inch h) {
  p.aperture(6.0_mil);
  p.move(cx - w / 2, cy - h / 2);
  p.draw_x(cx + w / 2);
  p.draw_y(cy + h / 2);
  p.draw_x(cx - w / 2);
  p.draw_y(cy - h / 2);
}

namespace l {
  struct top_copper {};
  struct top_copper_margin {};
  struct bottom_copper {};
  struct bottom_copper_margin {};
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

class turtle {
  cnc::pen *m_pen;

  d::inch m_x{};
  d::inch m_y{};

public:
  explicit constexpr turtle(cnc::pen *p) : m_pen{p} {}

  void move(const point & p) {
    m_x = p.x;
    m_y = p.y;
    m_pen->move(m_x, m_y);
  }

  void draw(const point & p) {
    auto dx = p.x - m_x;
    auto dy = p.y - m_y;
    if (dx.abs() > dy.abs()) {
      m_pen->draw(m_x + dy.abs() * dx.sign(), p.y);
    } else {
      m_pen->draw(p.x, m_y + dx.abs() * dy.sign());
    }

    m_x = p.x;
    m_y = p.y;
    m_pen->draw(m_x, m_y);
  }
  void draw(d::inch dx, d::inch dy) {
    m_x = m_x + dx;
    m_y = m_y + dy;
    m_pen->draw(m_x, m_y);
  }
  void draw_x(d::inch dx) {
    m_x = m_x + dx;
    m_pen->draw_x(m_x);
  }
  void draw_y(d::inch dy) {
    m_y = m_y + dy;
    m_pen->draw_y(m_y);
  }
};

// https://jlcpcb.com/partdetail/23933-0603WAF5602T5E/C23206
struct r0603 : point {
  void copper(cnc::pen & p, d::inch margin) {
    static constexpr const auto a = 0.90_mm;
    static constexpr const auto b = 0.65_mm;
    static constexpr const auto c = 0.80_mm;

    static constexpr const auto px = (a + b) / 2;

    p.aperture(b + margin, c + margin, false);
    p.flash(x + px, y);
    p.flash(x - px, y);
  };
};
template<> void penpen(cnc::pen & p, l::top_copper, r0603 r) {
  r.copper(p, 0);
};
template<> void penpen(cnc::pen & p, l::top_copper_margin, r0603 r) {
  r.copper(p, def_copper_margin);
}
template<> void penpen(cnc::pen & p, l::silk, r0603 r) {
  static constexpr const auto l = 1.6_mm;
  static constexpr const auto w = 0.8_mm;
  box(p, r.x, r.y, l, w);
};

// https://jlcpcb.com/partdetail/2503-S8050_J3Y_RANGE_200_350/C2146
struct sot23 : point {
  void copper(cnc::pen &p, d::inch m) {
    static constexpr const auto h = 2.02_mm / 2;
    static constexpr const auto w = 1.20_mm / 2;

    p.aperture(0.6_mm + m, 0.8_mm + m, false);
    p.flash(x,     y + h);
    p.flash(x - w, y - h);
    p.flash(x + w, y - h);
  }
};
template<> void penpen(cnc::pen & p, l::top_copper, sot23 r) {
  r.copper(p, 0);
}
template<> void penpen(cnc::pen & p, l::top_copper_margin, sot23 r) {
  r.copper(p, def_copper_margin);
}
template<> void penpen(cnc::pen & p, l::silk, sot23 r) {
  static constexpr const auto l = 2.9_mm;
  static constexpr const auto w = 1.3_mm;
  box(p, r.x, r.y, l, w);
}

template<unsigned N>
struct dip : point {
  static constexpr const auto pin_r = 0.5_mm;
  static constexpr const auto hole = pin_r + 0.2_mm;

  static constexpr const auto w = 0.3_in / 2;
  static constexpr const auto h = 0.1_in * (N / 4.0 - 0.5);

  point pin(int n) const {
    if (n <= N / 2) {
      auto i = (N / 2) - n;
      return { x - w, y - h + 0.1_in * i };
    } else {
      auto i = n - (N / 2) - 1;
      return { x + w, y - h + 0.1_in * i };
    }
  }

  void copper(cnc::pen & p) const {
    for (auto i = 0; i < N / 2; i++) {
      p.flash(pin(i + 1).x, pin(i + 1).y);
      p.flash(pin(N - i).x, pin(N - i).y);
    }
  }
};
template<unsigned N> void penpen(cnc::pen & p, l::top_copper, dip<N> r) {
  p.aperture(dip<N>::hole + 0.6_mm);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::bottom_copper, dip<N> r) {
  penpen(p, l::top_copper {}, r);
}
template<unsigned N> void penpen(cnc::pen & p, l::top_copper_margin, dip<N> r) {
  p.aperture(dip<N>::hole + 0.6_mm + def_copper_margin);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::bottom_copper_margin, dip<N> r) {
  penpen(p, l::top_copper_margin {}, r);
}
template<unsigned N> void penpen(cnc::pen & p, l::holes, dip<N> r) {
  p.aperture(dip<N>::hole);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::silk, dip<N> r) {
  box(p, r.x, r.y, 0.3_in, 0.1_in * N / 2);

  p.aperture(0.6_mm);
  p.flash(r.pin(1).x + 1.4_mm, r.pin(1).y);
}

template<unsigned N>
struct header : point {
  static constexpr const auto pin = 0.5_mm;
  static constexpr const auto hole = pin + 0.2_mm;

  static constexpr const auto len = 0.1_in * (N / 2.0 - 0.5);

  void copper(cnc::pen & p) {
    for (auto i = 0; i < N; i++) {
      p.flash(x - len + 0.1_in * i, y);
    }
  }
};
template<unsigned N> void penpen(cnc::pen & p, l::top_copper, header<N> r) {
  p.aperture(header<N>::hole + 0.6_mm);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::bottom_copper, header<N> r) {
  penpen(p, l::top_copper {}, r);
}
template<unsigned N> void penpen(cnc::pen & p, l::top_copper_margin, header<N> r) {
  p.aperture(header<N>::hole + 0.6_mm + def_copper_margin);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::bottom_copper_margin, header<N> r) {
  penpen(p, l::top_copper_margin {}, r);
}
template<unsigned N> void penpen(cnc::pen & p, l::holes, header<N> r) {
  p.aperture(header<N>::hole);
  r.copper(p);
}
template<unsigned N> void penpen(cnc::pen & p, l::silk, header<N> r) {
  box(p, r.x, r.y, 0.1_in * N, 0.1_in);
}

// 100k
const auto r1 = r0603({ -6.0_mm, 3.0_mm });
const auto r2 = r0603({ -6.0_mm, 5.0_mm });
const auto r3 = r0603({ -6.0_mm, 7.0_mm });
const auto r4 = r0603({ -6.0_mm, 9.0_mm });
// 68
const auto r5  = r0603({ 6.0_mm, 3.0_mm });
const auto r6  = r0603({ 6.0_mm, 5.0_mm });
const auto r7  = r0603({ 6.0_mm, 7.0_mm });
const auto r8  = r0603({ 6.0_mm, 9.0_mm });
const auto r9  = r0603({ 6.0_mm, 11.0_mm });
const auto r10 = r0603({ 6.0_mm, 13.0_mm });
const auto r11 = r0603({ 6.0_mm, 15.0_mm });

// 1nF
const auto c1 = r0603({ -12.0_mm, 0.0_mm });
// 10nF
const auto c2 = r0603({  12.0_mm, 0.0_mm });

// BJT NPN
const auto q1 = sot23({-12.0_mm, -board_h / 2 + 8.0_mm});
const auto q2 = sot23({  0.0_mm, -board_h / 2 + 8.0_mm});
const auto q3 = sot23({ 12.0_mm, -board_h / 2 + 8.0_mm});

// 7-digit displays
const auto msd = dip<14>({-12.0_mm, -board_h/2 + 10.0_mm});
const auto nsd = dip<14>({  0.0_mm, -board_h/2 + 10.0_mm});
const auto lsd = dip<14>({ 12.0_mm, -board_h/2 + 10.0_mm});

// MC14553 - 3-digit BCD counter
const auto ic1 = dip<16>({-6.0_mm, 7.0_mm});
// MC14511 - BCD to 7 segments
const auto ic2 = dip<16>({ 6.0_mm, 7.0_mm});

// Strobe, disable, clock, reset, overflow, V+, V-
const auto hdr = header<7>({
  0.4_in - board_w / 2.0,
  -0.1_in + board_h / 2.0,
});

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
      ic1, ic2,
      hdr);
}

void link_digits(turtle & t, unsigned pin, d::inch d0x, d::inch d0y, d::inch d1x) {
  t.move(msd.pin(pin));
  t.draw(d0x, d0y);
  t.draw_x(d1x);
  t.draw(nsd.pin(pin));
  t.draw(d0x, d0y);
  t.draw_x(d1x);
  t.draw(lsd.pin(pin));
}
void link_digit_gnd(turtle & t, const auto & d) {
  t.move(d.pin(12));
  t.draw(d.pin(4));
}

void top_nets(cnc::pen & p) {
  turtle t { &p };

  link_digits(t,  1, 2.2_mm,  2.2_mm, 7.0_mm);
  link_digits(t,  2, 4.0_mm,  4.0_mm, 4.0_mm);
  link_digits(t, 14, 3.8_mm, -3.8_mm, 4.5_mm);
  link_digits(t, 13, 3.8_mm, -3.8_mm, 4.5_mm);

  link_digits(t, 8,  1.25_mm, -1.25_mm, 8.0_mm);
  link_digits(t, 7,  1.25_mm,  1.25_mm, 8.0_mm);
  link_digits(t, 6,  1.25_mm,  1.25_mm, 8.0_mm);

  link_digit_gnd(t, msd);
  link_digit_gnd(t, nsd);
  link_digit_gnd(t, lsd);
}
void bottom_nets(cnc::pen & p) {
  turtle t { &p };
}

void top_copper(cnc::pen & p) {
  penny(p, l::top_copper {});

  p.aperture(10.0_mil);
  top_nets(p);
}
void top_copper_margin(cnc::pen & p) {
  penny(p, l::top_copper_margin {});

  p.aperture(10.0_mil + def_copper_margin);
  top_nets(p);
}
void top_silk(cnc::pen & p) {
  penny(p, l::silk {});
}

void bottom_copper(cnc::pen & p) {
  penny(p, l::bottom_copper {});

  p.aperture(10.0_mil);
  bottom_nets(p);
}
void bottom_copper_margin(cnc::pen & p) {
  penny(p, l::bottom_copper_margin {});

  p.aperture(10.0_mil + def_copper_margin);
  bottom_nets(p);
}

void holes(cnc::pen & p) {
  penny(p, l::holes {});
}
void border_margin(cnc::pen & p) {
  box(p, 0, 0, board_w + 25.0_mil, board_h + 25.0_mil);
}

void border(cnc::pen & p) {
  box(p, 0, 0, board_w, board_h);
}
void plane(cnc::fanner & p) {
  static constexpr const auto m = 1.0_mm;
  static constexpr const auto w = (board_w - m) / 2;
  static constexpr const auto h = (board_h - m) / 2;
  p.move(-w, -h);
  p.draw_x(w);
  p.draw_y(h);
  p.draw_x(-w);
  p.draw_y(-h);
}

extern "C" void draw(cnc::builder * b, cnc::grb_layer l) {
  switch (l) {
    case cnc::gl_top_copper:
      b->add_region(plane, red);
      b->add_lines(top_copper_margin, black);
      b->add_lines(top_copper, red);
      b->add_lines(holes, black);
      break;
    case cnc::gl_top_silk:
      b->add_lines(top_silk, white);
      break;
    case cnc::gl_bot_copper:
      b->add_lines(bottom_copper_margin, black);
      b->add_lines(bottom_copper, blue);
      b->add_lines(holes, black);
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
