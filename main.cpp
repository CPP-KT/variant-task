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

struct throwing_move_t {
  throwing_move_t(throwing_move_t &&) noexcept(false) { throw std::exception(); }
  throwing_move_t &operator=(throwing_move_t &&) = default;
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
  using variant4 = variant<int, double, throwing_move_t>;
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
  bool construct1 = std::is_constructible_v<variant1, in_place_type_t<throwing_move_t>>;
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
  using variant6 = variant<int, std::string, throwing_move_t, double>;
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
