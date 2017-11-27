#include <string>         // std::string
#include <cctype>         // isdigit
#include <vector>         // std::vector
#include <cstdlib>        // strtoul
#include <cstdint>        // uint32_t
#include <iostream>       // std::cout
#include <exception>      // std::runtime_error
#include <type_traits>    // std::is_unsigned

using value_type = uint32_t;
static_assert(std::is_unsigned<value_type>(), "Value type is signed!");

template <value_type n_max>
class Lookup {
  protected:
    value_type lookup[n_max + 1];
  public:
    constexpr Lookup() : lookup() {}
    constexpr value_type operator()(value_type i) const {
      if (i > n_max) {
        return 0;
      }
      return lookup[i];
    }
};

template <value_type n_max>
class Factorial : public Lookup<n_max> {
  public:
    constexpr Factorial(value_type modulo) : Lookup<n_max>() {
      auto value = value_type(1);
      Lookup<n_max>::lookup[0] = value_type(1);
      for (auto i = value_type(1); i <= n_max; i++) {
        value = (uint64_t(value) * uint64_t(i)) % modulo;
        Lookup<n_max>::lookup[i] = value;
      }
    }
};

constexpr value_type mod_exp(
  value_type value, 
  value_type exponent, 
  value_type modulo
) {
  auto result = value_type(1);
  while (exponent) {
    if (exponent % 2) {
      result = (uint64_t(value) * uint64_t(result)) % modulo;
    }
    value = (uint64_t(value) * uint64_t(value)) % modulo;
    exponent /= 2;
  }
  return result;
}

template <value_type n_max>
class InvFactorial : public Lookup<n_max> {
  public:
    constexpr InvFactorial(value_type modulo) : Lookup<n_max>() {
      Lookup<n_max>::lookup[0] = value_type(1);
      auto value = value_type(1);
      for (auto i = value_type(1); i <= n_max; i++) {
        value = (uint64_t(value) * uint64_t(i)) % modulo;
        Lookup<n_max>::lookup[i] = mod_exp(value, modulo - 2, modulo);
      }
    }
};

template <value_type n_max>
class Binomial {
    Factorial<n_max> factorial;
    InvFactorial<n_max> inv_factorial;
    const value_type modulo;
  public:
    constexpr Binomial(value_type mod) 
      : modulo(mod), factorial(mod), inv_factorial(mod) {}
    constexpr value_type operator()(value_type n, value_type k) const {
      if (n < k) {
        return 0;
      }
      uint64_t result = uint64_t(factorial(n)) * uint64_t(inv_factorial(k));
      result = (result % modulo) * uint64_t(inv_factorial(n - k));
      return result % modulo;
    }
};

constexpr value_type NMAX = value_type(100000);
constexpr value_type MODULO = value_type(1000000007);

constexpr bool is_prime(value_type x) {
  if ((x < 2) || (x % 2 == 0)) {
    return x == 2; 
  }
  for (value_type i = 3; (i * i) <= x; i += 2) {
    if (x % i == 0) {
      return false;
    }
  }
  return true;
}

static_assert(is_prime(MODULO), "MODULO is not prime!");
static_assert((MODULO & ~uint32_t(1 << 31)) == MODULO, "MODULO not 31 bit!");

// This needs a lot of RAM to compile for large NMAX
static constexpr Binomial<NMAX> binomial(MODULO);

std::vector<int64_t> parse_stdin() {

  // get stream length
  std::cin.seekg(0, std::ios_base::end);
  std::size_t len = std::cin.tellg();
  std::cin.seekg(0, std::ios_base::beg);

  // allocate for reading & reserve for output
  std::string chars(len, ' ');
  std::vector<int64_t> out;
  out.reserve(len);

  // read
  std::cin.read(static_cast<char *>(&chars[0]), len);

  // parse
  auto ptr = chars.data();
  const auto end = chars.data() + len;
  while (ptr != end) {

    // skip until next digit
    while (!isdigit(*ptr)) {
      if (ptr == end) {
        goto parse_stdin_end;
      }
      ++ptr;
    }

    // try parsing to uint32_t
    char * e_ptr = nullptr;
    auto value = strtoll(ptr, &e_ptr, 10);

    // check parse was successful
    if (e_ptr == ptr) {
      throw std::runtime_error("Error: Could not parse integer from input!");
    }
    ptr = e_ptr;
    out.push_back(value);

  }
  parse_stdin_end:
  return out;
}

inline value_type composition(value_type n, value_type c) {
  static constexpr value_type lookup[] = {0, 1};
  return n == 0 ? lookup[c == 0] : binomial(n - 1, c - 1);
}

inline value_type formula(value_type n, value_type m, value_type c) {
  uint64_t w = composition(n - m, c);
  uint64_t x = (w * uint64_t(composition(m, c - 1))) % MODULO;
  uint64_t y = (w * uint64_t(composition(m, c + 1))) % MODULO;
  uint64_t z = (w * uint64_t(composition(m, c))) % MODULO;
  z = (z * 2) % MODULO;
  return (((x + y) % MODULO) + z) % MODULO;
}

int main(int argc, char** argv) {
  try {
    auto input = parse_stdin();
    for (size_t i = 1; i < input.size(); i += 3) {
      std::cout << formula(input[i], input[i + 1], input[i + 2]) << "\n";
    }
  } catch(std::exception& exception) {
    std::cout << exception.what() << "\n";
  }
  return 0;
}