2dfit: 2dfit.cpp data.o typeset.o layout_worst.hpp debug.hpp data.hpp typeset.hpp
	c++ -std=c++1y 2dfit.cpp *.o -O3 -o 2dfit

data.o : data.cpp debug.hpp data.hpp
	c++ -std=c++1y data.cpp -O3 -c

typeset.o : data.cpp debug.hpp typeset.cpp typeset.hpp
	c++ -std=c++1y typeset.cpp -O3 -c
