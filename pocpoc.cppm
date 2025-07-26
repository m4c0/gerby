export module pocpoc;
import dotz;
import gerby;
import traits;

using namespace gerby::literals;
using namespace gerby::palette;
using namespace gerby;

namespace pocpoc {
  export constexpr const auto def_copper_margin = 15.0_mil;
  export constexpr const auto def_cu_w = 10.0_mil;
  export constexpr const auto def_cu_wm = def_cu_w + def_copper_margin;

  export constexpr const auto def_mask_margin = 10.0_mil;
  
  export void box(cnc::pen & p, d::inch cx, d::inch cy, d::inch w, d::inch h, d::inch a) {
    p.aperture(a);
    cnc::utils::box(p, cx, cy, w, h);
  }
  export void box(cnc::pen & p, d::inch cx, d::inch cy, d::inch w, d::inch h) {
    p.aperture(6.0_mil);
    cnc::utils::box(p, cx, cy, w, h);
  }
  
  export namespace l {
    struct top_copper {};
    struct top_copper_margin {};
    struct top_mask {};
    struct bottom_copper {};
    struct bottom_copper_margin {};
    struct bottom_mask {};
    struct holes {};
    struct silk {};
  }
  template<typename L, typename T> void penpen(cnc::pen & p, L, T t) {
  }
  
  template<typename T>
  concept coppered = requires (T t, cnc::pen p, d::inch m) { t.copper(p, m); };
  void penpen(cnc::pen & p, l::top_copper, coppered auto t) {
    t.copper(p, 0);
  }
  void penpen(cnc::pen & p, l::top_mask, coppered auto t) {
    t.copper(p, def_mask_margin);
  }
  void penpen(cnc::pen & p, l::top_copper_margin, coppered auto t) {
    t.copper(p, def_copper_margin);
  }
  
  template<typename T>
  concept drillable = requires (T t, cnc::pen p) { t.drill(p); };
  void penpen(cnc::pen & p, l::holes, drillable auto t) {
    t.drill(p);
  }
  
  struct tht {};
  template<typename T>
  concept is_tht = traits::is_assignable_from<tht &, T>;
  void penpen(cnc::pen & p, l::bottom_copper, is_tht auto t) {
    penpen(p, l::top_copper{}, t);
  }
  void penpen(cnc::pen & p, l::bottom_copper_margin, is_tht auto t) {
    penpen(p, l::top_copper_margin{}, t);
  }
  void penpen(cnc::pen & p, l::bottom_mask, is_tht auto t) {
    penpen(p, l::top_mask{}, t);
  }
  
  // Aligned at the center of the compo
  export struct point : cnc::point {
    point plus(d::inch dx, d::inch dy) const {
      return { x + dx, y + dy };
    }
    point plus(cnc::point p) const {
      return { x + p.x, y + p.y };
    }
  
    point flip() const {
      return { y, x };
    }
  };
  constexpr point operator+(const point & a, const cnc::point & b) { return a.plus(b.x, b.y); }
  constexpr point operator-(const point & a, const cnc::point & b) { return a.plus(-b.x, -b.y); }
  constexpr point operator+(const point & a, d::inch i) { return { a.x + i, a.y + i }; }
  constexpr point operator*(const point & a, float f) { return { a.x * f, a.y * f }; }
  
  export void thermal(cnc::pen & p, point pin, point size) {
    p.aperture(25.0_mil, size.y + 25.0_mil, false);
    p.flash(pin);
  
    p.aperture(size.x + 25.0_mil, 25.0_mil, false);
    p.flash(pin);
  }
  export template<typename T>
  void thermal(cnc::pen & p, const T & c, int pin) {
    thermal(p, c.pin(pin), T::pad);
  }
  
  export class turtle {
    cnc::pen *m_pen;
  
    cnc::point m_pos {};
  
  public:
    explicit constexpr turtle(cnc::pen *p) : m_pen{p} {}
  
    void move(const point & p) {
      m_pos = p;
      m_pen->move(m_pos);
    }
  
    // TODO: rename to line_dl
    void draw(const point & p) {
      auto [dx, dy] = p - m_pos;
      if (dx.abs() > dy.abs()) {
        m_pen->draw(m_pos.x + dy.abs() * dx.sign(), p.y);
      } else {
        m_pen->draw(p.x, m_pos.y + dx.abs() * dy.sign());
      }
  
      m_pos = p;
      m_pen->draw(m_pos);
    }
    void draw_ld(const point & p) {
      auto [dx, dy] = p - m_pos;
      if (dx.abs() > dy.abs()) {
        m_pen->draw_x(p.x - dy.abs() * dx.sign());
      } else {
        m_pen->draw_y(p.y - dx.abs() * dy.sign());
      }
  
      m_pos = p;
      m_pen->draw(m_pos);
    }
    void draw_x(d::inch dx) {
      m_pos.x = m_pos.x + dx;
      m_pen->draw_x(m_pos.x);
    }
    void draw_y(d::inch dy) {
      m_pos.y = m_pos.y + dy;
      m_pen->draw_y(m_pos.y);
    }
  };
  
  /// Utility to hold bus lines together
  export struct bus : point {
    point dist { def_cu_wm, 0 };
  
    point pin(int n) const {
      return *this + dist * n;
    }
  
    bus flip() const {
      return { *this, dist.flip() };
    }
  };
  
  // https://jlcpcb.com/partdetail/23933-0603WAF5602T5E/C23206
  export struct r0603 : point {
    static constexpr const auto a = 0.90_mm;
    static constexpr const auto b = 0.65_mm;
    static constexpr const auto c = 0.80_mm;
  
    static constexpr const point pad { b + def_copper_margin, c + def_copper_margin };
  
    point pin(int n) const {
      static constexpr const auto px = (a + b) / 2;
      if (n == 1) return point { x + px, y };
      if (n == 2) return point { x - px, y };
      return point {};
    }
  
    void copper(cnc::pen & p, d::inch margin) const {
      p.aperture(b + margin, c + margin, false);
      p.flash(pin(1));
      p.flash(pin(2));
    };
  };
  template<> void penpen(cnc::pen & p, l::silk, r0603 r) {
    static constexpr const auto l = 1.6_mm;
    static constexpr const auto w = 0.8_mm;
    box(p, r.x, r.y, l, w);
  };
  
  // https://jlcpcb.com/partdetail/Hubei_KENTOElec-KT0603R/C2286
  export struct d0603 : r0603 {
  };
  template<> void penpen(cnc::pen & p, l::silk, d0603 r) {
    static constexpr const auto l = 1.6_mm;
    static constexpr const auto w = 0.8_mm;
    box(p, r.x, r.y, l, w);
    p.move(r.x - 0.2_mm, r.y);
    p.draw_x(r.x + 0.2_mm);
    p.move_y(r.y - 0.2_mm);
    p.move_y(r.y + 0.2_mm);
  };
  
  // https://jlcpcb.com/partdetail/2503-S8050_J3Y_RANGE_200_350/C2146
  export struct sot23 : point { // NPN only
    static constexpr const auto h = 2.02_mm / 2;
    static constexpr const auto w = 1.90_mm / 2;
  
    static constexpr const point pad { 0.6_mm, 0.8_mm };
  
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
      p.aperture(pad + m, false);
      p.flash(pin(c));
      p.flash(pin(b));
      p.flash(pin(e));
    }
  };
  template<> void penpen(cnc::pen & p, l::silk, sot23 r) {
    static constexpr const auto l = 2.9_mm;
    static constexpr const auto w = 1.3_mm;
    box(p, r.x, r.y, l, w);
  }
  
  export dotz::vec2 dip_pin_tc_1up(int n, int mx) {
    if (n <= mx / 2) {
      return { -1, (mx / 2) - n };
    } else {
      return { 1, n - (mx / 2) - 1 };
    }
  }
  export dotz::vec2 dip_pin_tc_1down(int n, int mx) {
    if (n <= mx / 2) {
      return { 1, n - 1 };
    } else {
      return { -1, mx - n };
    }
  }
  
  export template<unsigned N>
  struct dip : point, tht {
    static constexpr const auto pin_r = 0.5_mm;
    static constexpr const auto hole = pin_r + 0.2_mm;
  
    static constexpr const auto w = 0.3_in / 2;
    static constexpr const auto h = 0.1_in * (N / 4.0 - 0.5);
  
    static constexpr const point pad { 0.6_mm + hole, 0.6_mm + hole };
  
    dotz::vec2 (*pin_fn)(int, int) = dip_pin_tc_1up;
  
    point pin(int n) const {
      auto [dx, dy] = pin_fn(n, N);
      return { x + w * dx, y - h + 0.1_in * dy };
    }
  
    void copper(cnc::pen & p, d::inch m) const {
      p.aperture(hole + m + 0.6_mm);
      for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
    }
  
    void drill(cnc::pen & p) const {
      p.aperture(hole);
      for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
    }
  };
  template<unsigned N> void penpen(cnc::pen & p, l::silk, dip<N> r) {
    box(p, r.x, r.y, 0.3_in, 0.1_in * N / 2);
  
    auto sign = (r.pin(N).x - r.pin(1).x).sign();
  
    p.aperture(0.5_mm);
    p.flash(r.pin(1).plus(1.4_mm * sign, 0));
  }
  
  // Untested: hole increased to +0.4mm from +0.2mm
  export template<unsigned N>
  struct header : point, tht {
    static constexpr const auto pin_r = 0.5_mm;
    static constexpr const auto hole = pin_r + 0.4_mm;
  
    static constexpr const auto len = 0.1_in * (N / 2.0 - 0.5);
  
    static constexpr const point pad { 0.6_mm + hole, 0.6_mm + hole };
  
    point pin(int n) const {
      return point { x - len + 0.1_in * (n - 1), y };
    }
  
    void copper(cnc::pen & p, d::inch m) const {
      p.aperture(hole + m + 0.6_mm);
      for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
    }
  
    void drill(cnc::pen & p) const {
      p.aperture(hole);
      for (auto i = 0; i < N; i++) p.flash(pin(i + 1));
    }
  };
  template<unsigned N> void penpen(cnc::pen & p, l::silk, header<N> r) {
    box(p, r.x, r.y, 0.1_in * N, 0.1_in);
  }
  
  export struct via : point, tht {
    static constexpr const auto hole = 0.3_mm;
    static constexpr const auto diam = hole + 0.15_mm;
  
    point pin(int n) const { return *this; }
  
    void copper(cnc::pen & p, d::inch m) const {
      p.aperture(via::diam + m);
      p.flash(pin(0));
    }
  
    void drill(cnc::pen & p) const {
      p.aperture(hole);
      p.flash(pin(0));
    }
  };
  void penpen(cnc::pen & p, l::top_mask, via r) {}

  export template<typename T>
  void pennies(cnc::pen & p, T t, auto... cs) {
    (penpen(p, t, cs), ...);
  }
}
