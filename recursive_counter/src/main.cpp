#include <string>       // std::string
#include <vector>       // std::vector
#include <chrono>       // std::chrono
#include <cstdint>      // int64_t
#include <cstdlib>      // std::strtoll
#include <iostream>     // std::cout
#include <algorithm>    // std::swap
#include <exception>    // std exception
#include <stdexcept>    // std::runtime_error

/** Wrapper around std::chrono
 *
 */
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

/** Basic positive int parser
 *
 */
inline size_t parse_positive_int(const char * argv) {
  using namespace std;
  char * e_ptr;
  int64_t ans = strtoll(argv, &e_ptr, 10);
  if(e_ptr == argv) {
    auto msg = string("Could not parse one of the values.");
    throw(runtime_error(msg));
  }
  if(ans < 0LL) {
    auto msg = string("All values must be positive");
    throw(runtime_error(msg));
  }
  return static_cast<size_t>(ans);
}

/** Basic argument parser
 *
 */
std::vector<size_t> parse_args(int argc, char const * argv[]) {
  if(argc < 2) {
    auto msg = std::string("Please provide at least one positive integer.");
    throw(std::runtime_error(msg));
  }
  std::vector<size_t> ans(argc - 1ULL, 0ULL);
  int i = 1;
  for(auto & val : ans) {
    val = parse_positive_int(argv[i++]);
  }
  return ans;
}

/** Class responsible for the actual computation
 *
 */
template<size_t max_num_counts>
class Counter {
  //private:

    /** Doesn't want to have a zero value in template parameter
     *
     */
    static_assert(max_num_counts > 0ULL, "max_num_counts must be above 0!!!");

    /** High template parameter values (e.g. 1024ULL) cause a segfault.
     *  Would need to move away from static arrays to fix this.
     */
    static_assert(max_num_counts <= 64ULL, "max_num_counts set too high!!!");
    
    /** Maximum number of entries in the upper triangle of the matrix.
     *  
     */
    static constexpr size_t max_num_triu = max_num_counts 
                                            * (max_num_counts + 1ULL) / 2ULL;

    /** Arrays for keeping track of the matrix and the set of possible
     *  entry values.
     */
    size_t cfg[max_num_triu + max_num_counts + 1ULL];
    size_t counts[max_num_triu + max_num_counts + 1ULL];
    size_t ccounts[max_num_triu + max_num_counts + 1ULL];

    /** Shorthand for pair minimum.
     *  
     */
    static constexpr size_t min(size_t a, size_t b) { 
      return a < b ? a : b; 
    }

    /** Number of distinct tuples (A, B, C), such that 
     *  0 <= A <= a, 0 <= B <= b, 0 <= C <= min(A, B) 
     *  and A, B, C \in \mathbb{Z}.
     *  In other words, the sum:
     *  $\sum_{i = 0}^m\sum_{j = 0}^n(\min\{i, j\} + 1)$
     */
    static constexpr size_t num_unique_triples(size_t a, size_t b) {
      if(a > b) {
        std::swap(a, b);
      }
      return a * (a + 1ULL) * (3ULL * b - a + 1ULL) / 6ULL 
              + (a + 1ULL) * (b + 1ULL);
    }

    /** Recursive routine running the calculation.
     *  
     */
    size_t num_unique_cfg(
      size_t i, 
      size_t n, 
      size_t * xptr, 
      size_t * cptr,
      size_t * ccptr
    ) noexcept {

      size_t sum = 0ULL;
      if(n > 1ULL) {
        if(i < n) {
          size_t xmin = *(ccptr + i);
          size_t xmax = *(cptr + i) + 1ULL;
          for(size_t j = xmin; j < xmax; j++) {
            *(xptr + i) = j;
            sum += num_unique_cfg(i + 1ULL, n, xptr, cptr, ccptr);
          }
          return sum;
        } else {
          size_t m = n--;
          ccptr += m;
          cptr += m;
          for(
            size_t *x = xptr, *y = xptr - m, *cc = ccptr, j = 0ULL;
            j < n; 
            x++, y++, cc++, j++
          ) {
            size_t xsum = *(x) + *(x + 1ULL); 
            *(cc) = xsum > *(y) ? (xsum - *(y)) : 0ULL;
          }
          for(size_t *x = xptr, *c = cptr, j = 0ULL; j < n; x++, c++, j++) {
            *(c) = min(*(x), *(x + 1ULL));
          }
          return num_unique_cfg(0ULL, n, xptr + m, cptr, ccptr);
        }
      } 
      else if(n == 1ULL) {
        return (*(cptr) + 1ULL) - *(ccptr);
      }
      else {
        return 1ULL;
      }
    }

  public:
    Counter() {
      for(auto & val: cfg) {
        val = 0ULL;
      }
      for(auto & val: counts) {
        val = 0ULL;
      }
      for(auto & val: ccounts) {
        val = 0ULL;
      }
    }

    /** Entry point for the calculation.
     *  
     */
    size_t operator()(const std::vector<size_t> & values) {
      static constexpr size_t pad = max_num_counts + 1ULL;
      const size_t num_counts = values.size();
      if(num_counts > max_num_counts) {
        throw(std::runtime_error(
          std::string("Unexpected number of counts.\n")
          + "Please increase num_max_counts template parameter."));
      }
      if(!num_counts) {
        throw(std::runtime_error("Number of counts must be greater than 0."));
      }
      for(size_t i = 0ULL; i < num_counts; i++) {
        counts[i + pad] = values[i];
        cfg[i + pad] = values[i];
      }
      for(size_t i = num_counts, j = 0ULL; j < num_counts - 1ULL; i++, j++) {
        counts[i + pad] = min(counts[j + pad], counts[j + 1ULL + pad]);
      }
      return num_unique_cfg(0ULL, 
                            num_counts - 1ULL, 
                            &cfg[num_counts + pad], 
                            &counts[num_counts + pad],
                            &ccounts[num_counts + pad]);
    }
};

/** Main routine
 *
 */
int main(int argc, char const * argv[]) {
  Counter<64ULL> counter;
  Timer<double> timer;
  try {
    std::vector<size_t> values = parse_args(argc, argv);
    std::cout << counter(values) << "\n";
    std::cout << timer.toc() << " seconds elapsed \n";
  } catch(std::exception & except) {
    std::cout << except.what() << "\n";
  }
  return 0;
}