#include "typeset.hpp"
#include <fstream>
#include <set>
#include <utility> // for pair

namespace typeset {
  class setter::impl {
  public:
    void operator()(const Page &p, 
		    const std::list<::layout::articlePlacement> & placements);
  private:
    bool alleyAt(const std::list<::layout::articlePlacement> & placements,
		  const std::pair<double, double> & start,
		  const std::pair<double, double> & end) const;
  };

  setter::setter() {}
  setter::~setter() {}
  void setter::operator()(const Page &p, const std::list<::layout::articlePlacement> & placements) {
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
   const std::list<::layout::articlePlacement> & placements) {

  std::ofstream layfile(p.layfile(), std::ios_base::trunc);

  double maxWidth = 0;
  for (auto &p : placements) maxWidth = std::max(maxWidth, p.area_.x2());
  // first we output the text width and h-margin:
  double hmargin = p.width() - maxWidth;
  layfile << "\\setlength{\\textwidth}{"
	  << p.width() - hmargin
	  << "pt}\\setlength{\\hsize}{\\textwidth}"
	  << std::endl
	  << "\\setlength{\\hoffset}{"
	  << hmargin/2
	  << "pt}"
	  << std::endl;

  // next we want the text height and v-margin:
  double maxY=0;
  for (auto &p : placements) {
    double y = p.area_.y_ + p.area_.h_;
    maxY = y > maxY ? y : maxY;
  }
  double vmargin = p.height() - maxY;
  layfile << "\\setlength{\\textheight}{"
	  << p.height() - vmargin
	  << "pt}\\setlength{\\vsize}{\\textheight}"
	  << std::endl
	  << "\\setlength{\\voffset}{"
	  << vmargin/2
	  << "pt}"
	  << std::endl;

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
  for (auto hi = h.begin(); hi != h.end(); ++hi) {
    for (auto vi = v.begin(); vi != vend; ++vi) {
      std::pair<double, double> hv(*hi, *vi);
      if (c.count(hv) !=0 ) { // 2.2 filter condition
	auto vii = vi;
	for (++vii; vii != v.end(); ++vii) {
	  std::pair<double, double> hvNext(*hi, *vii);
	  if (c.count(hvNext) !=0 ) // 2.2 filter condition
	    if (alleyAt(placements, hv, hvNext)) {
	      layfile << "\\valley{" << hv.first << "pt}{" << hvNext.second 
		      << "pt}{" << (hvNext.second - hv.second) << "pt}"
		      << std::endl;
	      vi = vii; --vi; break; // disallow overlaps
	    }
	}
      }
    }
  }
  for (auto vi = v.begin(); vi != v.end(); ++vi)
    for (auto hi = h.begin(); hi != hend; ++hi) {
      std::pair<double, double> vh(*hi, *vi);
      if (c.count(vh) !=0 ) { // 2.2 filter condition
	auto hii = hi;
	for (++hii; hii != h.end(); ++hii) {
	  std::pair<double, double> vhNext(*hii, *vi);
	  if (c.count(vhNext) != 0)
	    if (alleyAt(placements, vh, vhNext)) {
	      layfile << "\\halley{" << vh.second << "pt}{" << vh.first
		      << "pt}{" << (vhNext.first - vh.first) << "pt}" << std::endl;
	      hi = hii; --hi; break; // disallow overlaps
	    }
	}
      }
    }

  for (auto &p : placements) {
    layfile << "\\doarticle{" << p.art_.id() << "}{"
	    << p.area_.x_ << "pt}{" << p.area_.y_ << "pt}{"
	    << p.area_.w_ << "pt}{" << p.area_.h_ << "pt}{"
	    << p.opt_.numCols() << '}'
	    << std::endl;
  }
}

/*
 * Does (start--end) represent a alley?
 * - if so, there will be one article where both start and end are corners
 */
bool typeset::setter::impl::alleyAt
(const std::list<::layout::articlePlacement> & placements,
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

