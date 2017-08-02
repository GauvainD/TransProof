all: transfo
transfo: transfoparser.o transfoscanner.o transfo.o
	g++ -std=c++11 -o transfo transfodriver.hpp transfoparser.o transfoscanner.o transfo.o
transfoparser.o: transfoparser.cc transfoparser.hh
	g++ -c -std=c++11 transfoparser.cc
transfoparser.cc transfoparser.hh: transfoparser.yy
	bison --defines=transfoparser.hh -otransfoparser.cc transfoparser.yy
transfoscanner.o: transfoscanner.cc transfoparser.hh
	g++ -c -std=c++11 transfoscanner.cc
transfoscanner.cc: transfoscanner.ll
	flex -o transfoscanner.cc transfoscanner.ll
transfo.o: transfo.cpp
	g++ -c -std=c++11 transfo.cpp
.PHONY: clean
clean:
	-rm *.o transfoparser.hh transfoparser.cc transfoscanner.cc location.hh position.hh stack.hh transfo
