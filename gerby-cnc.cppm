export module gerby:cnc;
import :distance;

import dotz;

namespace gerby::cnc {
class aperture {
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
};
} // namespace gerby::cnc
