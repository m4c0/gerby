export module gerby:fileout;
import :cnc;
import :distance;
import :palette;
import dotz;
import silog;
import yoyo;

namespace gerby::out {
export using lb_t = void(cnc::builder *, cnc::grb_layer);

void fail(const char *msg) {
  silog::log(silog::error, "Failed to write file: %s", msg);
}

class file_pen : public cnc::pen, public cnc::fanner {
  yoyo::file_writer *m_f;

  void dxy(d::inch x, d::inch y, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_f->writef("X%dY%dD%02d*\n", ix, iy, n).take(fail);
  }
  void dx(d::inch x, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    m_f->writef("X%dD%02d*\n", ix, n).take(fail);
  }
  void dy(d::inch y, unsigned n) {
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_f->writef("Y%dD%02d*\n", iy, n).take(fail);
  }

public:
  explicit file_pen(yoyo::file_writer *f) : m_f{f} {}

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
  yoyo::file_writer *m_f;

  void polarity(dotz::vec4 colour) {
    if (colour.x + colour.y + colour.z + colour.w == 0) {
      m_f->writef("%%LPC*%%\n").take(fail);
    } else {
      m_f->writef("%%LPD*%%\n").take(fail);
    }
  }

public:
  explicit file_builder(yoyo::file_writer *f) : m_f{f} {}

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    polarity(colour);

    file_pen p{m_f};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    polarity(colour);
    m_f->writef("G36*\n").take(fail);

    file_pen p{m_f};
    fn(p);

    m_f->writef("G37*\n").take(fail);
  }
};

export void write(lb_t *lb) {
  silog::log(silog::info, "Generating top copper layer");

  auto f = yoyo::file_writer{"out/board.gtl"};
  f.writef("%%FSLAX26Y26*%%\n").take(fail);
  f.writef("%%MOIN*%%\n").take(fail);
  f.writef("G01*\n").take(fail);

  file_builder b{&f};
  lb(&b, cnc::gl_top_copper);

  f.writef("M02*\n").take(fail);
}
} // namespace gerby::out
