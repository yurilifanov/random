/** Check out: 
 *    - https://en.wikipedia.org/wiki/Euler%27s_theorem
 *    - https://en.wikipedia.org/wiki/Lucas%27s_theorem#Consequence
 *    - https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Existence_(constructive_proof)
 */

#include <string>         // std::string
#include <cctype>         // isdigit
#include <vector>         // std::vector
#include <cstdlib>        // strtoul
#include <cstdint>        // uint32_t
#include <iostream>       // std::cout
#include <exception>      // std::runtime_error
#include <type_traits>    // std::is_unsigned, std::is_integral

using value_type = uint32_t;
static_assert(std::is_unsigned<value_type>(), "Value type is signed!");

/** Simple check for whether input integer x is a prime.
 */ 
template <typename type>
constexpr bool IS_PRIME(type x) {

  static_assert(std::is_unsigned<type>() & std::is_integral<type>(), 
                "is_prime supports only unsigned integral types.");

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

/** From Lucas's theorem:
 *    A binomial coefficient (n choose k) is divisible by a prime p
 *    iif at least one digit of base p representation of k is greater
 *    than the corresponding digit of base p representation of n.
 *
 *    For p = 2, (n choose k) is even iif at least one digit of binary 
 *    representation of k is 1 and the corresponding digit of binary
 *    representation of n is 0.
 */
template <typename type>
constexpr type binom_mod2(type n, type k) {

  static_assert(std::is_unsigned<type>() & std::is_integral<type>(), 
                "binom_mod2 supports only unsigned integral types.");

  // among the digits which are different in n & k,
  // check if any digits of k are one
  return !(k & (n ^ k));

}

template <typename type>
constexpr type mod_mul(uint64_t left, uint64_t right, type modulo) {

  static_assert(std::is_unsigned<type>() & std::is_integral<type>(), 
                "mod_mul supports only unsigned integral types.");

  return (left * right) % modulo;

}

/** Modular exponentiation via squaring.
 */ 
template <typename type>
constexpr type mod_exp(uint64_t value, uint64_t exponent, type modulo) {

  static_assert(std::is_unsigned<type>() & std::is_integral<type>(), 
                "mod_exp supports only unsigned integral types.");

  auto result = type(1);

  while (exponent) {
    if (exponent % 2) {
      result = mod_mul(value, result, modulo);
    }
    value = mod_mul(value, value, modulo);
    exponent /= 2;
  }

  return result;

}

template <typename type>
constexpr type mod_inv(uint64_t value, type modulo) {

  static_assert(std::is_unsigned<type>() & std::is_integral<type>(), 
                "mod_inv supports only unsigned integral types.");

  return mod_exp(value, modulo - 2, modulo);

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
        Lookup<bool, n_max>::lookup[i] = IS_PRIME(i);
      }
    }
};

/** Lookup for the values 
 *  A_n = \prod_{k = 0} ^ n (k + 2) ^ {n \choose k}
 *  modulo some integer.
 *
 *  Uses Euler's theorem to get the (n choose k) power of (k + 2).
 */ 
template <value_type n_max>
class Product : public Lookup<value_type, n_max> {
    static constexpr value_type modulo = 1000000007;

    static_assert(IS_PRIME(modulo), "MODULO is not prime!");

    static_assert((modulo & ~uint32_t(1 << 31)) == modulo, 
                  "MODULO not 31 bit!");

  public:
    constexpr Product() : Lookup<value_type, n_max>() {

      value_type t = 1000 * 1000 * 1000 + 6;
      value_type p = t / 2;

      value_type factorial[n_max + 1] = {1, 1};
      value_type inv_factorial[n_max + 1] = {1, 1};

      value_type factorial_val = 1;

      for (value_type i = 2; i <= n_max; i++) {
        factorial_val = mod_mul(factorial_val, i, p);
        factorial[i] = factorial_val;
        inv_factorial[i] = mod_inv(factorial_val, p);
      }

      Lookup<value_type, n_max>::lookup[0] = value_type(2);
      for (auto i = value_type(1); i <= n_max; i++) {
        auto value = value_type(2);
        for (auto j = value_type(1); j <= i; j++) {

          auto bnm_p = mod_mul(factorial[i], inv_factorial[j], p);
          bnm_p = mod_mul(bnm_p, inv_factorial[i - j], p);

          auto bnm_2 = binom_mod2(i, j);

          auto offset = t - mod_mul(500000002, bnm_p, t);

          auto bnm = (offset + bnm_2 * 500000003) % t;

          value = mod_mul(value, mod_exp(2 + j, bnm, modulo), modulo);

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
    /* constexpr */ Formula(value_type imodulo)
      : modulo(imodulo), product() {}

    constexpr value_type operator()(value_type n, value_type m) const {
      if (n < 1) {
        return 0;
      }
      value_type value = product(m);
      for (auto i = value_type(0); i < n - m; i++) {
        value = mod_mul(value, value, modulo);
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
    /* constexpr */ InputParser() : is_prime() {}
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
  constexpr auto MMAX = value_type(1000 * 1000);
  constexpr auto MODULO = value_type(1000 * 1000 * 1000 + 7);

  /* constexpr */ Formula<NMAX> formula(MODULO);
  /* constexpr */ InputParser<MMAX> parse_stdin;

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