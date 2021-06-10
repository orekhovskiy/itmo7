SOURCES := $(wildcard Dead*.cpp)
TARGETS := $(patsubst %.cpp,%.out,$(SOURCES))


all: $(TARGETS)


Dead%: Dead%.out
	  LD_PRELOAD=../sanitizer.so  ./$@.out 

%.out:
	$(CXX) -std=c++17 -ldl -g -O3 -Werror -Wall -Wextra -pthread -pedantic -o $@ $(patsubst %.out,%.cpp,$@) 

clean:
	rm -f ./*.out
