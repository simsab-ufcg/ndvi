CC=g++
CFLAGS = -std=c++14
CLTIFF = -ltiff

DEPS = assets/*.h

SRCS = $(shell find -name *.cpp)
OBJS := $(addsuffix .o,$(basename $(SRCS)))

all: run

assets/%.o: assets/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CLTIFF)
	
run: $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(CLTIFF)

clean: 
	rm -f run
	rm -rf *.o
	rm -rf assets/*.o