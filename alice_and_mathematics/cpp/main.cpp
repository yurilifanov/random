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

/** Simple check for whether input integer x is a prime.
 */ 
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

/** Recursive routine computing the binomial coefficient
 *  modulo some integer.
 */ 
constexpr value_type recursive_binomial(
  value_type n, 
  value_type k, 
  value_type modulo
) {

  if (k > n) {
    return 0;
  } else if (k == 0 || n == 0) {
    return 1;
  } else if (k == 1 && n > 0) {
    return n % modulo;
  } else {
    return (uint64_t(recursive_binomial(n - 1, k, modulo))
            + uint64_t(recursive_binomial(n - 1, k - 1, modulo))) % modulo;
  }
}

/** Modular exponentiation via squaring.
 */ 
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

/** Simple compile time lookup base template. 
 *  Internal array is zero initialised.
 */ 
template <typename array_type, value_type n_max>
class Lookup {
  protected:
    array_type lookup[n_max + 1];
  public:
    constexpr Lookup() : lookup() {}
    constexpr array_type operator()(value_type i) const {
      if (i > n_max) {
        return 0;
      }
      return lookup[i];
    }
};

/** Lookup for checking if an integer is prime.
 */ 
template <value_type n_max>
class IsPrime : public Lookup<bool, n_max> {
  public:
    constexpr IsPrime() : Lookup<bool, n_max>() {
      for (auto i = value_type(1); i <= n_max; i++) {
        Lookup<bool, n_max>::lookup[i] = is_prime(i);
      }
    }
};

/** Lookup for the values 
 *  A_n = \prod_{k = 0} ^ m (k + 2) ^ {n \choose k}
 *  modulo some integer.
 */ 
template <value_type n_max>
class Product : public Lookup<value_type, n_max> {
    const value_type modulo;
    const value_type totient;
  public:
    constexpr Product(value_type imodulo, value_type itotient) 
      : Lookup<value_type, n_max>(), modulo(imodulo), totient(itotient) 
    {
      Lookup<value_type, n_max>::lookup[0] = value_type(2);
      for (auto i = value_type(1); i <= n_max; i++) {
        auto value = value_type(2);
        for (auto j = value_type(1); j <= i; j++) {
          auto k = j > (i - j) ? i - j : j;
          auto q = mod_exp(2 + j, recursive_binomial(i, k, totient), modulo);
          value = (uint64_t(value) * uint64_t(q)) % modulo;
        }
        Lookup<value_type, n_max>::lookup[i] = value;
      }
    }
};

/** Returns the result for n - number of numbers in input list
 *  m - number of primes among the n input numbers.
 */
template <value_type n_max>
class Formula {
    const value_type modulo;
    const Product<n_max> product;
  public:
    constexpr Formula(value_type imodulo, value_type itotient)
      : modulo(imodulo), product(imodulo, itotient) {}

    constexpr value_type operator()(value_type n, value_type m) const {
      if (n < 1) {
        return 0;
      }
      value_type value = product(m);
      for (auto i = value_type(0); i < n - m; i++) {
        value = (uint64_t(value) * uint64_t(value)) % modulo;
      }
      return value;
    }
};

template <value_type n_max>
class InputParser {
    const IsPrime<n_max> is_prime;

    value_type parse_int(const char*& ptr) const {
      char * e_ptr = nullptr;
      auto value = strtol(ptr, &e_ptr, 10);
      if (e_ptr == ptr) {
        throw std::runtime_error("Could not parse integer from input!");
      }
      ptr = e_ptr;
      return value;
    }

  public:
    constexpr InputParser() : is_prime() {}
    std::vector<value_type> operator()() const {

      // read stdin
      std::cin.seekg(0, std::ios_base::end);
      std::size_t len = std::cin.tellg();
      std::cin.seekg(0, std::ios_base::beg);
      std::string chars(len, ' ');
      std::cin.read(static_cast<char *>(&chars[0]), len);

      // get start & end
      auto ptr = chars.data();
      const auto end = chars.data() + len;

      // check number of tests
      while (!isdigit(*ptr) && ptr != end) { ++ptr; }
      std::vector<value_type> out(2 * parse_int(ptr), 0);

      // stored value counter
      auto i = value_type(0);

      // parse cases
      while (ptr != end) {

        // skip until next digit
        while (!isdigit(*ptr)) {
          if (ptr == end) {
            goto parse_stdin_end;
          }
          ++ptr;
        }

        // get the number of elements & setup count
        auto numel = parse_int(ptr);
        auto counter = value_type(0);

        // count primes
        for (auto i = value_type(0); i < numel; i++) {
          while (!isdigit(*ptr)) {
            if (ptr == end) {
              throw std::runtime_error("Unexpected end of input!");
            }
            ++ptr;
          }
          counter += is_prime(parse_int(ptr));
        }
        // push result
        out[i++] = numel;
        out[i++] = counter;
      }
      parse_stdin_end:
      return out;
    }
};

int main(int argc, char** argv) {

  /** Constants:
   *    NMAX - maximum number of input integers
   *    MMAX - maximum value of an input integer
   *    MODULO - 10 ^ 9 + 7
   *    TOTIENT - is equal n - 1, for any prime n
   */ 
  constexpr auto NMAX = value_type(1000);
  constexpr auto MMAX = value_type(1000000);
  constexpr auto MODULO = value_type(1000000007);
  constexpr auto TOTIENT = value_type(1000000006);

  static_assert(is_prime(MODULO), "MODULO is not prime!");
  static_assert((MODULO & ~uint32_t(1 << 31)) == MODULO, 
                "MODULO not 31 bit!");

  constexpr Formula<NMAX> formula(MODULO, TOTIENT);
  constexpr InputParser<MMAX> parse_stdin;

  try {
    auto input = parse_stdin();
    for (size_t i = 0; i < input.size(); i += 2) {
      std::cout << formula(input[i], input[i + 1]) << "\n";
    }
  } catch(std::exception& exception) {
    std::cout << exception.what() << "\n";
  }
  return 0;
}