lib_dir = lib
java_dir = java
build_dir = build
phoeg_dir = phoeg/src
java = java
javac = javac
jnipath = -I/usr/lib/jvm/java-8-openjdk/include/ -I/usr/lib/jvm/java-8-openjdk/include/linux/
export LD_LIBRARY_PATH = /usr/lib/jvm/java-8-openjdk/jre/lib/amd64/server

empty=
space=$(empty) $(empty)

classpath = $(build_dir):$(subst $(space),:,$(wildcard $(lib_dir)/*.jar))

VPATH = $(java_dir):$(build_dir)

.PHONY: default init clean

default : init Neo4jAdapter.class comptransproof run
#default: rundup

runsg: compsg
	./subgraphtrs

compsg: subgraphtrs.cpp
	rm -f subgraphtrs
	g++ -std=c++11 -g -o subgraphtrs subgraphtrs.cpp -lnauty

rundup: compdup
	./testdup

compdup: subgraphtrs.cpp
	rm -f testdup
	g++ -std=c++11 -g -o testdup test-duplicates.cpp -lnauty

run: init Neo4jAdapter.class comptransproof
ifneq ("$(wildcard test5.db/.)","")
	rm -r test5.db
endif
	./transproof sigs5.csv

test: init Neo4jAdapter.class comptest
	./test

debug:
	gdb --args ./transproof eci.csv

valgrind:
	#valgrind --gen-suppressions=all --tool=memcheck --leak-check=yes --log-file=valgrind-suppr ./transproof eci.csv
	valgrind --tool=massif --time-unit=B ./transproof eci.csv

comptest: test.cpp
	g++ -std=c++11 $(jnipath) -L/usr/bin/java -L/usr/lib/jvm/java-8-openjdk/jre/lib/amd64/server/ -Llib -o test test.cpp

comptransproof: transproof.cpp
	g++ -std=c++11 -g $(jnipath) -Iphoeg/src -Lphoeg/src -L/usr/bin/java -L/usr/lib/jvm/java-8-openjdk/jre/lib/amd64/server/ -Llib -o transproof transproof.cpp -ljvm -pthread -lnauty

comppathshow: utils/pathshow.cpp
	g++ -std=c++11 -g -Llib -I. -o pathshow utils/pathshow.cpp nauty.h -pthread -lnauty

compordcomp: utils/ordcomp.cpp
	g++ -std=c++11 -g -Llib -I. -o ordcomp utils/ordcomp.cpp nauty.h -pthread -lnauty

# runJava: Neo4jAdapter.class
# @$(java) -cp $(classpath) Neo4jAdapter

init: clean
	@if ! [ -d "$(build_dir)/" ]; then\
		mkdir $(build_dir);\
	fi

clean:
	@if [ -d "$(build_dir)/" ]; then\
		rm -r $(build_dir);\
	fi
	@if [ -e "transproof" ]; then\
		rm transproof;\
	fi
	@if [ -e "test" ]; then\
		rm test;\
	fi

%.class: %.java init
	@$(javac) -cp $(classpath) -d $(build_dir) $<
