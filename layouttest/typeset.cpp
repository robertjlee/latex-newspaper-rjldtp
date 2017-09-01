#include "typeset.hpp"
#include <iostream>
#include <set>
#include <utility> // for pair

namespace typeset {
  class setter::impl {
  public:
    void operator()(const Page &p, 
		    const std::vector<::layout::articlePlacement> & placements);
  private:
    bool alleyAt(const std::vector<::layout::articlePlacement> & placements,
		  const std::pair<double, double> & start,
		  const std::pair<double, double> & end) const;
  };

  setter::setter() {}
  setter::~setter() {}
  void setter::operator()(const Page &p, const std::vector<::layout::articlePlacement> & placements) {
    (*pImpl_)(p, placements);
  }


};

/*
 * Given the articles, we want to work out where the alleys are to go.
 * Don't output the article dimensions; TeX will already know those.
 * Instead, we output alleys before or after each article.
 */
void typeset::setter::impl::operator()
  (const Page &p, 
   const std::vector<::layout::articlePlacement> & placements) {

  /*
   * 1) Extract: the position of all corners (C) ; and a sorted set of the
   * all horizontal (H) and vertical (V) positions of alleys.
   * 2) For each H
   * 2.1) For each V, and each V1 from V+1 to end
   * 2.2) If (H,V) and (H,V1) are in (C)
   * 2.3) Check to see if an article starts or ends on (H,V) -- (H,V1)
   * 2.3.1) - if yes, output a alley on that line.
   * 2.3.2) - if no, then  (H,V) -- (H,V1) is space used by an article. Skip.
   * 3) Repeat step 2 with H and V swapped for vertical alleys
   * 4) Output each article.
   */
  std::set<std::pair<double, double> > c;
  std::set<double> h, v;
  for (auto &p : placements) {
    auto x1 = p.area_.x_, y1 = p.area_.y_;
    auto x2 = x1 + p.area_.w_, y2 = y1 + p.area_.h_;
    c.emplace(x1, y1);
    c.emplace(x1, y2);
    c.emplace(x2, y1);
    c.emplace(x2, y2);
    h.emplace(x1);
    h.emplace(x2);
    v.emplace(y1);
    v.emplace(y2);
  }
  auto hend = h.end(); --hend;
  auto vend = v.end(); --vend;
  for (auto hi = h.begin(); hi != hend; ++hi) {
    for (auto vi = v.begin(); vi != vend; ++vi) {
      std::pair<double, double> hv(*hi, *vi);
      if (c.count(hv) !=0 ) { // 2.2 filter condition
	auto vii = vi;
	for (++vii; vii != v.end(); ++vii) {
	  std::pair<double, double> hvNext(*hi, *vii);
	  //	  std::cout << "Alleyable " << hv.first << ',' << hv.second 
	  //		    << "--" << hvNext.first << ',' << hvNext.second << std::endl;
	  if (c.count(hvNext) !=0 ) // 2.2 filter condition
	    if (alleyAt(placements, hv, hvNext)) {
	      std::cout << "\\valley{" << hv.first << "}{" << hv.second 
			<< "}{" << hvNext.second << '}' << std::endl;
	      vi = vii; --vi; break; // disallow overlaps
	    }
	}
      }
    }
  }
  for (auto vi = v.begin(); vi != vend; ++vi)
    for (auto hi = h.begin(); hi != hend; ++hi) {
      std::pair<double, double> vh(*hi, *vi);
      if (c.count(vh) !=0 ) { // 2.2 filter condition
	auto hii = hi;
	for (++hii; hii != h.end(); ++hii) {
	  std::pair<double, double> vhNext(*hii, *vi);
	  if (c.count(vhNext) != 0)
	    if (alleyAt(placements, vh, vhNext)) {
	      std::cout << "\\halley{" << vh.second << "}{" << vh.first
			<< "}{" << vhNext.first << '}' << std::endl;
	      hi = hii; --hi; break; // disallow overlaps
	    }
	}
      }
    }

  for (auto &p : placements) {
    std::cout << "\\article{" << p.art_.id() << "}{"
	      << p.area_.x_ << "}{" << p.area_.y_ << "}{"
	      << p.area_.w_ << "}{" << p.area_.h_ << "}"
	      << std::endl;
  }
}

/*
 * Does (start--end) represent a alley?
 * - if so, there will be one article where both start and end are corners
 */
bool typeset::setter::impl::alleyAt
(const std::vector<::layout::articlePlacement> & placements,
 const std::pair<double, double> & start,
 const std::pair<double, double> & end) const {
  for (auto &p : placements) {
    auto x1 = p.area_.x_, y1 = p.area_.y_;
    auto x2 = x1 + p.area_.w_, y2 = y1 + p.area_.h_;
    
    std::set<std::pair<double, double> > c;
    c.emplace(x1, y1);
    c.emplace(x1, y2);
    c.emplace(x2, y1);
    c.emplace(x2, y2);
    if (c.count(start) + c.count(end) == 2)
      return true;
  }
  return false;
}

