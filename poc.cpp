#pragma leco app

import casein;
import dotz;
import gerby;

using namespace gerby::literals;

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
  using namespace gerby::palette;

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
