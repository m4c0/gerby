#pragma leco app

import casein;
import dotz;
import gerby;

constexpr const dotz::vec4 red{1, 0, 0, 0};
constexpr const dotz::vec4 black{0, 0, 0, 0};
constexpr const dotz::vec4 white{1, 1, 1, 0};

template <bool Inches> class distance {
  long double m_val;

public:
  explicit constexpr distance(long double d) noexcept : m_val{d} {}

  [[nodiscard]] constexpr auto value() const noexcept { return m_val; }

  [[nodiscard]] constexpr auto operator-() const noexcept {
    return distance<Inches>{-m_val};
  }

  [[nodiscard]] explicit constexpr operator distance<!Inches>() const noexcept;
};

template <>
[[nodiscard]] constexpr distance<true>::operator distance<false>()
    const noexcept {
  return distance<false>{m_val * 25.4};
}
template <>
[[nodiscard]] constexpr distance<false>::operator distance<true>()
    const noexcept {
  return distance<true>{m_val / 25.4};
}

constexpr auto operator""_in(long double d) { return distance<true>{d}; }
constexpr auto operator""_mm(long double d) { return distance<false>{d}; }
template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.value() + b.value()};
}
template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<!I> &b) {
  return a + static_cast<distance<I>>(b);
}
template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.value() - b.value()};
}
template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<!I> &b) {
  return a - static_cast<distance<I>>(b);
}
template <bool I>
constexpr auto operator*(const distance<I> &a, long double n) {
  return distance<I>{a.value() * n};
}

// https://www.ti.com/lit/ds/symlink/lm555.pdf
// https://www.pcb-3d.com/tutorials/how-to-calculate-pth-hole-and-pad-diameter-sizes-according-to-ipc-7251-ipc-2222-and-ipc-2221-standards/
constexpr const auto pdip_pin_allowance = 0.1_mm;
constexpr const auto pdip_pin_diam_max = 0.021_in;
constexpr const auto pdip_pin_dist = 0.1_in;
constexpr const auto pdip_pin_hole = pdip_pin_diam_max + 0.2_mm;
constexpr const auto pdip_pin_pad = pdip_pin_hole + pdip_pin_allowance + 0.5_mm;
constexpr const auto pdip_width = 0.3_in;
constexpr const auto pdip_draw_w = 0.24_in;
constexpr const auto pdip_draw_cx = pdip_width * 0.5;
constexpr const auto pdip_draw_h = 0.4_in;
constexpr const auto pdip_draw_cy = -pdip_pin_dist * 1.5;

void pdip_pads(auto &p) {
  for (auto i = 0; i < 4; i++) {
    auto n = pdip_pin_dist * -i;
    p.flash(0, n.value());
    p.flash(pdip_width.value(), n.value());
  }
};
void pdip_hole(auto &p) {
  p.aperture(pdip_pin_hole.value());
  pdip_pads(p);
}
void pdip_copper(auto &p) {
  p.aperture(pdip_pin_pad.value());
  pdip_pads(p);
}
void pdip_doc(auto &p, auto cx, auto cy, auto w, auto h) {
  auto l = (cx - w * 0.5).value();
  auto b = (cy - h * 0.5).value();
  auto r = (cx + w * 0.5).value();
  auto t = (cy + h * 0.5).value();

  p.aperture(0.01);

  p.move(l, t);
  p.draw_y(b);
  p.draw_x(r);
  p.draw_y(t);
  p.draw_x(l);

  p.move_y(t - 0.05);
  p.draw(l + 0.05, t);
};

extern "C" void casein_handle(const casein::event &e) {
  static gerby::thread t{[](auto b) {
    b->add_lines([](auto &p) { pdip_copper(p); }, red);
    b->add_lines([](auto &p) { pdip_hole(p); }, black);
    b->add_lines(
        [](auto &p) {
          pdip_doc(p, pdip_draw_cx, pdip_draw_cy, pdip_draw_w, pdip_draw_h);
        },
        white);
  }};
  t.handle(e);
}
