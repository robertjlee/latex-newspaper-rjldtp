/*
 * Templated definitions that need to be supplied in headers
 * so that the compiler knows to link the stubs.
 */

#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <vector>
#include <list>
#include <ostream>

// for debugging of vectors etc:
template <typename T>
std::ostream& operator<< (std::ostream& out, const std::vector<T>& v) {
  if ( !v.empty() ) {
    out << '[';
    for (auto x : v) out << x << ", ";
    out << "\b\b]" << std::endl;
  }
  return out;
}
template <typename T>
std::ostream& operator<< (std::ostream& out, const std::list<T>& v) {
  if ( !v.empty() ) {
    out << '[';
    for (auto x : v) out << x << ", ";
    out << "\b\b]" << std::endl;
  }
  return out;
}

class area;
std::ostream& operator<< (std::ostream& out, const area &a);


#endif //ndef DEBUG_HPP
