#include <exception>
#include <string>
#include <type_traits>
#include <vector>

#include "variant.h"
#include "gtest/gtest.h"

struct dummy_t {};

struct no_default_t {
  no_default_t() = delete;
};

struct throwing_default_t {
  throwing_default_t() { throw std::exception(); }
};

struct throwing_move_operator_t {
  throwing_move_operator_t() = default;
  throwing_move_operator_t(throwing_move_operator_t &&) noexcept(false) { throw std::exception(); }
  throwing_move_operator_t &operator=(throwing_move_operator_t &&) = default;
};

struct no_copy_t {
  no_copy_t(const no_copy_t &) = delete;
};

struct no_move_t {
  no_move_t(no_move_t &&) = delete;
};

struct non_trivial_copy_t {
  non_trivial_copy_t(const non_trivial_copy_t &other) noexcept : x{other.x + 1} {}

  int x;
};

struct non_trivial_copy_assigment_t {
  non_trivial_copy_assigment_t &operator=(const non_trivial_copy_assigment_t &other) {
    x = other.x + 1;
    return *this;
  };

  int x;
};

struct no_move_assigment_t {
  no_move_assigment_t &operator=(no_move_assigment_t &&) = delete;
};

struct no_copy_assigment_t {
  no_copy_assigment_t &operator=(const no_copy_assigment_t &) = delete;
};

struct throwing_move_assigment_t {
  throwing_move_assigment_t(throwing_move_assigment_t &&) = default;
  throwing_move_assigment_t &operator=(throwing_move_assigment_t &&) noexcept(false) { throw std::exception(); }
};

struct throwing_copy_operator_t {
  throwing_copy_operator_t() = default;
  throwing_copy_operator_t(throwing_copy_operator_t const &) = default;
  throwing_copy_operator_t &operator=(throwing_copy_operator_t const &) noexcept(false) { throw std::exception(); }
};

TEST(traits, destructor) {
  using variant1 = variant<int, double, dummy_t>;
  using variant2 = variant<int, std::string>;
  using variant3 = variant<char, long, variant1>;
  using variant4 = variant<char, variant2, int>;
  ASSERT_TRUE(std::is_trivially_destructible_v<variant1>);
  ASSERT_FALSE(std::is_trivially_destructible_v<variant2>);
  ASSERT_TRUE(std::is_trivially_destructible_v<variant3>);
  ASSERT_FALSE(std::is_trivially_destructible_v<variant4>);
}

TEST(traits, default_constructor) {
  using variant1 = variant<std::string, int, std::vector<int>>;
  using variant2 = variant<no_default_t, int>;
  using variant3 = variant<throwing_default_t, int, double>;
  ASSERT_TRUE(std::is_default_constructible_v<variant1>);
  ASSERT_FALSE(std::is_default_constructible_v<variant2>);
  ASSERT_TRUE(std::is_nothrow_default_constructible_v<variant1>);
  ASSERT_FALSE(std::is_nothrow_default_constructible_v<variant3>);
}

TEST(traits, copy_constructor) {
  using variant1 = variant<int, no_copy_t, std::vector<std::string>>;
  using variant2 = variant<std::string, std::vector<std::string>, int>;
  using variant3 = variant<int, double, dummy_t>;
  using variant4 = variant<double, int, non_trivial_copy_t>;
  ASSERT_FALSE(std::is_copy_constructible_v<variant1>);
  ASSERT_TRUE(std::is_copy_constructible_v<variant2>);
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<variant2>);
  ASSERT_TRUE(std::is_trivially_copy_constructible_v<variant3>);
  ASSERT_FALSE(std::is_trivially_copy_constructible_v<variant4>);
}

TEST(traits, move_constructor) {
  using variant1 = variant<int, std::string, no_move_t, double>;
  using variant2 = variant<double, std::string, int>;
  using variant3 = variant<int, dummy_t, char>;
  using variant4 = variant<int, double, throwing_move_operator_t>;
  ASSERT_FALSE(std::is_move_constructible_v<variant1>);
  ASSERT_TRUE(std::is_move_constructible_v<variant2>);
  ASSERT_TRUE(std::is_move_constructible_v<variant3>);
  ASSERT_TRUE(std::is_nothrow_move_constructible_v<variant2>);
  ASSERT_FALSE(std::is_trivially_move_constructible_v<variant2>);
  ASSERT_TRUE(std::is_trivially_move_constructible_v<variant3>);
  ASSERT_TRUE(std::is_move_constructible_v<variant4>);
  ASSERT_FALSE(std::is_nothrow_move_constructible_v<variant4>);
}

TEST(traits, converting_constructor) {
  using variant1 = variant<std::string, std::vector<double>>;
  bool construct1 = std::is_constructible_v<variant1, std::size_t>;
  bool construct2 = std::is_constructible_v<variant1, const char *>;
  bool construct3 = std::is_nothrow_constructible_v<variant1, std::string &&>;
  bool construct4 = std::is_nothrow_constructible_v<variant1, const char *>;
  ASSERT_FALSE(construct1);
  ASSERT_TRUE(construct2);
  ASSERT_TRUE(construct3);
  ASSERT_FALSE(construct4);
}

TEST(traits, in_place_type) {
  using variant1 = variant<int, float, std::string, dummy_t, std::vector<int>, no_default_t>;
  bool construct1 = std::is_constructible_v<variant1, in_place_type_t<throwing_move_operator_t>>;
  bool construct2 = std::is_constructible_v<variant1, in_place_type_t<dummy_t>>;
  bool construct3 = std::is_constructible_v<variant1, in_place_type_t<no_default_t>>;
  bool construct4 = std::is_constructible_v<variant1, in_place_type_t<std::vector<int>>, size_t, int>;
  bool construct5 = std::is_constructible_v<variant1, in_place_type_t<std::vector<int>>, size_t>;
  bool construct6 = std::is_constructible_v<variant1, in_place_type_t<std::string>>;
  ASSERT_FALSE(construct1);
  ASSERT_TRUE(construct2);
  ASSERT_FALSE(construct3);
  ASSERT_TRUE(construct4);
  ASSERT_TRUE(construct5);
  ASSERT_TRUE(construct6);
}

TEST(traits, in_place_index) {
  using variant1 = variant<int, float, std::string, dummy_t, std::vector<int>, no_default_t>;
  bool construct1 = std::is_constructible_v<variant1, in_place_index_t<1337>>;
  bool construct2 = std::is_constructible_v<variant1, in_place_index_t<3>>;
  bool construct3 = std::is_constructible_v<variant1, in_place_index_t<5>>;
  bool construct4 = std::is_constructible_v<variant1, in_place_index_t<4>, size_t, int>;
  bool construct5 = std::is_constructible_v<variant1, in_place_index_t<4>, size_t>;
  bool construct6 = std::is_constructible_v<variant1, in_place_index_t<3>>;
  ASSERT_FALSE(construct1);
  ASSERT_TRUE(construct2);
  ASSERT_FALSE(construct3);
  ASSERT_TRUE(construct4);
  ASSERT_TRUE(construct5);
  ASSERT_TRUE(construct6);
}

TEST(traits, copy_assigment) {
  using variant1 = variant<std::string, double, no_copy_t>;
  using variant2 = variant<std::vector<short>, int, no_copy_assigment_t>;
  using variant3 = variant<dummy_t, int, non_trivial_copy_assigment_t>;
  using variant4 = variant<double, non_trivial_copy_t, bool>;
  using variant5 = variant<int, short, char, dummy_t, bool>;
  ASSERT_FALSE(std::is_copy_assignable_v<variant1>);
  ASSERT_FALSE(std::is_copy_assignable_v<variant2>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant3>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant4>);
  ASSERT_TRUE(std::is_copy_assignable_v<variant5>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<variant3>);
  ASSERT_FALSE(std::is_trivially_copy_assignable_v<variant4>);
  ASSERT_TRUE(std::is_trivially_copy_assignable_v<variant5>);
}

TEST(traits, move_assigment) {
  using variant1 = variant<std::string, double, no_move_t>;
  using variant2 = variant<int, std::vector<std::string>, no_move_assigment_t, bool>;
  using variant3 = variant<dummy_t, int, std::vector<double>>;
  using variant4 = variant<double, std::string, bool>;
  using variant5 = variant<int, short, char, dummy_t, bool>;
  using variant6 = variant<int, std::string, throwing_move_operator_t, double>;
  using variant7 = variant<int, throwing_move_assigment_t, double>;
  ASSERT_FALSE(std::is_move_assignable_v<variant1>);
  ASSERT_FALSE(std::is_move_assignable_v<variant2>);
  ASSERT_TRUE(std::is_move_assignable_v<variant3>);
  ASSERT_TRUE(std::is_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_move_assignable_v<variant5>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<variant3>);
  ASSERT_FALSE(std::is_trivially_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_trivially_move_assignable_v<variant5>);
  ASSERT_TRUE(std::is_move_assignable_v<variant6>);
  ASSERT_TRUE(std::is_move_assignable_v<variant7>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<variant6>);
  ASSERT_FALSE(std::is_nothrow_move_assignable_v<variant7>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant3>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant4>);
  ASSERT_TRUE(std::is_nothrow_move_assignable_v<variant5>);
}

TEST(traits, converting_assigment) {
  using variant1 = variant<std::string, std::vector<char>, bool>;
  bool assigment1 = std::is_assignable_v<variant1 &, std::string &&>;
  bool assigment2 = std::is_assignable_v<variant1 &, const char *>;
  bool assigment3 = std::is_assignable_v<variant1 &, size_t>;
  bool assigment4 = std::is_nothrow_assignable_v<variant1 &, std::string &&>;
  bool assigment5 = std::is_nothrow_assignable_v<variant1 &, const std::string &>;
  bool assigment6 = std::is_assignable_v<variant1 &, double *>;
  ASSERT_TRUE(assigment1);
  ASSERT_TRUE(assigment2);
  ASSERT_FALSE(assigment3);
  ASSERT_TRUE(assigment4);
  ASSERT_FALSE(assigment5);
  ASSERT_FALSE(assigment6);
}

TEST(traits, variant_size) {
  using variant1 = variant<int, std::string, variant<int, std::vector<int>, size_t>, bool>;
  ASSERT_EQ(variant_size_v<variant1>, 4);
  ASSERT_EQ(variant_size_v<variant1>, variant_size_v<const variant1>);
  ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>::value);
  ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>{});
  ASSERT_EQ(variant_size_v<variant1>, variant_size<variant1>{}());
}

TEST(traits, variant_alternative) {
  using variant1 = variant<int, std::string, variant<int, std::vector<int>, size_t>, bool>;
  using T1 = variant_alternative_t<1, variant1>;
  using T2 = typename variant_alternative<1, variant1>::type;
  using T3 = variant_alternative_t<1, const variant1>;
  bool res1 = std::is_same_v<T1, std::string>;
  bool res2 = std::is_same_v<T1, T2>;
  bool res3 = std::is_same_v<const T1, T3>;
  ASSERT_TRUE(res1);
  ASSERT_TRUE(res2);
  ASSERT_TRUE(res3);
}

static_assert(variant<int>().index() == 0, "Constexpr empty ctor failed");
static_assert(holds_alternative<int>(variant<int, double>()), "Constexpr empty ctor holds_alternative test failed");
static_assert(holds_alternative<int>(variant<int>()), "Constexpr empty ctor holds_alternative test failed");
static_assert(variant<int, double>().index() == 0, "Constexpr empty ctor failed");

TEST(correctness, empty_ctor) {
  variant<int, double> v;
  ASSERT_TRUE(v.index() == 0);
  ASSERT_TRUE(holds_alternative<int>(v));
}

constexpr bool simple_copy_ctor_test() {
  variant<int, double> x{42.0};
  variant<int, double> other{x};
  if (x.index() != other.index())
    return false;
  if (get<1>(x) != get<1>(other))
    return false;
  if (!holds_alternative<double>(x) || !holds_alternative<double>(other))
    return false;
  return true;
}

static_assert(simple_copy_ctor_test(), "Basic constexpr copy-constructor failed");

TEST(correctness, copy_ctor1) { ASSERT_TRUE(simple_copy_ctor_test()); }

constexpr bool direct_init_copy_ctor() {
  variant<no_copy_assigment_t> x;
  variant<no_copy_assigment_t> other{x};
  if (!holds_alternative<no_copy_assigment_t>(x) || !holds_alternative<no_copy_assigment_t>(other))
    return false;
  return true;
}

TEST(correctness, copy_ctor2) { ASSERT_TRUE(direct_init_copy_ctor()); }

struct only_movable {
  constexpr only_movable() = default;

  constexpr only_movable(only_movable &&other) noexcept {
    assert(other.coin && "Move of moved value?");
    coin = true;
    other.coin = false;
  }

  constexpr only_movable &operator=(only_movable &&other) noexcept {
    assert(other.coin && "Move of moved value?");
    coin = true;
    other.coin = false;
    return *this;
  }

  constexpr bool has_coin() noexcept { return coin; }

private:
  only_movable(only_movable const &) = delete;
  only_movable &operator=(only_movable const &) = delete;

private:
  bool coin{true};
};

struct yac_coin {
  constexpr operator int() noexcept { return 42; }
};

struct coin_wrapper {
  constexpr coin_wrapper() noexcept = default;

  constexpr coin_wrapper(coin_wrapper &&other) noexcept {
    assert(other.coin && "Move of moved value?");
    coin = 1;
    other.coin = 0;
  }

  constexpr coin_wrapper &operator=(coin_wrapper &&other) noexcept {
    assert(other.coin && "Move of moved value?");
    coin = 1;
    other.coin = 0;
    return *this;
  }

  constexpr auto has_coins() const noexcept { return coin; }

  constexpr explicit coin_wrapper(yac_coin) noexcept : coin{17} {}

  constexpr coin_wrapper(coin_wrapper const &other) noexcept : coin(other.coin + 1) {}

  constexpr coin_wrapper &operator=(coin_wrapper const &other) noexcept {
    coin = other.coin + 1;
    return *this;
  }

private:
  int coin{1};
};

constexpr bool simple_move_ctor_test() {
  {
    variant<no_copy_assigment_t> x;
    variant<no_copy_assigment_t> other{std::move(x)};
    if (!holds_alternative<no_copy_assigment_t>(x) || !holds_alternative<no_copy_assigment_t>(other))
      return false;
  }
  {
    variant<int, double> x{42};
    variant<int, double> y = std::move(x);
    if (x.index() != y.index()
     || x.index() != 0
     || get<0>(x) != get<0>(y))
      return false;
  }
  return true;
}

static_assert(simple_move_ctor_test(), "Simple constexpr move test failed");

TEST(corectness, move_ctor) {
  simple_move_ctor_test();

  variant<coin_wrapper> x;
  variant<coin_wrapper> y = std::move(x);
  ASSERT_TRUE(!get<0>(x).has_coins());
  ASSERT_TRUE(get<0>(y).has_coins() == 1);
}

constexpr bool simple_value_move_ctor() {
  {
    only_movable x;
    variant<only_movable> y(std::move(x));
    if (x.has_coin() || !get<0>(y).has_coin())
      return false;
  }
  {
    coin_wrapper x;
    variant<coin_wrapper> y(std::move(x));
    if (x.has_coins() || !get<0>(y).has_coins())
      return false;
  }
  return true;
}

static_assert(simple_value_move_ctor(), "Simple value-forwarding ctor failed");

TEST(correctness, value_move_ctor) {
  simple_value_move_ctor();
  variant<int, coin_wrapper> x(yac_coin{});
  ASSERT_TRUE(x.index() == 0);

  // For brave and truth
  {
    // See NB in #4 https://en.cppreference.com/w/cpp/utility/variant/variant
    variant<bool, std::string> v("asdasd");
    ASSERT_TRUE(v.index() == 1); // Overload resolution is not your friend anymore
  }
}

constexpr bool in_place_ctor() {
  variant<bool, double> x1(in_place_type<double>, 42);
  variant<bool, double> x2(in_place_index<1>, 42);
  return (x1.index() == 1 && get<1>(x1) == 42.0)
      && (x2.index() == 1 && get<1>(x2) == 42.0);
}

static_assert(in_place_ctor(), "Simple in-place ctor failed");

TEST(correctness, inplace_ctors) {
  in_place_ctor();

  variant<bool, std::string> why_not(in_place_type<bool>, "asdasd");
  ASSERT_TRUE(why_not.index() == 0 && get<0>(why_not));

  variant<bool, std::string> x2(in_place_index<0>, "asdasd");
  ASSERT_TRUE(x2.index() == 0 && get<0>(x2));

  std::variant<std::string, std::vector<int>, char> var{in_place_index<1>, {1, 2, 3, 4, 5}};
  auto other = std::vector<int>{1, 2, 3, 4, 5};
  ASSERT_EQ(get<1>(var), other);
  auto other2 = std::vector<int>(4, 42);
  std::variant<std::string, std::vector<int>, char> var2{in_place_index<1>, 4, 42};
  ASSERT_EQ(get<1>(var2), other2);
}

TEST(correctness, variant_exceptions1) {
  using test_t = throwing_move_operator_t;
  using var_t = variant<throwing_move_operator_t>;

  {
    var_t x;
    try {
      x.emplace<test_t>(test_t{});
    } catch (std::exception const &item) {
      ASSERT_TRUE(x.valueless_by_exception());
      ASSERT_THROW(get<0>(x), bad_variant_access);
    }
  }

  {
    var_t x;
    try {
      x = test_t{};
    } catch (std::exception const &item) {
      ASSERT_TRUE(x.valueless_by_exception());
      ASSERT_THROW(get<0>(x), bad_variant_access);
    }
  }
}

TEST(correctness, variant_exceptions2) {
  variant<int, double> x;
  ASSERT_THROW(get<1>(x), bad_variant_access);
}
