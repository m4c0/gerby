#pragma leco app

import casein;
import gerby;

using namespace gerby::literals;

class pdip {
  // https://www.ti.com/lit/ds/symlink/lm555.pdf
  // https://www.pcb-3d.com/tutorials/how-to-calculate-pth-hole-and-pad-diameter-sizes-according-to-ipc-7251-ipc-2222-and-ipc-2221-standards/
  static constexpr const auto pin_allowance = 0.1_mm;
  static constexpr const auto pin_diam_max = 0.021_in;
  static constexpr const auto pin_dist = 0.1_in;
  static constexpr const auto pin_hole = pin_diam_max + 0.2_mm;
  static constexpr const auto pin_pad = pin_hole + pin_allowance + 0.5_mm;
  static constexpr const auto width = 0.3_in;
  static constexpr const auto draw_w = 0.24_in;
  static constexpr const auto draw_cx = width * 0.5;

  float m_cx{};
  float m_cy{};
  unsigned m_rows;

  void pads(auto &p) const {
    for (auto i = 0; i < m_rows; i++) {
      auto n = pin_dist * -i;
      p.flash(0, n.value());
      p.flash(width.value(), n.value());
    }
  };

public:
  explicit constexpr pdip(unsigned pins) : m_rows{pins / 2} {}

  void holes(auto &p) const {
    p.aperture(pin_hole.value());
    pads(p);
  }
  void copper(auto &p) const {
    p.aperture(pin_pad.value());
    pads(p);
  }
  void doc(auto &p) const {
    float w = draw_w.value();
    float h = (pin_dist * m_rows).value();

    auto draw_cy = -pin_dist * (m_rows / 2.0 - 0.5);
    float cx = (draw_cx + m_cx).value();
    float cy = (draw_cy + m_cy).value();

    auto l = cx - w * 0.5;
    auto b = cy - h * 0.5;
    auto r = cx + w * 0.5;
    auto t = cy + h * 0.5;

    p.aperture(0.01);

    p.move(l, t);
    p.draw_y(b);
    p.draw_x(r);
    p.draw_y(t);
    p.draw_x(l);

    p.move_y(t - 0.05);
    p.draw(l + 0.05, t);
  };
};

extern "C" void casein_handle(const casein::event &e) {
  using namespace gerby::palette;

  static constexpr const auto ic555 = pdip{8};
  static gerby::thread t{[](auto b) {
    b->add_lines([](auto &p) { ic555.copper(p); }, red);
    b->add_lines([](auto &p) { ic555.holes(p); }, black);
    b->add_lines([](auto &p) { ic555.doc(p); }, white);
  }};
  t.handle(e);
}
