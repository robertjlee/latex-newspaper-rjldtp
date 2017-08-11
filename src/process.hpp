#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

class shellout {
private:
  std::array<char, 128> buffer_;
  std::shared_ptr<FILE> pipe_;
public:
  shellout(std::string &cmd) :
    pipe_(popen(cmd.c_str(), "r"), pclose) {
    std::cout << "Executing [" << cmd.c_str() << ']' << std::endl;
    if (!pipe_) throw std::runtime_error("popen() failed!");
  }

  bool eof() const {
    return feof(pipe_.get());
  }

  shellout & operator >> (std::string &line) {
    if (eof()) throw std::ios_base::failure("EOF");
    if (fgets(buffer_.data(), 128, pipe_.get()) != NULL) {
      line = buffer_.data();
      while (line.size() > 0 && 
	     (line.back() == '\r' || line.back() == '\n'))
      line.resize(line.length() - 1);
    }
    return *this;
  }
};


#endif //ndef PROCESS_HPP
