CXXOPTS = -std=c++1y -O3 -Wall

news: news.cpp data.o typeset.o cmdline.o layout_worst.hpp layout_tidy.hpp debug.hpp data.hpp process.hpp
	c++ $(CXXOPTS) news.cpp *.o -o news

cmdline.o : cmdline.cpp cmdline.hpp
	c++ $(CXXOPTS) cmdline.cpp -c

data.o : data.cpp debug.hpp data.hpp
	c++ $(CXXOPTS) data.cpp -c

typeset.o : data.cpp debug.hpp typeset.cpp typeset.hpp
	c++ $(CXXOPTS) typeset.cpp -c


# Utility

clean :
	rm -f *.o *.log *.aux *.dvi *.glo *.idx
