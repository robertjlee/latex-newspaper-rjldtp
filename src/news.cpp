/*
 * A program to lay out newspaper pages
 */

#include "data.hpp"
#include "layout_worst.hpp"
#include "typeset.hpp"
#include "cmdline.hpp"
#include "process.hpp"
#include <iostream>
#include <sstream>

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


/*
 * Read the next comma-separated token from the given stream
 * and return it.
 */
template<typename T>
T readCSV(std::istream &in);
template<>
std::string readCSV<std::string>(std::istream &in) {
  std::string item;
  std::getline(in, item, ',');
  return item;
}
template<>
double readCSV<double>(std::istream &in) {
  std::string text = readCSV<std::string>(in);
  // strip "pt" suffix and convert to double:
  text.resize(text.length() - 2);
  return std::atof(text.c_str());
}
template<>
int readCSV<int>(std::istream &in) {
  std::string text = readCSV<std::string>(in);
  return std::atoi(text.c_str());
}


/*
 * Populate arts (articles and options) from the given
 * size_calculator process's result.
 * Returns the page with these articles, whose dimensions are
 * also read from the stream.
 */
Page readArtOptions(shellout &size_calculator, bool verbose) {
  std::string shellline;
  Page page(0,0);
  while (!size_calculator.eof()) {
    size_calculator >> shellline;
    if (shellline.compare(0,10,"PAGESIZE: ") == 0) {
      std::stringstream instream(shellline.substr(10));
      double width = readCSV<double>(instream);
      double length = readCSV<double>(instream);
      page = Page(width, length);

    } else if (shellline.compare(0,24,"COLS,WIDTH,HEIGHT,FILE: ") == 0) {
      std::stringstream instream(shellline.substr(24));
      int numCols = readCSV<int>(instream);
      double width = readCSV<double>(instream);
      double length = readCSV<double>(instream);
      std::string file = readCSV<std::string>(instream);

      if (page.empty() || page.back().filename() != file)
	page.newArticle(file);

      auto &art=page.back();
      art.addOption(numCols, width, length);
    } else if (shellline.compare(0,23,"Generating Layout file ") == 0) {
      auto filename = shellline.substr(23);
      page.layfile(filename);
      std::cout << "Writing to " << filename << std::endl;
    } else if (verbose) {
      std::cout << shellline << std::endl;
    }
  }

  // tell the user what we're considering:
  std::cout << "Page size is " << page.width() << " by " << page.height() 
	    << " ( = " << (page.width() * page.height()) << " pt^2)"
	    << std::endl;
  std::cout << "Articles (count=" << page.articles() << "):" << std::endl;
  for (auto &art : page) {
    std::cout << "Article #" << art.id() << " (" << art.filename() << ')' << std::endl;
    for (auto opt : art) {
      std::cout << '\t' << opt.numCols() << " cols (" << opt.layoutWidth()
		<< " pts)\tgives " << opt.layoutHeight() << " points\t(area=" 
		<< opt.area() << " pt^2)"
		<< std::endl;
    }
  }

  return page;
}

int main(int argc, char** argv) {
  using namespace std;

  // first read the parameters
  cmdline cmd(argc, argv);
  if (!cmd.has("file"))
    cmd.doHelp();
  if (cmd.isHelp()) {
    std::cout 
      << "Usage: " << argv[0]
      << " --file <file>.tex"
      << " [--verbose t]"
      << " [--stage size|set|all]"
      << std::endl
      << " --file: (required): LaTeX input source file to process"
      << std::endl
      << " --verbose: Boolean; include LaTeX logging on standard output"
      << std::endl
      << " --stage:"
      << std::endl
      << "          size; layout and generate the .lay file only"
      << std::endl
      << "          lay; typeset with existing .lay file"
      << std::endl
      << "          all (default); layout and produce final file"
      << std::endl
      << " --output-directory <dir>; passed to LaTeX"
      << std::endl
      << "          (directory will be searched for .cls file)"
      << std::endl;
    return 0;
  }

  std::string file = cmd.get("file");
  std::string texcmd = cmd.get("latex", "pdflatex");
  std::cout << "Processing " << file << std::endl;

  auto stage = cmd.get("stage", "all");
  bool stageSize = (stage[0] == 's' || stage[0] == 'S' || stage[0] == 'a' || stage[0] == 'A');
  bool stageSet = (stage[0] == 'l' || stage[0] == 'L' || stage[0] == 'a' || stage[0] == 'A');

  std::string outdir = cmd.get("output-directory", ".");

  // you can use \PassOptionsToClass{class}{option}\input{file}
  // see https://tex.stackexchange.com/questions/1492/passing-parameters-to-a-document#answer-22525
  // Answer 25699 is a trap; it claims to provide a more natural syntax but
  // needs extra parsing in LaTeX, and will error if a command-line argument
  // is not passed.
  if (stageSize) {
    std::string texcmdline = texcmd + 
      " -interaction=nonstopmode -output-directory=" + outdir + " " + 
      "'\\PassOptionsToClass{sizing}{rjlnewsp4} \\input{" +
      file + "}'";
    shellout size_calculator(texcmdline);
    
    Page p = readArtOptions(size_calculator, cmd.getBool("verbose"));
    
    std::cout << "Page has " << p.articles() << " article options " << std::endl;

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
      // typeset the result into the .lay file:
      typeset::setter set;
      set(p, result);
    } catch (const char* error) {
      cout << error << endl;
      return 1;
    }
  }

  try {
    if (stageSet) {
      std::string texcmdline = texcmd +
	" -interaction=nonstopmode -output-directory=" +outdir+ " " + 
	"'\\PassOptionsToClass{layoutnews}{rjlnewsp4} \\input{" +
	file + "}'";
      shellout generation(texcmdline);
      while (!generation.eof()) {
	std::string line;
	generation >> line;
	if (cmd.getBool("verbose"))
	  std::cout << line << std::endl;
      }
    }

  } catch (const char* error) {
    cout << error << endl;
    return 1;
  }
}

