/*
 * Template for a delegating layout algorithm that
 * neatens the edges.
 */

#ifndef LAYOUT_TIDY_HPP
#define LAYOUT_TIDY_HPP

#include "data.hpp"

namespace layout {

/*
 * Class to decorate a layout by stretching the layouts at the edges to fit 
 * the space exactly. When columns set at fractionally different lengths, this
 * neatens up the edges.
 */
template <class T>
class stretchDecorator {
private:
  // raw pointer is safe here as we keep delegate_ alive and don't assume ownership
  std::list<articlePlacement> result_;
  T delegate_;
  const unsigned int LEFT  =              1;
  const unsigned int RIGHT = LEFT  << 1;//2
  const unsigned int ABOVE = RIGHT << 1;//4
  const unsigned int BELOW = ABOVE << 1;//8
public:
  stretchDecorator(T &delegate) :
    delegate_(delegate) {}
  const std::list<articlePlacement> &
  operator()(const Page & p, const std::vector<int> & preferredArticles) {
    auto res = delegate_(p,preferredArticles);
    for (auto &i : res)
      result_.emplace_back(i);
    fit(p);
    return result_;
  }
private:
  void fit(const Page & p) {
    double minX = p.width(), minY = p.height();
    double maxX = 0, maxY = 0;
    for (auto &placement : result_) {
      const area & a = placement.area_;
      minX = std::min(minX, a.x_);
      minY = std::min(minY, a.y_);
      maxX = std::max(maxX, a.x2());
      maxY = std::max(maxY, a.y2());
    }
    for (auto iter = result_.begin();
	 iter != result_.end();
	 ++iter) {
      area a = iter->area_;
      auto next = iter; next++;
      auto blocked = find(a, result_.begin(), iter) |
	find(a, next, result_.end());
      // if there is nothing on the {left,right,above,below} this article, stretch it.
      if (!(blocked & ABOVE))
	a.y_ = minY;
      if (!(blocked & BELOW))
	a.h_ = maxY - a.y_;
      if (!(blocked & LEFT))
	a.x_ = minX;
      if (!(blocked & RIGHT))
	a.w_ = maxX - a.x_;
      auto &art = iter->art_;
      auto &opt = iter->opt_;
      // we cannot resize fixed-size articles
      if (iter->art_.filename() != std::string("RASTER")) {
	iter = result_.erase(iter);
	iter = result_.emplace(iter, articlePlacement(a, art, opt));
      }
    }
  }
  /*
   * On which sides do articles in the past iterator range sit
   * relative to pos? Only counting articles that block the article
   * from being pushed to the edge.
   *
   * For A is each article in the iterator range:
   * - if pos overlaps A horizontally, consider the vertical dimensions.
   *  - if the bottom edge of pos is below A, A is blocked on the top edge
   *  - if the top edge of pos is below A, A is blocked on the bottom edge
   * - if pos overlaps A vertically, consider the horizontal dimensions.
   *  - if the right edge of pos is right of A, A is blocked on the left edge
   *  - if the left edge of pos is left of A, A is blocked on the right edge
   *
   * Returns a bitset of the edges on which this is blocked.
   */
#define adblLt(a,b) (a < b)
#define adblGt(a,b) (a > b)
  unsigned int find(const area &pos,
		    std::list<articlePlacement>::const_iterator begin,
		    std::list<articlePlacement>::const_iterator end) {
    unsigned int rtn=0;
    for (auto &art = begin; art != end; ++art) {
      auto &a = art->area_;

      /*
       * |A|
       *      |B|   - no overlap; A2 <= B1
       * |A|
       *   |B|      - no overlap; A2 <= B1
       * |  A  |
       *   |B|      - overlap; A2 > B1 && B1 < A2
       *   |A|  
       * |  B  |    - overlap; A2 > B1 && B1 < A2
       *   |A|
       * |B|        - no overlap; A1 >= B2
       *      |A|
       * |B|        - no overlap; A1 >= B2
       */

      /*
       * If A2 < Pos1, there is no overlap because A is to the left of Pos.
       * If A1 > Pos2, there is no overlap because A is to the right of Pos.
       * Otherwise, there must be an overlap. (Exactly equal is not overlapping for our purposes)
       *
       *     (!(a.x2() <= pos.x_ || a.x_ >= pos.x2()))
       * ==     a.x2() > pos.x_ && a.x_ < pos.x2()
       */

      if (a.x2() > pos.x_ && a.x_ < pos.x2()) {
	// horizontal overlap
	if (adblGt(a.y2(), pos.y2())) rtn |= BELOW;
	if (adblLt(a.y_, pos.y_))  rtn |= ABOVE;
      }

      if (a.y2() > pos.y_ && a.y_ < pos.y2()) {
	// vertical overlap
	if (adblLt(a.x_, pos.x_)) rtn |= LEFT;
	if (adblGt(a.x2(), pos.x2())) rtn |= RIGHT;
      }
    }
    return rtn;
  }
};

  /*
   * Layout is:
   *
   * |------------|----|
   * |about       |date|   about = 10 (BELOW|RIGHT)
   * |            |    |   date  = 1  (LEFT)
   * |            |    |   lipsum= 6  (ABOVE|RIGHT)
   * |------------|    |   git   = 15 (ABOVE|LEFT|RIGHT|BELOW)
   * |lipsum|git  |    |   git2  = 7  (ABOVE|LEFT|RIGHT)
   * |      |-----|    |
   * |      |git2 |    |
   * |------|_____|----|


about.art:	10	[542.025x295.361 @ 0      ,      0] ==> [542.025x295.361 @ 0      ,0]
date.art:	1	[ 216.81x499.545 @ 542.025,      0] ==> [ 216.81x0       @ 542.025,0]
lipsum.art:	6	[ 216.81x203.517 @ 0,      295.361] ==> [ 216.81x295.361 @ 0      ,295.361]
git.art:	15	[325.215x140.765 @ 216.81 ,295.361] ==> [325.215x140.765 @ 216.81 ,295.361]
git2.art:	7	[325.215x 72.505 @ 216.81 ,436.127] ==> [325.215x436.127 @ 216.81 ,436.127]


   */


} // namespace layout

#endif //fndef LAYOUT_TIDY_HPP
