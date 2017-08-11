#include "cmdline.hpp"
#include <vector>

class cmdline::impl {
private:
  std::vector<std::string> args_;
  // downcase and remove leading hyphens
  std::string lcst(const char* arg) const {
    std::string rtn;
    bool hyph = true; // true while stripping leading hyphens
    for (const char* c = arg; *c != '\0'; ++c) {
      if (hyph && *c == '-') continue;
      else hyph = false;
      if (*c >= 'A' && *c <= 'Z') rtn += ((*c) - 'A' + 'a');
      else rtn += *c;
    }
    return rtn;
  }
  std::string lcst(const std::string & arg) const {
    return lcst(arg.c_str());
  }
public:
  /*
   * Parse the passed command-line
   */
  impl(int argc, char** argv) {
    for (int i=1; i < argc; ++i) 
      args_.emplace_back(argv[i]);
  }
  ~impl() {}
  /*
   * True if we should output help text
   */
  bool isHelp() const {
    if (args_.size() == 0) return true;
    for (auto str : args_)
      if (str.compare("/?") == 0 ||
	  lcst(str).compare("?") == 0 ||
	  lcst(str).compare("h") == 0 ||
	  lcst(str).compare("help") == 0)
	return true;
    return false;
  }
  /*
   * Force help text
   */
  void doHelp() {
    args_.clear(); // prentend no arguments passed
  }
  /*
   * Get argument with the given name 
   * (leading hyphens ignored in user input)
   * param: case-insensitive. No leading hyphens.
   */
  std::string get(const std::string &param, const std::string & pdefault) const {
    bool next = false;
    for (auto str : args_) {
      if (next) return str;
      if (lcst(str).compare(param) == 0) next = true;
    }
    return pdefault;
  }
  /*
   * Test for presence of (perhaps required) argument
   * param: case-insensitive. No leading hyphens.
   * returns true if present, false if omitted
   */
  bool has(const std::string &param) const {
    bool next = false;
    for (auto str : args_) {
      if (next) return true;
      if (lcst(str).compare(param) == 0) next = true;
    }
    return false;
  }
};

cmdline::cmdline(int argc, char** argv) :
  pImpl_(new impl(argc, argv)) {};

cmdline::~cmdline() {};
/*
 * True if we should output help text
 */
bool cmdline::isHelp() const {
  return pImpl_->isHelp();
}
/*
 * Force help text
 */
void cmdline::doHelp() {
  pImpl_->doHelp();
}
/*
 * Get argument with the given name 
 * (leading hyphens ignored in user input)
 * param: case-insensitive. No leading hyphens.
 */
std::string cmdline::get(const std::string &param, const std::string & pdefault) const {
  return pImpl_->get(param, pdefault);
}
bool cmdline::getBool(const std::string & param, const bool pdefault) const {
  std::string strVal=get(param, std::string(pdefault ? "T" : ""));
  if (strVal.length() == 0) return false;
  char c = strVal[0];
  return !(c == '0' || c == 'f' || c == 'F');
}
/*
 * Test for presence of (perhaps required) argument
 * param: case-insensitive. No leading hyphens.
 * returns true if present, false if omitted
 */
bool cmdline::has(const std::string &param) const {
  return pImpl_->has(param);
}
