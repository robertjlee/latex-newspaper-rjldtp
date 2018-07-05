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



ArticleOption::ArticleOption(int numCols, double width, double length) :
  numCols_(numCols),
  width_(width),
  length_(length) {
  if (numCols <= 0) throw "Columns not a positive integer!";
  }
ArticleOption::ArticleOption(const ArticleOption &o) :
  numCols_(o.numCols_),
  width_(o.width_),
  length_(o.length_) {}
ArticleOption ArticleOption::operator =(const ArticleOption &o) {
  numCols_ = o.numCols_;
  width_ = o.width_;
  length_ = o.length_;
  return *this;
}
ArticleOption::~ArticleOption() {}
int ArticleOption::numCols() const { return numCols_; }
double ArticleOption::length() const { return length_; }

double ArticleOption::area() const {
  return length_ * width_;
}

double ArticleOption::layoutWidth() const {
  return width_;
}

double ArticleOption::layoutHeight() const {
  return length_;
}




Article::Article(int artId, const std::string &filename) : 
  artId_(artId),
  filename_(filename) {}
Article::~Article() {}
void Article::addOption(int numCols, double width, double length) {
  // NB: May be better to change the implementation to a set and keep sorted.
  // we can't guarantee that articles will be in sorted order, as headlines can make articles start going bigger as the number of columns increases. We assume that we want smallest articles first.
  ArticleOption option(numCols, width, length);
  auto pos = std::upper_bound(options_.begin(), options_.end(), option,
			       [] (const ArticleOption& a, const ArticleOption &b) {
				 return a.area() < b.area();
			       });
  options_.insert(pos, option);

  //options_.emplace_back(numCols, width, length);
  //    ::std::cout << " Article option @ " << numCols << " has length " << length << " col cm " << std::endl;
}
std::vector<ArticleOption>::iterator Article::begin() { return options_.begin(); }
std::vector<ArticleOption>::const_iterator Article::begin() const { return options_.begin(); }
std::vector<ArticleOption>::iterator Article::end() { return options_.end(); }
std::vector<ArticleOption>::const_iterator Article::end() const { return options_.end(); }
int Article::size() const {
  return options_.size();
}
const std::string& Article::filename() const {
  return filename_;
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

std::string Page::layfile() const {
  return layfile_;
}
void Page::layfile(const std::string &filename) {
  layfile_ = filename;
}

const Article & Page::operator[] (const int idx) const {
  return arts_[idx];
}
Article &Page::back() {
  return arts_.back();
}
const Article &Page::back() const {
  return arts_.back();
}
bool Page::empty() const {
  return arts_.empty();
}
std::vector<Article>::iterator Page::begin() {
  return arts_.begin();
}
std::vector<Article>::const_iterator Page::begin() const {
  return arts_.begin();
}
std::vector<Article>::iterator Page::end() {
  return arts_.end();
}
std::vector<Article>::const_iterator Page::end() const {
  return arts_.end();
}



Page::Page(const Page &other) :
  width_(other.width()),
  height_(other.height()),
  colWidth_(other.colWidth()) {
  for (int i=0; i < other.articles(); ++i)
    arts_.push_back(other[i]);
}

Page & Page::operator=(const Page &other) {
  width_ = other.width();
  height_ = other.height();
  colWidth_ = other.colWidth();
  for (int i=0; i < other.articles(); ++i)
    arts_.push_back(other[i]);
  return *this;
}

Article & Page::newArticle(const std::string & filename) {
  int id = arts_.size();
  arts_.emplace_back(id, filename);
  return arts_.back();
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
  std::vector<std::vector<int> > combinations = calcPermutations();
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
      area += arts_[j++][idx].area();
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
    v.emplace_back((a.begin() + toRemap[i])->area(),
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
 * Algorithm: We have N articles, each with P(N) options:
 *
 * Options: (1,2,3) ; (4,5) ; (6)
 * P(N): 3,2,1
 *
 * P(N) >= 1, N and P(N) both integer.
 *
 * We start by calculating the product "prod" of P(N) for all N; this
 * is the number of results (6 is this example)
 *
 * For x in articles, divide "prod" by P(N); add "prod" of each
 * result to the output.
 *
 * article 1, "prod" becomes 6/3=2, result list is:
 * [1] [1] [2] [2] [3] [3]
 * article 2, "prod" becomes 2/2=1, result list is:
 * [1,4] [1,5] [2,4] [2,5]
 * article 3, "prod" becomes 1/1=1, result list is:
 * [1,4,6] [1,5,6] [2,4,6] [2,5,6]
 */
std::vector<std::vector<int > >
Page::calcPermutations() const {
  //  std::cout << "Calculating permutations" << std::endl;
  std::vector<std::vector<int > > rtn;
  int prod=1;
  for (auto a : arts_)
    prod *= a.size();
  for (int i=0; i < prod; ++i)
    rtn.push_back(std::vector<int>());
  const int totalResults = prod; // rtn.size()
  //  std::cout << " Prepared " << prod << " results" << std::endl;
  int num = prod;
  for (auto a : arts_) {
    // std::cout << "Next article: " << a.filename() << std::endl;
    num /= a.size();
    int i=0;
    while (i < totalResults) {
      int optIdx=0;
      for (auto o : a) {
	for (int loop =0; loop < num; ++loop) {
	  rtn[i].push_back(optIdx);// record index into options
	  ++i;
	}
	++optIdx;
      }
    }

  }
  // std::cout << "Pemutations done" << std::endl;
  // std::cout << "Permutations are: " << rtn;
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
