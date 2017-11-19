#include <cstdlib>
#include <string>
#include <iostream>
#include <tuple>
#include <utility> 
#include <cstdint>
#include <fstream>
#include <chrono>
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
/*
    To do:
      - StringView parse
      - parsing a multi line string - file line parser
      - performance comparison with template free approach
*/
template<char sep, char ldel, char term>
class StrFieldView {
    const char * s_ptr;
    const char * e_ptr;
  public:
    StrFieldView() {}
    StrFieldView(const char * st, const char *& ed) {
      s_ptr = st;
      while((*st) != sep && (*st) != ldel && (*st) != term) {
        ++st;
      }
      e_ptr = st;
      ed = st;
    }
    friend std::ostream& operator<<(std::ostream &os, const StrFieldView& sfv) {
      os.write(sfv.s_ptr, sfv.e_ptr - sfv.s_ptr);
      return os;
    }
};
using StringView = StrFieldView<',', '\n', '\0'>;

template<typename type>
constexpr bool is_one_of() {
  return false;
}
template<typename type, typename first, typename... rest>
constexpr bool is_one_of() {
  return std::is_same<type, first>::value || is_one_of<type, rest...>();
}
template<typename Type> 
static inline typename std::enable_if< 
  is_one_of <Type , int 
                  , double 
                  , int64_t 
                  , uint64_t
                  , StringView
            >(), Type>::type 
str2type(const char * s_ptr, char ** e_ptr) { 
  std::cout << "Boo!\n";
  return Type(0); 
}
template<> 
inline int str2type<int>(const char * s_ptr, char ** e_ptr) {
  return strtol(s_ptr, e_ptr, 10);
}
template<> 
inline int64_t str2type<int64_t>(const char * s_ptr, char ** e_ptr) {
  return strtoll(s_ptr, e_ptr, 10);
}
template<> 
inline uint64_t str2type<uint64_t>(const char * s_ptr, char ** e_ptr) {
  return strtoull(s_ptr, e_ptr, 10);
}
template<> 
inline double str2type<double>(const char * s_ptr, char ** e_ptr) {
  return strtod(s_ptr, e_ptr);
}
template<>
inline StringView str2type<StringView>(const char * s_ptr, char ** e_ptr) {
  return StringView(s_ptr, const_cast<const char *&>(*e_ptr));
}

template<char sep, char ldel, char term, typename... Args>
class TupleParser {
    std::tuple<Args...> internal;
    template<std::size_t i = std::size_t(0)> constexpr 
    typename std::enable_if<(i == sizeof...(Args) - 1), bool>::type 
    parse(const char *& ptr) {
      using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
      char * e_ptr = nullptr;
      std::get<i>(internal) = str2type<type>(ptr, &e_ptr);
      if(e_ptr == ptr) {
        return false;
      }
      ptr = const_cast<const char *>(e_ptr);
      return true;
    }
    template<std::size_t i = std::size_t(0)> constexpr 
    typename std::enable_if<(i < sizeof...(Args) - 1), bool>::type 
    parse(const char *& ptr) {
      using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
      char * e_ptr = nullptr;
      std::get<i>(internal) = str2type<type>(ptr, &e_ptr);
      if(e_ptr == ptr) {
        return false;
      }
      ptr = const_cast<const char *>(e_ptr);
      while(*ptr != sep) {
        if(*ptr== ldel || *ptr == term) {
          return false;
        }
        ++ptr;
      }
      ++ptr;
      return parse<i + 1>(ptr);
    }
  public:
    bool parse(const char *& ptr) { return parse<>(ptr); }
    template<std::size_t i> constexpr
    typename std::tuple_element<i, std::tuple<Args...>>::type get() const {
      return std::get<i>(internal);  
    }
    bool nextl(const char *& ptr) const {
      while((*ptr) != ldel) {
        if((*ptr) == term) {
          return false;
        }
        ++ptr;
      }
      ++ptr;
      return true;
    }
};
using LineParser = TupleParser<',', '\n', '\0', StringView, int64_t, double>;

class FileBuffer {
    const std::size_t   bf_size;
    char * const        ptr;
  public:
    FileBuffer(std::size_t size)
      : bf_size(size)
      , ptr(new char[bf_size])
    {}
   ~FileBuffer() { delete [] ptr; }
   void load_file(const char * f_name) {
     std::ifstream is(f_name, std::ios::binary);
     is.seekg(0, std::ios::end);
     std::size_t len = is.tellg();
     is.seekg(0, std::ios::beg);
     is.read(ptr, len > bf_size ? bf_size:len);
     is.close();
   }
    void load_file(std::string & f_name) {
      load_file(f_name.c_str());
    }
    const char * get_ptr() const { return ptr; } 
};

void test(FileBuffer &fb, LineParser& lp) {
  using namespace std;
  fb.load_file("test.csv");
  const char *ptr = fb.get_ptr();
  while(true) {
    if(lp.parse(ptr)) {
    }
    if(!lp.nextl(ptr)) {
      break;
    }
  }
}

int main() {
  using namespace std;
  int64_t n_tests = 1000LL;
  FileBuffer fb(1048576ULL);
  LineParser lp;
  Timer<double> timer;
  for(int64_t i = 0LL; i < n_tests; i++) {
    test(fb, lp);
  }
  cout << n_tests << " file reads and parses in: " << timer.toc() << "s\n";
}