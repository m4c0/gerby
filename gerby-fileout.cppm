export module gerby:fileout;
import :cnc;
import :distance;
import dotz;
import silog;

namespace gerby::out {
export using lb_t = void(cnc::builder *, cnc::grb_layer);

class file_pen : public cnc::pen {
public:
  void move(d::inch x, d::inch y) override {}
  void move_x(d::inch x) override {}
  void move_y(d::inch y) override {}

  void draw(d::inch x, d::inch y) override {}
  void draw_x(d::inch x) override {}
  void draw_y(d::inch y) override {}

  void flash(d::inch x, d::inch y) override {}
  void flash_x(d::inch x) override {}
  void flash_y(d::inch y) override {}
};

class file_fanner : public cnc::fanner {
public:
  void move(d::inch x, d::inch y) override {}
  void draw(d::inch x, d::inch y) override {}
  void draw_x(d::inch x) override {}
  void draw_y(d::inch y) override {}
};

class file_builder : public cnc::builder {
public:
  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    file_pen p{};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    file_fanner p{};
    fn(p);
  }
};

export void write(lb_t *lb) {
  silog::log(silog::info, "Generating top copper layer");

  file_builder b{};
  lb(&b, cnc::gl_top_copper);
}
} // namespace gerby::out
