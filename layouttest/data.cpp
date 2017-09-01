/*
 * Implementation of the immutable data's model, ie the page and articles to be
 * laid out
 */

#include "data.hpp"

double area::size() const { return w_ * h_; }
area::area() : 
  w_(0), h_(0), x_(0), y_(0) {}
area::area(const double & w,const double & h,const double & x,const double & y) :
  w_(w), h_(h), x_(x), y_(y) {}
area::area(const area & a) {
  w_ = a.w_; h_ = a.h_; x_ = a.x_; y_ = a.y_;
}
area area::operator =(const area & a) {
  w_ = a.w_; h_ = a.h_; x_ = a.x_; y_ = a.y_;   
  return *this;
}
area::~area() {}



ArticleOption::ArticleOption(int numCols, double length) :
  numCols_(numCols),
  length_(length) {
  if (numCols <= 0) throw "Columns not a positive integer!";
  }
ArticleOption::ArticleOption(const ArticleOption &o) :
  numCols_(o.numCols_),
  length_(o.length_) {}
ArticleOption ArticleOption::operator =(const ArticleOption &o) {
  numCols_ = o.numCols_;
  length_ = o.length_;
  return *this;
}
ArticleOption::~ArticleOption() {}
int ArticleOption::numCols() const { return numCols_; }
double ArticleOption::length() const { return length_; }

double ArticleOption::area(double colWidth) const {
  return length_ * colWidth * numCols_;
}

double ArticleOption::layoutWidth(double colWidth) const {
  return colWidth * numCols_;
}

double ArticleOption::layoutHeight() const {
  return length_;
}




Article::Article(int artId) : artId_(artId) {}
Article::~Article() {}
void Article::addOption(int numCols, double length) {
  options_.emplace_back(numCols, length);
  //    ::std::cout << " Article option @ " << numCols << " has length " << length << " col cm " << std::endl;
}
std::vector<ArticleOption>::iterator Article::begin() { return options_.begin(); }
std::vector<ArticleOption>::const_iterator Article::begin() const { return options_.begin(); }
std::vector<ArticleOption>::iterator Article::end() { return options_.end(); }
std::vector<ArticleOption>::const_iterator Article::end() const { return options_.end(); }
int Article::size() const {
  return options_.size();
}
ArticleOption & Article::operator[] (const int idx) {
  return options_[idx];
}
const ArticleOption & Article::operator[] (const int idx) const {
  return options_[idx];
}
int Article::id() const { return artId_; }




Page::Page(double width, double height, double colWidth) :
  width_(width),
  height_(height),
  colWidth_(colWidth) {}
Page::~Page() {}
int Page::colWidth() const { return colWidth_; }
double Page::width() const { return width_; }
double Page::height() const { return height_; }

const Article & Page::operator[] (const int idx) const {
  return arts_[idx];
}

void Page::addArticle(int artId, int headingLength, double colInches) {
  int numCols; double artWidth;
  arts_.emplace_back(artId);
  auto& art = *(arts_.end() - 1);
  for (numCols = 1, artWidth = colWidth_;
       artWidth < width_ && numCols <= MAX_COLS_PER_ARTICLE;
       ++numCols, ++artWidth)
    // rough sizing:
    art.addOption(numCols, headingLength + colInches / numCols);
}

/*
 * Returns a vector of indicies, in article order.
 * Each index is the offset within the list of options for that article.
 * eg [3,2,5] would mean the 3rd option for the first article, the 2nd for the next, and the 5th for the third.
 */
std::vector<int> Page::findBestOptions() const {
  const double target = width_ * height_;
  /*
   * For each combination: calculate the total area.
   * Largest total area less than page area wins
   *
   */

  std::vector<std::vector<int> > combinations = calcPermutations(arts_.size(), arts_[0].size());
  if (0) {
    using namespace std;
    //cout << "Combinations";
    //cout << combinations << endl;
  }
  int best=-1; double bestArea = 0;
  int i=0;
  for (auto combo : combinations) {
    double area = 0;
    int j=0;
    for (auto idx : combo) {
      // std::cout << "arts_[" << j << "][" << idx << "] : ";
      area += arts_[j++][idx].area(colWidth_);
      // std::cout << area << std::endl;
    }

    if (area <= target && area > bestArea) {
      // std::cout << "New best!" << std::endl;
      bestArea = area; best = i;
    }

    ++i;
  }

  if (best < 0) {
    throw "No solution without page overflow";
  }
  auto &bestCombo = combinations[best];

  using namespace std;
  cout << "Best result: area = " << bestArea << "; " << bestCombo << endl;

  return bestCombo;
}

/*
 * Sort arts_ such that largest artcles come first.
 * takes a parameter of a vector of the same size as arts_, and returns it modified by
 * the same reordering as arts_
 */
std::vector<int> Page::sortArticlesBySize(std::vector<int> toRemap) {
  using namespace std;
  // 0 - area
  // 1 - original index in list
  // 2 - value from toRemap
  vector<tuple<double, int, int> > v;
  int i=0;
  for (auto a : arts_) {
    v.emplace_back((a.begin() + toRemap[i])->area(colWidth_),
		   i,toRemap[i]);
    i++;
  }
  sort(v.begin(), v.end(), [](tuple<double,int,int> a,tuple<double,int,int> b){
      return get<0>(a) > get<0>(b);
    });
  vector<int> rtn;
  vector<Article> arts;
  for (auto t : v) {
    arts.emplace_back(arts_[get<1>(t)]);
    rtn.emplace_back(get<2>(t));
  }
  arts_ = arts;
  return rtn;
}

/*
 * Given A = [1,2,3] [1,2,3]
 * Produce combinations:
 * 1,1 ; 1,2 ; 1,3 ; 2,1 ; 2,2 ; 2,3 ; 3,1 ; 3,2 ; 3,3
 */
std::vector<std::vector<int > > 
Page::calcPermutations(int numArticles, int numOptions) const {
  std::vector<std::vector<int > > rtn;
  if (numArticles > 1) { // break condition
    auto tail = calcPermutations(numArticles - 1, numOptions);
    for (int i=0; i < numOptions; ++i) {
      for (auto tails : tail) {
	std::vector<int> ibox;
	ibox.emplace_back(i);
	int pos = rtn.size();
	rtn.emplace_back(ibox);
	for (auto t : tails) rtn[pos].push_back(t);
      }
    }
  } else {
    for (int i=0; i < numOptions; ++i) {
      std::vector<int> ibox;
      ibox.emplace_back(i);
      rtn.push_back(ibox);
    }
  }
  return rtn;
}

// for debugging: how many article options are to be considered.
// may give a little indication of the time to be taken.
int Page::articles() const {
  int rtn(0);
  for (auto& a : arts_) {
    rtn += a.size();
  }
  return rtn;
}
