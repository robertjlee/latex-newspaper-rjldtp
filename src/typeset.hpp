/*
 * Typeset: build up a page from the given layout
 */

#ifndef TYPESET_HPP
#define TYPESET_HPP

#include "data.hpp"
#include <memory>

namespace typeset {


  class setter{
  private:
    class impl;
    std::unique_ptr<impl> pImpl_;
  public:
    setter();
    ~setter();
    void operator()(const Page &p, const std::list<::layout::articlePlacement> & placements);
  }; 

}; // namespace typeset

#endif // ndef TYPESET_HPP
