export module gerby:fileout;
import :cnc;
import :distance;
import :palette;
import hai;
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

class apdict : public cnc::pen, public cnc::fanner, public cnc::builder {
  hai::varray<cnc::aperture> m_dict{1024};
  unsigned m_cur{};

  void ap() {
    auto a = aperture();
    if (a.diameter() < 0.000001)
      return;
    for (const auto &a1 : m_dict) {
      if (a == a1)
        return;
    }
    m_dict.push_back(a);
  }

public:
  void move(d::inch x, d::inch y) override { ap(); }
  void move_x(d::inch x) override { ap(); }
  void move_y(d::inch y) override { ap(); }

  void draw(d::inch x, d::inch y) override { ap(); }
  void draw_x(d::inch x) override { ap(); }
  void draw_y(d::inch y) override { ap(); }

  void flash(d::inch x, d::inch y) override { ap(); }
  void flash_x(d::inch x) override { ap(); }
  void flash_y(d::inch y) override { ap(); }

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    fn(*this);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    fn(*this);
  }

  void set(file *f, cnc::aperture a) {
    unsigned d = 10;
    for (const auto &a1 : m_dict) {
      if (a != a1) {
        d++;
        continue;
      }
      if (m_cur != d) {
        f->write("D%d*\n", d);
        m_cur = d;
      }
      return;
    }
  }

  void write(file *f) const {
    unsigned d = 10;
    for (const auto &a : m_dict) {
      f->write("%%AD%d", d++);
      if (a.roundness() > 0) {
        f->write("C,%.6f", a.diameter());
      } else {
        auto v = a.smear() * 2.0f + a.diameter();
        f->write("R,%.6fX%.6f", v.x, v.y);
      }
      f->write("*%%\n");
    }
  }
};

class file_pen : public cnc::pen, public cnc::fanner {
  file *m_f;
  apdict *m_ad;

  void dxy(d::inch x, d::inch y, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    m_f->write("X%dY%dD%02d*\n", ix, iy, n);
  }
  void dx(d::inch x, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    m_f->write("X%dD%02d*\n", ix, n);
  }
  void dy(d::inch y, unsigned n) {
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    m_f->write("Y%dD%02d*\n", iy, n);
  }

public:
  explicit file_pen(file *f, apdict *ad) : m_f{f}, m_ad{ad} {}

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
  apdict *m_ad;

  void polarity(dotz::vec4 colour) {
    if (colour.x + colour.y + colour.z + colour.w == 0) {
      m_f->write("%%LPC*%%\n");
    } else {
      m_f->write("%%LPD*%%\n");
    }
  }

public:
  explicit file_builder(file *f, apdict *ad) : m_f{f}, m_ad{ad} {}

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    polarity(colour);

    file_pen p{m_f, m_ad};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    polarity(colour);
    m_f->write("G36*\n");

    file_pen p{m_f, m_ad};
    fn(p);

    m_f->write("G37*\n");
  }
};

void write(const char *fn, lb_t lb, cnc::grb_layer l) {
  silog::log(silog::info, "Generating [%s]", fn);

  apdict ad{};
  lb(&ad, l);

  file f{fn};
  f.write("%%FSLAX26Y26*%%\n");
  f.write("%%MOIN*%%\n");
  f.write("G01*\n");
  ad.write(&f);

  file_builder b{&f, &ad};
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
