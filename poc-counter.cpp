#pragma leco dll
import dotz;
import gerby;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;

static constexpr const auto def_copper_margin = 15.0_mil;
static constexpr const auto def_cu_w = 10.0_mil;
static constexpr const auto def_cu_wm = def_cu_w + def_copper_margin;

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

  point plus(d::inch dx, d::inch dy) const {
    return { x + dx, y + dy };
  }

  point flip() const {
    return { y, x };
  }
};
constexpr point operator+(const point & a, const point & b) { return a.plus(b.x, b.y); }
constexpr point operator*(const point & a, float f) { return { a.x * f, a.y * f }; }

void thermal(cnc::pen & p, point pin, d::inch w, d::inch h) {
  p.aperture(25.0_mil, h + 25.0_mil, false);
  p.flash(pin.x, pin.y);

  p.aperture(w + 25.0_mil, 25.0_mil, false);
  p.flash(pin.x, pin.y);
}
template<typename T>
void thermal(cnc::pen & p, const T & c, int pin) {
  thermal(p, c.pin(pin), T::pad_w, T::pad_h);
}

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

  // TODO: rename to line_dl
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
  void draw_ld(const point & p) {
    auto dx = p.x - m_x;
    auto dy = p.y - m_y;

    if (dx.abs() > dy.abs()) {
      m_pen->draw_x(p.x - dy.abs() * dx.sign());
    } else {
      m_pen->draw_y(p.y - dx.abs() * dy.sign());
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

/// Utility to hold bus lines together
struct bus : point {
  point dist { def_cu_wm, 0 };

  point pin(int n) const {
    return *this + dist * n;
  }

  bus flip() const {
    return { *this, dist.flip() };
  }
};

// https://jlcpcb.com/partdetail/23933-0603WAF5602T5E/C23206
struct r0603 : point {
  static constexpr const auto a = 0.90_mm;
  static constexpr const auto b = 0.65_mm;
  static constexpr const auto c = 0.80_mm;

  static constexpr const auto pad_w = b + def_copper_margin;
  static constexpr const auto pad_h = c + def_copper_margin;

  point pin(int n) const {
    static constexpr const auto px = (a + b) / 2;
    if (n == 1) return point { x + px, y };
    if (n == 2) return point { x - px, y };
    return point {};
  }

  void copper(cnc::pen & p, d::inch margin) const {
    p.aperture(b + margin, c + margin, false);
    p.flash(pin(1).x, pin(1).y);
    p.flash(pin(2).x, pin(2).y);
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
struct sot23 : point { // NPN only
  static constexpr const auto h = 2.02_mm / 2;
  static constexpr const auto w = 1.20_mm / 2;

  static constexpr const auto pad_w = 0.6_mm;
  static constexpr const auto pad_h = 0.8_mm;

  enum { nil, b, e, c };

  point pin(int n) const {
    switch (n) {
      case b: return { x + w, y + h };
      case e: return { x - w, y + h };
      case c: return { x,     y - h };
      default: return {};
    }
  }

  void copper(cnc::pen &p, d::inch m) const {
    p.aperture(pad_w + m, pad_h + m, false);
    p.flash(pin(c).x, pin(c).y);
    p.flash(pin(b).x, pin(b).y);
    p.flash(pin(e).x, pin(e).y);
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

static dotz::vec2 dip_pin_tc_1up(int n, int mx) {
  if (n <= mx / 2) {
    return { -1, (mx / 2) - n };
  } else {
    return { 1, n - (mx / 2) - 1 };
  }
}
static dotz::vec2 dip_pin_tc_1down(int n, int mx) {
  if (n <= mx / 2) {
    return { 1, n - 1 };
  } else {
    return { -1, mx - n };
  }
}

template<unsigned N>
struct dip : point {
  static constexpr const auto pin_r = 0.5_mm;
  static constexpr const auto hole = pin_r + 0.2_mm;

  static constexpr const auto w = 0.3_in / 2;
  static constexpr const auto h = 0.1_in * (N / 4.0 - 0.5);

  static constexpr const auto pad_w = 0.6_mm + hole;
  static constexpr const auto pad_h = 0.6_mm + hole;

  dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;

  point pin(int n) const {
    auto [dx, dy] = pin_fn(n, N);
    return { x + w * dx, y - h + 0.1_in * dy };
  }

  void copper(cnc::pen & p) const {
    for (auto i = 0; i < N; i++) p.flash(pin(i + 1).x, pin(i + 1).y);
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

  auto sign = (r.pin(N).x - r.pin(1).x).sign();

  p.aperture(0.6_mm);
  p.flash(r.pin(1).x + 1.4_mm * sign, r.pin(1).y);
}

template<unsigned N>
struct header : point {
  static constexpr const auto pin_r = 0.5_mm;
  static constexpr const auto hole = pin_r + 0.2_mm;

  static constexpr const auto len = 0.1_in * (N / 2.0 - 0.5);

  static constexpr const auto pad_w = 0.6_mm + hole;
  static constexpr const auto pad_h = 0.6_mm + hole;

  point pin(int n) const {
    return point { x - len + 0.1_in * (n - 1), y };
  }

  void copper(cnc::pen & p) {
    for (auto i = 0; i < N; i++) {
      p.flash(pin(i + 1).x, pin(i + 1).y);
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

struct via : point {
  static constexpr const auto hole = 0.2_mm;
  static constexpr const auto diam = hole + 0.15_mm;
};
void penpen(cnc::pen & p, l::top_copper, via r) {
  p.aperture(via::diam);
  p.flash(r.x, r.y);
}
void penpen(cnc::pen & p, l::top_copper_margin, via r) {
  p.aperture(via::diam + def_copper_margin);
  p.flash(r.x, r.y);
}
void penpen(cnc::pen & p, l::bottom_copper, via r) {
  penpen(p, l::top_copper {}, r);
}
void penpen(cnc::pen & p, l::bottom_copper_margin, via r) {
  penpen(p, l::top_copper_margin {}, r);
}
void penpen(cnc::pen & p, l::holes, via r) {
  p.aperture(via::hole);
  p.flash(r.x, r.y);
}

// MC14553 - 3-digit BCD counter
const auto ic1 = dip<16>{{ 12.0_mm, 7.0_mm}};
// MC14511 - BCD to 7 segments
const auto ic2 = dip<16>{{-6.0_mm, 7.0_mm}, dip_pin_tc_1down};

enum hdr_pins {
  h_nil = 0, // not a real pin
  h_overflow,
  h_reset,
  h_clock,
  h_disable,
  h_strobe,
  h_v_plus,
  h_v_minus,
};
const auto hdr = header<7>({
  -0.4_in + board_w / 2.0,
  -0.1_in + board_h / 2.0,
});

// 100k
const auto r1 = r0603(ic1.pin(12) + point { -3.0_mm, 0 });
const auto r2 = r0603(ic1.pin(11) + point { -3.0_mm, 0 });
const auto r3 = r0603(ic1.pin(10) + point { -3.0_mm, 0 });
const auto r4 = r0603(ic1.pin(13) + point { -3.0_mm, 0 });
// 68
const auto r5  = r0603(ic2.pin(13).plus(-3.0_mm, 0));
const auto r6  = r0603(ic2.pin(12).plus(-3.0_mm, 0));
const auto r7  = r0603(ic2.pin(11).plus(-3.0_mm, 0));
const auto r8  = r0603(ic2.pin(10).plus(-3.0_mm, 0));
const auto r9  = r0603(ic2.pin( 9).plus(-3.0_mm, 0));
const auto r10 = r0603(ic2.pin(15).plus(-3.0_mm, 0));
const auto r11 = r0603(ic2.pin(14).plus(-3.0_mm, 0));

const auto vr8  = via { r8.pin(2).plus(-1.3_mm, 0) };
const auto vr9  = via { r9.pin(2).plus(-1.3_mm, 0) };
const auto vr11 = via { r11.pin(2).plus(-1.3_mm, 0) };

// 1nF
const auto c1 = r0603(ic1.pin(2).plus(3.0_mm, 0));
// 10nF
const auto c2 = r0603(hdr.pin(h_v_minus).plus(-0.5_mm, -2.9_mm));

// BJT NPN
const auto q1 = sot23({ 4.0_mm, 17.0_mm });
const auto q2 = sot23({ 4.0_mm, 13.0_mm });
const auto q3 = sot23({ 4.0_mm,  9.0_mm });

const auto vq1 = via { q1.pin(sot23::c).plus(-1.3_mm, 0) };
const auto vq2 = via { q2.pin(sot23::c).plus(-1.3_mm, 0) };
const auto vq3 = via { q3.pin(sot23::c).plus(-1.3_mm, 0) };

// 7-digit displays
const auto msd = dip<14>{{-12.0_mm, -board_h/2 + 10.0_mm}};
const auto nsd = dip<14>{{  0.0_mm, -board_h/2 + 10.0_mm}};
const auto lsd = dip<14>{{ 12.0_mm, -board_h/2 + 10.0_mm}};

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
      hdr,
      vr8, vr9, vr11,
      vq1, vq2, vq3);
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

  t.move(ic1.pin(15));
  t.draw(q1.pin(sot23::b));

  t.move(ic1.pin(1));
  t.draw(q2.pin(sot23::b));

  t.move(ic1.pin(2));
  t.draw(q3.pin(sot23::b));

  bus hb0 { ic1.pin(15).plus(1.5_mm, 0) };
  bus hb1 = bus { hdr.pin(h_strobe).plus(-1.5_mm, -3.0_mm) }.flip();

  t.move(hdr.pin(h_overflow));
  t.draw(hb1.pin(0));
  t.draw(hb0.pin(0));
  t.draw_ld(ic1.pin(14));

  t.move(hdr.pin(h_reset));
  t.draw(hb1.pin(1));
  t.draw(hb0.pin(1));
  t.draw_ld(ic1.pin(13)); t.draw(r4.pin(1)); 

  t.move(hdr.pin(h_clock));
  t.draw(hb1.pin(2));
  t.draw(hb0.pin(2));
  t.draw_ld(ic1.pin(12)); t.draw(r1.pin(1)); 

  t.move(hdr.pin(h_disable));
  t.draw(hb0.pin(3));
  t.draw_ld(ic1.pin(11)); t.draw(r2.pin(1)); 

  t.move(hdr.pin(h_strobe));
  t.draw(hb0.pin(4));
  t.draw_ld(ic1.pin(10)); t.draw(r3.pin(1)); 

  t.move(hdr.pin(h_v_plus));
  t.draw(c2.pin(2));

  t.move(ic1.pin(3));
  t.draw(c1.pin(2));

  t.move(ic1.pin(4));
  t.draw(c1.pin(1));

  t.move(ic1.pin(9));
  t.draw(ic2.pin(1).plus(0, -2.0_mm));
  t.draw(ic2.pin(6).plus(-1.8_mm, 0));
  t.draw_ld(ic2.pin(7));

  t.move(ic1.pin(7));
  t.draw(ic2.pin(1));

  t.move(ic1.pin(6));
  t.draw(ic2.pin(2));

  t.move(ic1.pin(5));
  t.draw_ld(ic2.pin(6));

  t.move(ic2.pin(13));
  t.draw(r5.pin(1));
  t.move(r5.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(2).plus(-1.5_mm, -0.3_mm));
  t.draw(msd.pin(2).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(14));

  t.move(ic2.pin(12));
  t.draw(r6.pin(1));
  t.move(r6.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(3).plus(-2.5_mm, -0.3_mm));
  t.draw(msd.pin(3).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(13));

  t.move(ic2.pin(11));
  t.draw(r7.pin(1));
  t.move(r7.pin(2));
  t.draw_x(-2.0_mm);
  t.draw(msd.pin(7).plus(-3.5_mm, -0.3_mm));
  t.draw(msd.pin(7).plus(0, -1.3_mm));
  t.draw_ld(msd.pin(8));

  t.move(ic2.pin(10));
  t.draw(r8.pin(1));
  t.move(r8.pin(2));
  t.draw(vr8);

  t.move(ic2.pin(9));
  t.draw(r9.pin(1));
  t.move(r9.pin(2));
  t.draw(vr9);

  t.move(ic2.pin(15));
  t.draw(r10.pin(1));
  t.move(r10.pin(2));
  t.draw_ld(msd.pin(1));

  t.move(ic2.pin(14));
  t.draw(r11.pin(1));
  t.move(r11.pin(2));
  t.draw(vr11);

  t.move(q1.pin(sot23::c)); t.draw(vq1);
  t.move(q2.pin(sot23::c)); t.draw(vq2);
  t.move(q3.pin(sot23::c)); t.draw(vq3);
}
void bottom_nets(cnc::pen & p) {
  turtle t { &p };
  t.move(vr11);
  t.draw(vr11.plus(0.5_mm, -0.5_mm));
  t.draw_ld(msd.pin(2));

  t.move(vr8);
  t.draw(vr8.plus(1.5_mm, -1.5_mm));
  t.draw_ld(msd.pin(7));

  t.move(vr9);
  t.draw(vr9.plus(-4.0_mm, -4.0_mm));
  t.draw_ld(msd.pin(6));

  t.move(vq1);
  t.draw(ic2.pin(1).plus(2.0_mm, 0));
  t.draw(ic2.pin(1).plus(-2.0_mm, -2.0_mm));
  t.draw(msd.pin(13).plus(1.5_mm, 0));
  t.draw_ld(msd.pin(12));

  t.move(vq2);
  t.draw(ic2.pin(1).plus(3.0_mm, 0));
  t.draw_ld(nsd.pin(4));

  t.move(vq3);
  t.draw(ic1.pin(8).plus(-2.0_mm, 0));
  t.draw_ld(lsd.pin(4));
}

void top_copper(cnc::pen & p) {
  penny(p, l::top_copper {});

  p.aperture(10.0_mil);
  top_nets(p);

  thermal(p, q1, sot23::e);
  thermal(p, q2, sot23::e);
  thermal(p, q3, sot23::e);
  thermal(p, ic1, 8);
  thermal(p, ic2, 8);
  thermal(p, ic2, 5);
  thermal(p, r1, 2);
  thermal(p, r2, 2);
  thermal(p, r3, 2);
  thermal(p, r4, 2);
  thermal(p, c2, 1);
  thermal(p, hdr, 7);
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

  thermal(p, ic1, 16);
  thermal(p, ic2, 16);
  thermal(p, ic2, 4);
  thermal(p, ic2, 3);
  thermal(p, hdr, 6);
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
      b->add_region(plane, blue);
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
