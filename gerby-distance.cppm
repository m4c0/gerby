export module gerby:distance;

export namespace gerby {
enum distance_type { inch, mm, mil };

constexpr long double convert(distance_type from, distance_type to,
                              long double v) {
  static constexpr const auto inch2mm = 25.4;
  static constexpr const auto inch2mil = 1000.0;
  static constexpr const auto mil2mm = 0.0254;

  switch (from) {
  case inch:
    switch (to) {
    case inch:
      return v;
    case mm:
      return v * inch2mm;
    case mil:
      return v * inch2mil;
    }
  case mm:
    switch (to) {
    case inch:
      return v / inch2mm;
    case mm:
      return v;
    case mil:
      return v / mil2mm;
    }
  case mil:
    switch (to) {
    case inch:
      return v / inch2mil;
    case mm:
      return v * mil2mm;
    case mil:
      return v;
    }
  }
}

template <distance_type DT> class distance {
  long double m_val;

public:
  constexpr distance() noexcept = default;
  constexpr distance(long double d) noexcept : m_val{d} {}
  constexpr distance(distance<DT> &&o) noexcept = default;
  constexpr distance(const distance<DT> &o) noexcept = default;
  template <distance_type DT2>
    requires requires { DT != DT2; }
  constexpr distance(const distance<DT2> &o) noexcept
      : m_val{convert(DT2, DT, o.raw_value())} {}

  constexpr distance &operator=(distance<DT> &&o) noexcept = default;
  constexpr distance &operator=(const distance<DT> &o) noexcept = default;

  [[nodiscard]] constexpr auto raw_value() const noexcept { return m_val; }
  [[nodiscard]] constexpr auto value() const noexcept {
    return convert(DT, inch, m_val);
  }

  [[nodiscard]] constexpr auto operator-() const noexcept {
    return distance<DT>{-m_val};
  }

  [[nodiscard]] constexpr float as_float() const noexcept {
    return static_cast<float>(value());
  }

  [[nodiscard]] constexpr auto abs() const noexcept {
    return distance<DT>{m_val > 0 ? m_val : -m_val};
  }
  [[nodiscard]] constexpr float sign() const noexcept {
    return m_val > 0 ? 1 : -1;
  }

  template <distance_type O>
  constexpr auto operator+(const distance<O> &o) const noexcept {
    return distance<DT>{m_val + convert(O, DT, o.raw_value())};
  }
  template <distance_type O>
  constexpr auto operator-(const distance<O> &o) const noexcept {
    return distance<DT>{m_val - convert(O, DT, o.raw_value())};
  }

  template <distance_type O>
  constexpr auto operator>(const distance<O> &o) const noexcept {
    return m_val > convert(O, DT, o.raw_value());
  }
  template <distance_type O>
  constexpr auto operator<(const distance<O> &o) const noexcept {
    return m_val < convert(O, DT, o.raw_value());
  }

  constexpr auto operator*(long double n) const noexcept {
    return distance<DT>{m_val * n};
  }
};
} // namespace gerby

export namespace gerby::d {
using inch = gerby::distance<gerby::inch>;
using mm = gerby::distance<gerby::mm>;
using mil = gerby::distance<gerby::mil>;
}; // namespace gerby::d

export namespace gerby::literals {
constexpr auto operator""_in(long double d) { return distance<inch>{d}; }
constexpr auto operator""_mm(long double d) { return distance<mm>{d}; }
constexpr auto operator""_mil(long double d) { return distance<mil>{d}; }
} // namespace gerby::literals

namespace {
static constexpr int _(auto n) {
  return static_cast<int>(n.as_float() * 100.0f);
}

using namespace gerby::d;
using namespace gerby::literals;
static_assert(_(1.25_in) == 125);
static_assert(_(2.54_mm) == 10);
static_assert(_(inch{2.54_mm}) == 10);
static_assert(_(mm{1.25_in}) == 125);
static_assert(_(1.25_in + 2.54_mm) == 135);
static_assert(_(2.54_mm + 1.25_in) == 135);
} // namespace
