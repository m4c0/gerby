export module gerby:cnc;
import :distance;

import dotz;

namespace gerby::cnc {
export class aperture {
  dotz::vec2 m_smear{};
  float m_d{};
  float m_round{1};

public:
  constexpr aperture() noexcept = default;
  constexpr aperture(d::inch d) noexcept : aperture{d, d, true} {}
  constexpr aperture(d::inch iw, d::inch ih, bool round) noexcept {
    auto w = iw.as_float();
    auto h = ih.as_float();
    if (w > h) {
      m_d = h;
      m_smear = {w - h, 0.0};
    } else {
      m_d = w;
      m_smear = {0.0, h - w};
    }
    m_smear = m_smear / 2.0f;
    m_round = round;
  }

  [[nodiscard]] constexpr auto diameter() const noexcept { return m_d; }
  [[nodiscard]] constexpr auto roundness() const noexcept { return m_round; }
  [[nodiscard]] constexpr auto smear() const noexcept { return m_smear; }

  [[nodiscard]] constexpr bool operator==(const aperture &o) const noexcept {
    auto ss = dotz::sq_length(m_smear - o.m_smear);
    auto dd = m_d - o.m_d;
    auto rr = m_round - m_round;
    return 0.00001f > ss + dd * dd + rr * rr;
  }
};

export class fanner {
public:
  virtual ~fanner() = default;

  virtual void move(d::inch x, d::inch y) = 0;

  virtual void draw(d::inch x, d::inch y) = 0;
  virtual void draw_x(d::inch x) = 0;
  virtual void draw_y(d::inch y) = 0;
};

export class pen {
  cnc::aperture m_aperture{};

protected:
  [[nodiscard]] constexpr const auto aperture() const noexcept {
    return m_aperture;
  }

public:
  virtual ~pen() = default;

  constexpr void aperture(auto a, auto... args) { m_aperture = {a, args...}; }

  virtual void move(d::inch x, d::inch y) = 0;
  virtual void move_x(d::inch x) = 0;
  virtual void move_y(d::inch y) = 0;

  virtual void draw(d::inch x, d::inch y) = 0;
  virtual void draw_x(d::inch x) = 0;
  virtual void draw_y(d::inch y) = 0;

  virtual void flash(d::inch x, d::inch y) = 0;
  virtual void flash_x(d::inch x) = 0;
  virtual void flash_y(d::inch y) = 0;
};

export struct builder {
  virtual ~builder() = default;

  virtual void add_lines(void (*fn)(pen &p), dotz::vec4 colour) = 0;
  virtual void add_region(void (*fn)(fanner &p), dotz::vec4 colour) = 0;
};

export enum grb_layer {
  gl_top_copper = 0,
  gl_top_mask,
  gl_drill_holes,
  gl_count,
};
} // namespace gerby::cnc
