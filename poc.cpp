#pragma leco app

import casein;
import dotz;
import gerby;
import hai;
import voo;

void example_lines_1(gerby::pen &p) {
  p.aperture(0.1); // D10C,0.1
  p.move(0, 2.5);
  p.draw(0, 0);
  p.draw(2.5, 0);
  p.move(10, 10);
  p.draw_x(15);
  p.draw(20, 15);
  p.move_x(25);
  p.draw_y(10);

  p.aperture(0.6); // D11C,0.6
  p.flash(10, 10);
  p.flash_x(20);
  p.flash_x(25);
  p.flash_y(15);
  p.flash_x(20);

  // we simulate rectangles/obround with a small segment

  p.aperture(0.6, 0.6, false); // D12R,0.6x0.6
  p.flash(10, 15);

  p.aperture(0.4, 1.0, false); // D13R,0.4x1.00
  p.flash(30, 15);

  p.aperture(1.0, 0.4, false); // D14R,1.00X0.4
  p.flash_y(12.5);

  p.aperture(0.4, 1.0, true); // D15O0.4X1.00
  p.flash_y(10);

  p.aperture(0.1); // D10C,0.1
  p.move(37.5, 10);
  // p.arc(37.5, 10, 25.0, 0);

  p.aperture(1);
  // p.aperture(1, 3); // D16P,1.00X3 "triangle"
  p.flash(34, 10);
  p.flash(35, 9);
}

void example_region_1(gerby::fanner &f) {
  f.move(5, 20);
  f.draw_y(37.5);
  f.draw_x(37.5);
  f.draw_y(20.0);
  f.draw_x(5);
}

void example_region_2(gerby::fanner &f) {
  f.move(10, 25);
  f.draw_y(30);
  f.draw(12.5, 32.5); // X12500000Y32500000I2500000J0D01*
  f.draw_x(30);
  f.draw(30, 25); // X30000000Y25000000I0J-3750000D01*
  f.draw_x(10);
}

void example_lines_2(gerby::pen &p) {
  p.aperture(0.1); // D10
  p.move(15, 28.75);
  p.draw_x(20);

  p.aperture(0.6); // D11
  p.flash(15, 28.75);
  p.flash_x(20);
}

constexpr const dotz::vec4 red{1, 0, 0, 0};
constexpr const dotz::vec4 black{0, 0, 0, 0};

void build_example(gerby::builder *b) {
  b->add_lines(example_lines_1, red);
  b->add_region(example_region_1, red);
  b->add_region(example_region_2, black);
  b->add_lines(example_lines_2, red);
}
auto &example_thread() {
  static gerby::thread t{build_example};
  return t;
}

template <bool Inches> class distance {
  long double m_val;

public:
  explicit constexpr distance(long double d) noexcept : m_val{d} {}

  [[nodiscard]] constexpr auto value() const noexcept { return m_val; }

  [[nodiscard]] explicit constexpr operator distance<!Inches>() const noexcept;
};

template <>
[[nodiscard]] constexpr distance<true>::operator distance<false>()
    const noexcept {
  return distance<false>{m_val * 2.54};
}
template <>
[[nodiscard]] constexpr distance<false>::operator distance<true>()
    const noexcept {
  return distance<true>{m_val / 2.54};
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
constexpr auto operator*(const distance<I> &a, long double n) {
  return distance<I>{a.value() * n};
}

// https://www.ti.com/lit/ds/symlink/lm555.pdf
// https://www.pcb-3d.com/tutorials/how-to-calculate-pth-hole-and-pad-diameter-sizes-according-to-ipc-7251-ipc-2222-and-ipc-2221-standards/
constexpr const auto pdip_pin_allowance = 0.1_mm;
constexpr const auto pdip_pin_diam_max = 0.021_in;
constexpr const auto pdip_pin_hole = pdip_pin_diam_max + 0.2_mm;
constexpr const auto pdip_pin_pad = pdip_pin_hole + pdip_pin_allowance + 0.5_mm;
constexpr const auto pdip_width = 0.3_in;
auto &pcb_example() {
  static gerby::thread t{[](auto b) {
    b->add_lines(
        [](auto &p) {
          p.aperture(pdip_pin_pad.value());
          for (auto i = 0; i < 4; i++) {
            auto x = 0.1_in * i;
            p.flash(x.value(), 0);
            p.flash(x.value(), pdip_width.value());
          }
        },
        red);
    b->add_lines(
        [](auto &p) {
          p.aperture(pdip_pin_hole.value());
          for (auto i = 0; i < 4; i++) {
            auto x = 0.1_in * i;
            p.flash(x.value(), 0);
            p.flash(x.value(), pdip_width.value());
          }
        },
        black);
  }};
  return t;
}

extern "C" void casein_handle(const casein::event &e) {
  // example_thread().handle(e);
  pcb_example().handle(e);
}
