#ifndef CMDLINE_HPP
#define CMDLINE_HPP

#include <string>
#include <memory>

// foolishly I'm not using Boost libraries to save dependencies, so
// let's process the command-line the old-fashioned way
class cmdline {
private:
  class impl;
  std::unique_ptr<impl> pImpl_;
public:
  /*
   * Parse the passed command-line
   */
  cmdline(int argc, char** argv);
  ~cmdline();
  /*
   * True if we should output help text
   */
  bool isHelp() const;
  /*
   * Force help text
   */
  void doHelp();
  /*
   * Get argument with the given name 
   * (leading hyphens ignored in user input)
   * param: case-insensitive. No leading hyphens.
   */
  std::string get(const std::string &param, const std::string &pdefault = std::string("")) const;
  /*
   * Get argument with the given name as boolean.
   * Empty strings are false. Otherwise, only the first char is considered.
   * 0,f,F,"" = false; anything else is true.
   * param: case-insensitive. No leading hyphens.
   */
  bool getBool(const std::string & param, const bool pdefault = false) const;
  /*
   * Test for presence of (perhaps required) argument
   * param: case-insensitive. No leading hyphens.
   * returns true if present, false if omitted
   */
  bool has(const std::string &param) const;
};

#endif //ndef CMDLINE_HPP
