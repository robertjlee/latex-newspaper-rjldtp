/*
 * Template for worst-fit layout algorithm.
 */

#ifndef LAYOUT_WORST_HPP
#define LAYOUT_WORST_HPP

#include "data.hpp"
#include "debug.hpp"
#include <vector>
#include <list>
#include <memory>



namespace layout {
  /*
   * Simple 2-way tagged union (nullable, holds copy of passed value)
   */
template<class T, class U>
class either {
private:
  int type_;
  void* ptr_;
public:
  either() : type_(0), ptr_(0) {};
  either(T & t) : type_(1), ptr_(new T(t)) {};
  either(U & u) : type_(2), ptr_(new U(u)) {};
  ~either() {
    if (isT()) delete static_cast<T*>(ptr_);
    if (isU()) delete static_cast<U*>(ptr_);
  }
  int type() const { return type_; }
  bool isNull() const { return type_ == 0; }
  bool isT() const { return type_ == 1; }
  bool isU() const { return type_ == 2; }
  template <typename C>
  C& cast() { return static_cast<C>(*ptr_); }
};

typedef either<articlePlacement,area> space;

struct splitSpace {
  bool vert_, split_;
  std::unique_ptr<splitSpace> before_;
  std::unique_ptr<splitSpace> after_;
  splitSpace(splitSpace & s) : // copy and transfer ownership
    vert_(s.vert_),
    split_(s.split_),
    before_(s.before_.get()),
    after_(s.after_.get()) {}
  splitSpace(Page p) {
    area a(p.width(), p.height(), 0, 0);
    space s(a);
    before_ = std::unique_ptr<splitSpace>(new splitSpace(s));
  }
  splitSpace(space & rhs) {
    before_= std::unique_ptr<splitSpace>(new splitSpace(rhs));
  }
  void vertSplit(space & rhs) {
    split_ = true; vert_ = true;
    after_ = std::unique_ptr<splitSpace>(new splitSpace(rhs));
  }
  void horizSplit(space & rhs) {
    split_ = true; vert_ = false;
    after_ = std::unique_ptr<splitSpace>(new splitSpace(rhs));
  }
  splitSpace & operator=(splitSpace & s) {
    vert_ = s.vert_;
    split_ = s.split_;
    before_.reset(s.before_.get());
    after_.reset(s.after_.get());
    return (*this);
  }
};


/*
 * algorithm to lay out a page by worst fit.
 *
 * This will work best if the largest article is presented first.
 * This requires that article options are sorted by size ascending, and the
 * index to the largest allowed article option is provided.
 *
 * A rectangle of free space is set at the size of the page.
 * Repeatedly, each artcle (starting with the first) is placed into the 
 * largest rectangle.
 * The remaining free space is split into 2 (unless the article fits exactly
 * into the width/height of one block).
 *
 * widthFirst : true to split free space width-wise before height-wise
 */
template <bool widthFirst = true>
class worstFit {
private: 
  std::vector<articlePlacement> result_;
public:
  const std::vector<articlePlacement> & 
  operator()(const Page & p, const std::vector<int> & preferredArticles) {
    layoutRecurse(p, preferredArticles, preferredArticles.size()-1);
    return result_;
  }
private:
  /*
   * As the public method, but uses the "start" parameter to recurse through all combinations.
   * NB: the first option (generally the largest article) is tried for alternative sizes first in order to
   * get the fastest option.
   */
  void layoutRecurse(const Page & p, const std::vector<int> & options, int start) {
    if (start == -1) {
      //std::cout << "TRYING " << options;
      layout(p, options); // may throw
      return;
    }
    bool success = false;
    for (int j = options[start]; j >= 0; --j) {
      std::vector<int> opt = options;
      opt[start] = j;
      try {
	layoutRecurse(p, opt, start-1);
	success = true;
	break;
      } catch (const char* error) {
      }
    }
    if (!success) 
      throw "No layouts found with these article sizes.";    
  }

  /*
   * Perform the actual page layout.
   * We will attempt to use a worst-fit algorithm, dividing the page into <=4 areas for each
   * article placed.
   */
  void layout(const Page & p, const std::vector<int> & options) {
    std::vector<articlePlacement> result;
    area wholePage(p.width(), p.height(), 0, 0);
    std::list<area> areas(1, wholePage);

    int i=0;
    for (auto idx : options) {
      auto &art = p[i++];
      auto &opt = art[idx];
      auto artArea = opt.area();
      auto artWidth = opt.layoutWidth();
      auto artHeight = opt.layoutHeight();
      //std::cout  << "Article #" << art.id() << " option #" << idx 
      // << "; width = " << artWidth << ", height = " << artHeight << std::endl;

      // find the smallest area large enough to fit the article.
      double worstSpace = 0;
      area worstArea;
      std::list<area>::iterator worstAreaIter;
      for (auto a = areas.begin(); a != areas.end(); ++a) {
	// skip areas where we don't fit:
	if (a->w_ < artWidth) continue;
	if (a->h_ < artHeight) continue;
	// find the worst-fitting space left:
	double space = a->size() - artArea;
	if (space > worstSpace) {
	  worstSpace = space;
	  worstArea = area(artWidth, artHeight, a->x_, a->y_);
	  worstAreaIter = a;
	}
      }
      if (worstSpace == 0) {
	//std::cout << "Space remaining " << areas << std::endl;
	throw "No solution found. Backtracing needed. ";
      }
      // now we divide the area
      area toSplit = *worstAreaIter;
      //      std::cout << "toSplit = " << toSplit << std::endl;
      // we split the free space lengthways first, then widthways.
      // -----        -----
      // |A| |        |A| |
      // |---|   or   |-| |
      // |___|        |_|_|
      if (widthFirst) {
	if (dblGt(toSplit.w_, artWidth)) {
	  worstAreaIter = areas.insert(worstAreaIter,
				       area(toSplit.w_ - artWidth, toSplit.h_,
					    toSplit.x_ + artWidth, toSplit.y_)
				       );
	  ++worstAreaIter;
	}
	if (dblGt(toSplit.h_ , artHeight)) {
	  worstAreaIter = areas.insert(worstAreaIter, 
				       area(artWidth, toSplit.h_ - artHeight,
					    toSplit.x_, toSplit.y_ + artHeight)
				       );
	  ++worstAreaIter;
	}
      } else {
	if (dblGt(toSplit.w_ , artWidth)) {
	  worstAreaIter = areas.insert(worstAreaIter, 
				       area(toSplit.w_ - artWidth, artHeight,
					    toSplit.x_ + artWidth, toSplit.y_)
				       );
	  ++worstAreaIter;
	}
	if (dblGt(toSplit.h_ , artHeight)) {
	  worstAreaIter = areas.insert(worstAreaIter,
				       area(toSplit.w_, toSplit.h_ - artHeight,
					    toSplit.x_, toSplit.y_ + artHeight)
				       );
	  ++worstAreaIter;
	}
      }
      areas.erase(worstAreaIter);
      //std::cout << "Space remaining " << areas << std::endl;

      area res(artWidth, artHeight, worstArea.x_, worstArea.y_);
      /*std::cout << "Placing article #" << art.id() << " at " << res
	  << " (" << opt.numCols() << " columns)"
	  << std::endl
	  << " - space lost " << worstSpace << std::endl;*/
      result.emplace_back(res, art, opt);

    }
    result_.clear();
    for (auto &r : result) result_.emplace_back(r);
    std::cout << "Unfilled space is now " << areas << std::endl;
  }

};

} // namespace layout

#endif // ndef LAYOUT_WORST_HPP
