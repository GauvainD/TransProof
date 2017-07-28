all: parser
parser: myparser.o myscanner.o parser.o
	g++ -o parser mydriver.hpp myparser.o myscanner.o parser.o
myparser.o: myparser.cc myparser.hh
	g++ -c myparser.cc
myparser.cc myparser.hh: myparser.yy
	bison --defines=myparser.hh -omyparser.cc myparser.yy
myscanner.o: myscanner.cc myparser.hh
	g++ -c myscanner.cc
myscanner.cc: myscanner.ll
	flex -omyscanner.cc myscanner.ll
parser.o: parser.cpp
	g++ -c parser.cpp
.PHONY: clean
clean:
	-rm *.o myparser.hh myparser.cc myscanner.cc location.hh position.hh stack.hh parser
