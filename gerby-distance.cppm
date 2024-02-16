export module gerby:distance;

export namespace gerby {
template <bool Inches> class distance {
  long double m_val;

public:
  constexpr distance() noexcept = default;
  constexpr distance(long double d) noexcept : m_val{d} {}
  constexpr distance(distance<Inches> &&o) noexcept = default;
  constexpr distance(const distance<Inches> &o) noexcept = default;
  constexpr distance(const distance<!Inches> &o) noexcept;

  [[nodiscard]] constexpr auto raw_value() const noexcept { return m_val; }
  [[nodiscard]] constexpr auto value() const noexcept;

  [[nodiscard]] constexpr auto operator-() const noexcept {
    return distance<Inches>{-m_val};
  }

  [[nodiscard]] constexpr float as_float() const noexcept {
    return static_cast<float>(value());
  }
};

template <>
constexpr distance<true>::distance(const distance<false> &o) noexcept
    : distance{o.raw_value() / 25.4} {}

template <>
constexpr distance<false>::distance(const distance<true> &o) noexcept
    : distance{o.raw_value() * 25.4} {}

template <>
[[nodiscard]] constexpr auto distance<true>::value() const noexcept {
  return m_val;
}
template <>
[[nodiscard]] constexpr auto distance<false>::value() const noexcept {
  return m_val / 25.4;
}

template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.raw_value() + b.raw_value()};
}
template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<!I> &b) {
  return a + static_cast<distance<I>>(b);
}

template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.raw_value() - b.raw_value()};
}
template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<!I> &b) {
  return a - static_cast<distance<I>>(b);
}

template <bool I>
constexpr auto operator*(const distance<I> &a, long double n) {
  return distance<I>{a.raw_value() * n};
}
} // namespace gerby

export namespace gerby::d {
using inch = gerby::distance<true>;
using mm = gerby::distance<false>;
}; // namespace gerby::d

export namespace gerby::literals {
constexpr auto operator""_in(long double d) { return distance<true>{d}; }
constexpr auto operator""_mm(long double d) { return distance<false>{d}; }
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
