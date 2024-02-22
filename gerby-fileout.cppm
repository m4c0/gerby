export module gerby:fileout;
import :cnc;
import :distance;
import :palette;
import dotz;
import silog;

namespace gerby::out {
export using lb_t = void(cnc::builder *, cnc::grb_layer);

class file_pen : public cnc::pen, public cnc::fanner {
  void dxy(d::inch x, d::inch y, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    silog::log(silog::debug, "X%dY%dD%02d", ix, iy, n);
  }
  void dx(d::inch x, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    silog::log(silog::debug, "X%dD%02d", ix, n);
  }
  void dy(d::inch y, unsigned n) {
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    silog::log(silog::debug, "Y%dD%02d", iy, n);
  }

public:
  void move(d::inch x, d::inch y) override { dxy(x, y, 2); }
  void move_x(d::inch x) override { dx(x, 2); }
  void move_y(d::inch y) override { dy(y, 2); }

  void draw(d::inch x, d::inch y) override { dxy(x, y, 1); }
  void draw_x(d::inch x) override { dx(x, 1); }
  void draw_y(d::inch y) override { dy(y, 1); }

  void flash(d::inch x, d::inch y) override { dxy(x, y, 3); }
  void flash_x(d::inch x) override { dx(x, 3); }
  void flash_y(d::inch y) override { dy(y, 3); }
};

class file_builder : public cnc::builder {
  void polarity(dotz::vec4 colour) {
    if (colour.x + colour.y + colour.z + colour.w == 0) {
      silog::log(silog::debug, "%%LPC*%%");
    } else {
      silog::log(silog::debug, "%%LPD*%%");
    }
  }

public:
  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    polarity(colour);

    file_pen p{};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    polarity(colour);
    silog::log(silog::debug, "G36*");

    file_pen p{};
    fn(p);

    silog::log(silog::debug, "G37*");
  }
};

export void write(lb_t *lb) {
  silog::log(silog::info, "Generating top copper layer");

  silog::log(silog::debug, "%%FSLAX26Y26*%%");
  silog::log(silog::debug, "%%MOIN*%%");
  silog::log(silog::debug, "G01*");

  file_builder b{};
  lb(&b, cnc::gl_top_copper);

  silog::log(silog::debug, "M02*");
}
} // namespace gerby::out
