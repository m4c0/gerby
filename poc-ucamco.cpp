#pragma leco dll

import gerby;

void example_lines_1(gerby::cnc::pen &p) {
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

void example_region_1(gerby::cnc::fanner &f) {
  f.move(5, 20);
  f.draw_y(37.5);
  f.draw_x(37.5);
  f.draw_y(20.0);
  f.draw_x(5);
}

void example_region_2(gerby::cnc::fanner &f) {
  f.move(10, 25);
  f.draw_y(30);
  f.draw(12.5, 32.5); // X12500000Y32500000I2500000J0D01*
  f.draw_x(30);
  f.draw(30, 25); // X30000000Y25000000I0J-3750000D01*
  f.draw_x(10);
}

void example_lines_2(gerby::cnc::pen &p) {
  p.aperture(0.1); // D10
  p.move(15, 28.75);
  p.draw_x(20);

  p.aperture(0.6); // D11
  p.flash(15, 28.75);
  p.flash_x(20);
}

extern "C" void draw(gerby::cnc::builder *b, gerby::cnc::grb_layer l) {
  using namespace gerby::palette;

  b->add_lines(example_lines_1, red);
  b->add_region(example_region_1, red);
  b->add_region(example_region_2, black);
  b->add_lines(example_lines_2, red);
}
