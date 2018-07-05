#ifndef DATA_HPP
#define DATA_HPP

#include "debug.hpp"
#include <vector>
#include <algorithm>
#include <iostream>

class area;


/*
 * Is a > b within eps of a measurement unit?
 */
bool dblGt(const double a, const double b, const double eps = 0.0001);

/*
 * Is a < b within eps of a measurement unit?
 * Note that this is not the inverse of dblGt
 */
bool dblLt(const double a, const double b, const double eps = 0.0001);

// simple POJO to hold the location of an article on the page
class area {
public:
  double w_, h_, x_, y_;
  area();
  area(const double & w,const double & h,const double & x,const double & y);
  area(const area & a);
  area operator =(const area & a);
  ~area();
  double size() const;
  double x2() const { return x_ + w_; };
  double y2() const { return y_ + h_; };
};


// stores one possible way of laying out an article.
// currently we allow articles to be laid out in rectangles with different numbers of
// columns, and this class provides the size of paper required based on the number of column inches.
class ArticleOption {
private:
  // number of columns on which the article is set out
  int numCols_;
  // layout width of the article (including gutters)
  double width_;
  // layout length of the article (not column inches)
  double length_;
public:
  ArticleOption(int numCols, double width, double length);
  ArticleOption(const ArticleOption &o);
  ArticleOption operator =(const ArticleOption &o);
  ~ArticleOption();
  int numCols() const;
  double length() const;
  double area() const;
  double layoutWidth() const;
  double layoutHeight() const;
};

// Article: holds metadata about an individual piece of text to be typeset,
// including a collection of all different layout methods (ArticleOptions)
// can be used as a container of ArticleOptions (begin(), end(), operator[] etc).
class Article {
private:
  // Internal identifier. This is an index into some external collection, so we can re-sort articles.
  int artId_;
  std::string filename_;
  // List of all possible article options, sorted into smallest area first.
  std::vector<ArticleOption> options_;
public:
  Article(int artId, const std::string & filename);
  ~Article();
  void addOption(int numCols, double width, double length);
  std::vector<ArticleOption>::iterator begin();
  std::vector<ArticleOption>::const_iterator begin() const;
  std::vector<ArticleOption>::iterator end();
  std::vector<ArticleOption>::const_iterator end() const;
  int size() const;
  const std::string &filename() const;
  ArticleOption & operator[] (const int idx);
  const ArticleOption & operator[] (const int idx) const;
  int id() const;
};


/*
 * Models the page onto which the articles will be typeset, including the layout algorithm(s).
 */
class Page {
private:
  // theory: max cols should be no more than half the width of the paper?
  static const int MAX_COLS_PER_ARTICLE = 4;
  double width_;
  double height_;
  double colWidth_; // let's ignore alleys for now
  std::vector<Article> arts_;
  std::string layfile_; // output file for layout
public:
  Page(double width, double height, 
       double colWidth = 46.56666663 /*11 picas in mm*/);
  Page(const Page &other);
  ~Page();
  Page &operator=(const Page &other);
  int colWidth() const;
  double width() const;
  double height() const;

  std::string layfile() const;
  void layfile(const std::string &filename);

  const Article & operator[] (const int idx) const;
  Article & back();
  const Article & back() const;
  bool empty() const;
  std::vector<Article>::iterator begin();
  std::vector<Article>::const_iterator begin() const;
  std::vector<Article>::iterator end();
  std::vector<Article>::const_iterator end() const;

  /*
   * Create an article with the given filename for this page
   */
  Article & newArticle(const std::string & filename);

  /*
   * Returns a vector of indicies, in article order.
   * Each index is the offset within the list of options for that article.
   * eg [3,2,5] would mean the 3rd option for the first article, the 2nd for the next, and the 5th for the third.
   */
  std::vector<int> findBestOptions() const;

  /*
   * Sort arts_ such that largest artcles come first.
   * takes a parameter of a vector of the same size as arts_, and returns it modified by
   * the same reordering as arts_
   */
  std::vector<int> sortArticlesBySize(std::vector<int> toRemap);

  /*
   * Given A = [1,2,3] [1,2,3]
   * Produce combinations:
   * 1,1 ; 1,2 ; 1,3 ; 2,1 ; 2,2 ; 2,3 ; 3,1 ; 3,2 ; 3,3
   */
  std::vector<std::vector<int > > 
  calcPermutations() const;

  // for debugging: how many article options are to be considered.
  // may give a little indication of the time to be taken.
  int articles() const;

};


namespace layout {

  /*
   * Concept: A layout routine will supply the following signature:
   *
   * rtn operator()(const Page & p, const std::vector<int> & preferredArticles) {
   *
   * rtn.begin() and rtn.end() both return forward_const_iterator<articlePlacement>.
   * p supplies the page information, including articles and article options to be laid out
   * preferredArticles supplies the preferred index into the article
   *   options list for each article. An earlier option in the list may
   *   be used to find a valid layout.
   */

  /*
   * Not strictly part of the immutable data model, as this defines
   * the output of the layout routine.
   */
  struct articlePlacement {
  const area area_;
  const Article &art_;
  const ArticleOption &opt_;
  articlePlacement(const area area,
		   const Article &art,
		   const ArticleOption &opt) :
    area_(area), art_(art), opt_(opt) {};
  articlePlacement(const articlePlacement &rhs) :
    area_(rhs.area_),
    art_(rhs.art_),
    opt_(rhs.opt_) {}
  ~articlePlacement() {}
    articlePlacement & operator =(const articlePlacement & rhs) = delete;
};

}


#endif // ndef DATA_HPP
