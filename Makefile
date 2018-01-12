phoeg_dir = phoeg/src
lib_dir = lib
javac = javac
jdkpath = /usr/lib/jvm/java-8-openjdk/
jre_server = $(jdkpath)/jre/lib/amd64/server/

#both can be set from commandline
output_db = test5
input_file = sigs5.csv

empty=
space = $(empty) $(empty)
classpath = .:$(subst $(space),:,$(wildcard $(lib_dir)/*.jar))
cpcmd = "-Djava.class.path=$(classpath)"

INCFLAGS = -I$(phoeg_dir) -I$(jdkpath)include/ -I$(jdkpath)include/linux -I/usr/include/docopt/
CXXFLAGS = -std=c++11 -g -DCPCMD='$(cpcmd)' $(INCFLAGS)
LDFLAGS = -L$(phoeg_dir) -L$(jre_server) -L$(lib_dir)
LDLIBS = -ljvm -pthread -lnauty -ldocopt

export LD_LIBRARY_PATH = $(jre_server)

default: clean Neo4jAdapter.class runtransproof

#run with "make run exec=<cpp_to_compile_and_exec>"
run: $(exec)
	./$(exec)

runtransproof: transproof
	$(RM) -r $(output_db)
	./transproof --output=$(output_db) $(input_file)


%.class : %.java
	$(javac) $(jflags) -cp $(classpath) $<

clean:
	$(RM) transproof
	$(RM) Neo4jAdapter.class
