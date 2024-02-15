export module gerby:distance;

export namespace gerby {
template <bool Inches> class distance {
  long double m_val;

public:
  explicit constexpr distance(long double d) noexcept : m_val{d} {}

  [[nodiscard]] constexpr auto value() const noexcept;

  [[nodiscard]] constexpr auto operator-() const noexcept {
    return distance<Inches>{-m_val};
  }

  [[nodiscard]] explicit constexpr operator distance<!Inches>() const noexcept;
};

template <>
[[nodiscard]] constexpr auto distance<true>::value() const noexcept {
  return m_val;
}
template <>
[[nodiscard]] constexpr auto distance<false>::value() const noexcept {
  return m_val / 25.4;
}

template <>
[[nodiscard]] constexpr distance<true>::operator distance<false>()
    const noexcept {
  return distance<false>{m_val * 25.4};
}
template <>
[[nodiscard]] constexpr distance<false>::operator distance<true>()
    const noexcept {
  return distance<true>{m_val / 25.4};
}

template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.value() + b.value()};
}
template <bool I>
constexpr auto operator+(const distance<I> &a, const distance<!I> &b) {
  return a + static_cast<distance<I>>(b);
}

template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<I> &b) {
  return distance<I>{a.value() - b.value()};
}
template <bool I>
constexpr auto operator-(const distance<I> &a, const distance<!I> &b) {
  return a - static_cast<distance<I>>(b);
}

template <bool I>
constexpr auto operator+(const distance<I> &a, long double n) {
  return distance<I>{a.value() + n};
}
template <bool I>
constexpr auto operator-(const distance<I> &a, long double n) {
  return distance<I>{a.value() - n};
}
template <bool I>
constexpr auto operator*(const distance<I> &a, long double n) {
  return distance<I>{a.value() * n};
}
} // namespace gerby

export namespace gerby::literals {
constexpr auto operator""_in(long double d) { return distance<true>{d}; }
constexpr auto operator""_mm(long double d) { return distance<false>{d}; }
} // namespace gerby::literals
