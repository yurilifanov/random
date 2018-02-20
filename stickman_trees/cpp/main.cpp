#include <string>         // std::string
#include <cctype>         // isdigit
#include <chrono>         // std::chrono::high_resolution_clock
#include <cstdlib>        // strtoul
#include <cstdint>        // uint32_t
#include <iostream>       // std::cout
#include <exception>      // std::runtime_error
#include <algorithm>      // std::sort
#include <functional>     // std::greater
#include <type_traits>    // std::is_unsigned

using value_type = uint64_t;
static_assert(std::is_unsigned<value_type>(), "Value type is signed!");

template<typename Type> 
class Timer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    TimePoint tick_point;
  public:
    Timer() { tick_point = Clock::now(); }
    void tic() { tick_point = Clock::now(); }
    Type toc() { 
      using namespace std::chrono;
      return duration_cast<duration<Type>>(Clock::now() - tick_point).count(); 
    }
};

/** Totally safe function, nothing to see here.
 *
 */
auto prepare_input(value_type *arr_start, value_type *arr_end) {

  // get stream length
  std::cin.seekg(0, std::ios_base::end);
  std::size_t len = std::cin.tellg();
  std::cin.seekg(0, std::ios_base::beg);

  // allocate for reading & reserve for output
  std::string chars(len, ' ');

  // read
  std::cin.read(static_cast<char *>(&chars[0]), len);

  // parse
  auto ptr = chars.data();
  const auto end = chars.data() + len;

  // skip first digit, can infer number of nodes from length of sequence
  while (!isdigit(*ptr)) {
    ++ptr;
  }

  // try parsing to uint32_t
  char * e_ptr = nullptr;
  size_t num_vals = strtoul(ptr, &e_ptr, 10);

  // check parse was successful
  if (e_ptr == ptr) {
    throw std::runtime_error("Error: Could not parse integer from input!");
  }
  ptr = e_ptr;

  auto arr_ptr = arr_start;

  while (arr_ptr - arr_start < num_vals) {

    // skip until next digit
    while (!isdigit(*ptr)) {
      if (ptr == end) {
        goto parse_stdin_end;
      }
      ++ptr;
    }

    // try parsing to uint32_t
    char * e_ptr = nullptr;
    auto value = strtoul(ptr, &e_ptr, 10);

    // check parse was successful
    if (e_ptr == ptr) {
      throw std::runtime_error("Error: Could not parse integer from input!");
    }
    ptr = e_ptr;

    // store, value guaranteed to be in [1, 99999]
    *(arr_ptr++) = value;

  }
  parse_stdin_end:

  // sort input degree sequence in place in descending order
  std::sort(arr_start, arr_end, std::greater<value_type>());

  return num_vals;
}

inline constexpr value_type mod_mul(
  value_type left,
  value_type right,
  value_type modulo
) {
  return (uint64_t(left) * uint64_t(right)) % modulo;
}

constexpr value_type mod_exp(
  value_type value, 
  value_type exponent, 
  value_type modulo
) {
  auto result = value_type(1);
  while (exponent) {
    if (exponent % 2) {
      result = mod_mul(value, result, modulo);
    }
    value = mod_mul(value, value, modulo);
    exponent /= 2;
  }
  return result;
}

inline constexpr value_type mod_inv(value_type value, value_type modulo) {
  return mod_exp(value, modulo - 2, modulo);
}

int main(int argc, char** argv) {
  try {
    constexpr size_t N_MAX = 100000;
    constexpr value_type MODULO = 1000 * 1000 * 1000 + 7;
    constexpr value_type inv12 = mod_inv(12, MODULO);

    value_type array[N_MAX] = {};

    auto n = prepare_input(&array[0], &array[N_MAX - 1]);

    // at least 7 nodes are required for non zero result
    if (n < 7) {
      std::cout << 0 << "\n";
      return 0;
    }

    const value_type denom = mod_inv(n - 2, MODULO);
    uint64_t sum = 0;

    // Timer<double> timer;
    for (value_type *i = &array[0]; *i > 3; i++) {

      const uint64_t DI = *i;
      const uint64_t di = *i - 1;
      const uint64_t didim1 = di * (di - 1);

      uint64_t agg = 0;
      uint64_t ival = ((4 * didim1 * didim1) % MODULO) * (didim1 - di);

      auto j = i + 1;

      while (*j == DI) {
        j++;
      }

      sum = (sum + mod_mul(ival, j - i - 1, MODULO)) % MODULO;

      for (; *j > 2; j++) {
        const uint64_t dj = *j - 1;
        const uint64_t djdjm1 = dj * (dj - 1);
        const auto d = di + dj;
        uint64_t jval = ((d - 4) * d) % MODULO;
        jval *= djdjm1;
        agg = (agg + jval) % MODULO;
      }

      sum = (sum + agg * didim1) % MODULO;

    }

    std::cout << mod_mul(mod_mul(sum, inv12, MODULO), denom, MODULO) << "\n";
    // std::cout << timer.toc() << "\n";


  } catch(std::exception& exception) {
    std::cout << exception.what() << "\n";
  }
  return 0;
}