/** This code will work for 31 bit representable prime moduli. 
 *  and must use unsigned data types.
 *
 *  in general, for large moduli, need to use safe modulo 
 *  multiplication here. see: https://stackoverflow.com/a/21901761
 */

#include <cstdint>
#include <iostream>
#include <type_traits>

using value_type = uint32_t;
static_assert(std::is_unsigned<value_type>(), "Value type is is signed!");

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

template <value_type n_max>
class InvFactorial : public Lookup<n_max> {
  public:
    constexpr InvFactorial(value_type modulo) : Lookup<n_max>() {
      for (auto i = value_type(0); i <= n_max; i++) {
        auto result = value_type(1);
        auto value = value_type(1);
        for (auto j = value_type(1); j <= i; j++) {
          value = (uint64_t(value) * uint64_t(j)) % modulo;
        }
        value_type exponent = modulo - 2;
        while (exponent) {
          if (exponent % 2) {
            result = (uint64_t(value) * uint64_t(result)) % modulo;
          }
          value = (uint64_t(value) * uint64_t(value)) % modulo;
          exponent /= 2;
        }
        Lookup<n_max>::lookup[i] = result;
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

constexpr value_type NMAX = value_type(100);
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

/** This needs a lot of RAM to compile for large NMAX
 */
static constexpr Binomial<NMAX> binomial(MODULO);

int main(int argc, char** argv) {
  for (value_type i = 0; i <= (NMAX + 5); i++) {
    std::cout << binomial(i, 2) << ", ";
  }
  std::cout << "\n";
  return 0;
}