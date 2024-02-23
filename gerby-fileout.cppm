export module gerby:fileout;
import :cnc;
import :distance;
import :palette;
import dotz;
import silog;
import yoyo;

namespace gerby::out {
export using lb_t = void(cnc::builder *, cnc::grb_layer);

class file {
  yoyo::file_writer m_f;

public:
  explicit file(const char *name) : m_f{name} {}

  void write(const char *fmt, auto... args) {
    m_f.writef(fmt, args...).take([](auto err) {
      silog::log(silog::error, "Failed to write file: %s", err);
    });
  }
};

class file_pen : public cnc::pen, public cnc::fanner {
  file *m_f;

  void dxy(d::inch x, d::inch y, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_f->write("X%dY%dD%02d*\n", ix, iy, n);
  }
  void dx(d::inch x, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    m_f->write("X%dD%02d*\n", ix, n);
  }
  void dy(d::inch y, unsigned n) {
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_f->write("Y%dD%02d*\n", iy, n);
  }

public:
  explicit file_pen(file *f) : m_f{f} {}

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
  file *m_f;

  void polarity(dotz::vec4 colour) {
    if (colour.x + colour.y + colour.z + colour.w == 0) {
      m_f->write("%%LPC*%%\n");
    } else {
      m_f->write("%%LPD*%%\n");
    }
  }

public:
  explicit file_builder(file *f) : m_f{f} {}

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    polarity(colour);

    file_pen p{m_f};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    polarity(colour);
    m_f->write("G36*\n");

    file_pen p{m_f};
    fn(p);

    m_f->write("G37*\n");
  }
};

void write(const char *fn, lb_t lb, cnc::grb_layer l) {
  silog::log(silog::info, "Generating [%s]", fn);

  file f{fn};
  f.write("%%FSLAX26Y26*%%\n");
  f.write("%%MOIN*%%\n");
  f.write("G01*\n");

  file_builder b{&f};
  lb(&b, l);

  f.write("M02*\n");
}

export void write(lb_t *lb) {
  write("out/board.gtl", lb, cnc::gl_top_copper);
  write("out/board.gts", lb, cnc::gl_top_mask);
  write("out/board.gko", lb, cnc::gl_border);
  write("out/board.xln", lb, cnc::gl_drill_holes);
}
} // namespace gerby::out
