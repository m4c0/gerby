module;
#include <stdio.h>

#define MCT_SYSCALL_IMPLEMENTATION
#include "../mct/mct-syscall.h"

export module gerby:fileout;
import :cnc;
import :distance;
import :palette;
import hai;
import hay;
import dotz;
import print;
import silog;

namespace gerby::out {
export using lb_t = void(cnc::builder *, cnc::grb_layer);

export using file = hay<FILE *, mct_syscall_fopen, ::fclose>;

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

  void set(file & f, cnc::aperture a) {
    unsigned d = 10;
    for (const auto &a1 : m_dict) {
      if (a != a1) {
        d++;
        continue;
      }
      if (m_cur != d) {
        fputfn(f, "D%d*", d);
        m_cur = d;
      }
      return;
    }
  }
  void tool(file & f, cnc::aperture a) {
    unsigned d = 1;
    for (const auto &a1 : m_dict) {
      if (a != a1) {
        d++;
        continue;
      }
      if (m_cur != d) {
        fputfn(f, "T%02d", d);
        m_cur = d;
      }
      return;
    }
  }

  void write(file & f) const {
    unsigned d = 10;
    for (const auto &a : m_dict) {
      fputf(f, "%%ADD%d", d++);
      if (a.roundness() > 0) {
        fputf(f, "C,%.6f", a.diameter());
      } else {
        auto v = a.smear() * 2.0f + a.diameter();
        fputf(f, "R,%.6fX%.6f", v.x, v.y);
      }
      fputfn(f, "*%%");
    }
  }
  void write_tools(file & f) const {
    unsigned d = 1;
    for (const auto &a : m_dict) {
      if (a.roundness() <= 0) {
        silog::log(silog::error, "only circles are supported in drilling");
        continue;
      }
      auto din = a.diameter();
      d::mm dmm{d::inch{din}};
      if (dmm.raw_value() < 0.15 || dmm.raw_value() > 6.3) {
        silog::log(silog::error, "incompatible drill size: %Lfmm/%fin",
                   dmm.raw_value(), din);
        continue;
      }

      fputfn(f, "T%02dC%0.4f", d++, din);
    }
  }
};

class file_pen : public cnc::pen, public cnc::fanner {
  file & m_f;
  apdict *m_ad;

  void dxy(d::inch x, d::inch y, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    fputfn(m_f, "X%dY%dD%02d*", ix, iy, n);
  }
  void dx(d::inch x, unsigned n) {
    int ix = static_cast<int>(x.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    fputfn(m_f, "X%dD%02d*", ix, n);
  }
  void dy(d::inch y, unsigned n) {
    int iy = static_cast<int>(y.raw_value() * 1000000.0);
    m_ad->set(m_f, aperture());
    fputfn(m_f, "Y%dD%02d*", iy, n);
  }

public:
  explicit file_pen(file & f, apdict *ad) : m_f{f}, m_ad{ad} {}

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
  file & m_f;
  apdict *m_ad;

  void polarity(dotz::vec4 colour) {
    if (colour.x + colour.y + colour.z + colour.w == 0) {
      fputfn(m_f, "%%LPC*%%");
    } else {
      fputfn(m_f, "%%LPD*%%");
    }
  }

public:
  explicit file_builder(file & f, apdict *ad) : m_f{f}, m_ad{ad} {}

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    polarity(colour);

    file_pen p{m_f, m_ad};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    polarity(colour);
    fputln(m_f, "G36*");

    file_pen p{m_f, m_ad};
    fn(p);

    fputln(m_f, "G37*");
  }
};

class drill_pen : public cnc::pen, public cnc::fanner {
  file & m_f;
  apdict *m_ad;

  void dxy(d::inch x, d::inch y) {
    m_ad->tool(m_f, aperture());
    fputfn(m_f, "X%.6fY%.6f", x.as_float(), y.as_float());
  }

  void unsup() {
    silog::log(silog::error,
               "only absolute 'stamp' is supported when drilling");
  }

public:
  explicit drill_pen(file & f, apdict *ad) : m_f{f}, m_ad{ad} {}

  void move(d::inch x, d::inch y) override { unsup(); }
  void move_x(d::inch x) override { unsup(); }
  void move_y(d::inch y) override { unsup(); }

  void draw(d::inch x, d::inch y) override { unsup(); }
  void draw_x(d::inch x) override { unsup(); }
  void draw_y(d::inch y) override { unsup(); }

  void flash(d::inch x, d::inch y) override { dxy(x, y); }
  void flash_x(d::inch x) override { unsup(); }
  void flash_y(d::inch y) override { unsup(); }
};

class drill_builder : public cnc::builder {
  file & m_f;
  apdict *m_ad;

public:
  explicit drill_builder(file & f, apdict *ad) : m_f{f}, m_ad{ad} {}

  void add_lines(void (*fn)(cnc::pen &p), dotz::vec4 colour) override {
    drill_pen p{m_f, m_ad};
    fn(p);
  }
  void add_region(void (*fn)(cnc::fanner &p), dotz::vec4 colour) override {
    silog::log(silog::error, "Drilling regions is not supported");
  }
};

void write(const char *fn, lb_t lb, cnc::grb_layer l) {
  silog::log(silog::info, "Generating [%s]", fn);

  apdict ad{};
  lb(&ad, l);

  file f { fn, "wb" };
  fputln(f, "%FSLAX26Y26*%");
  fputln(f, "%MOIN*%");
  fputln(f, "G01*");
  ad.write(f);

  file_builder b { f, &ad };
  lb(&b, l);

  fputln(f, "M02*");
}
void write_drill(const char *fn, lb_t lb) {
  silog::log(silog::info, "Generating drill [%s]", fn);

  apdict ad{};
  lb(&ad, cnc::gl_drill_holes);

  file f { fn, "wb" };
  fputln(f, "M48");
  fputln(f, "INCH");
  ad.write_tools(f);
  fputln(f, "%");
  fputln(f, "G05");

  drill_builder b { f, &ad };
  lb(&b, cnc::gl_drill_holes);

  fputln(f, "M30");
}

export void write(lb_t *lb) {
  write("out/board.gtl", lb, cnc::gl_top_copper);
  write("out/board.gts", lb, cnc::gl_top_mask);
  write("out/board.gto", lb, cnc::gl_top_silk);
  write("out/board.gbl", lb, cnc::gl_bot_copper);
  write("out/board.gbs", lb, cnc::gl_bot_mask);
  write("out/board.gbo", lb, cnc::gl_bot_silk);
  write("out/board.gko", lb, cnc::gl_border);
  write_drill("out/board.xln", lb);
}
} // namespace gerby::out
