/*
 * This is a proof-of-concept application to demonstrate the
 * algorithm(s) described in Readme.tex.
 */

#include "data.hpp"
#include "layout_worst.hpp"
#include "typeset.hpp"
#include <iostream>
#include <iterator>
#include <vector>
//#include <list>

/*
 * Is a > b within eps of a measurement unit?
 */
bool dblGt(const double a, const double b, const double eps) {
  return (a - eps > b);
}

std::ostream& operator<< (std::ostream& out, const area &a) {
  out << '[' << a.w_ << "x" << a.h_ << '@' 
      << a.x_ << "," << a.y_ << ']';
  return out;
}



int main() {
  using namespace std;

  // 600x750mm seems to be a common size for broadsheets.
  //  Page p(600,750); 

  Page p(600-41.20000044,300-45); 

  // short article of 10cm long with 2cm headline
  p.addArticle(1, 20, 100);

  // longer article
  p.addArticle(2, 25, 327.5);

  // some more
  p.addArticle(3, 21, 317.15);
  p.addArticle(4, 22, 137.25);
  p.addArticle(5, 23, 257.35);
  p.addArticle(6, 24, 377.45);
  p.addArticle(7, 25, 897.55);
  //  p.addArticle(8, 20, 22);

  /*
  int i=0;
  for (; i < 7; ++i) {
    p.addArticle(i, 10, 1000);
  }
  p.addArticle(++i, 0, 41.2);
  p.addArticle(++i, 0, 41.2);
  p.addArticle(++i, 0, 41.2);
  p.addArticle(++i, 0, 41.2);
  */

  cout << "Article options: " << p.articles() << endl;
  try {
    auto combo = p.findBestOptions();
    combo = p.sortArticlesBySize(combo);
    //    p.layoutRecurse(combo);
    auto layout = layout::worstFit<>();
    auto &result = layout(p, combo);
    for (auto &r : result) {
      cout << "Placing article #" << r.art_.id() << " at " << r.area_
	   << " (" << r.opt_.numCols() << " columns)"
	   << std::endl;
    }
    typeset::setter set;
    set(p, result);
    //    cout << result << endl;
  } catch (const char* error) {
    cout << error << endl;
    return 1;
  }


}

