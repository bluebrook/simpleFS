all: test

OBJS := disk.o fs.o 

CFLAGS = -g -std=c++11 -c
LDFLAGS = -lstdc++

gcc = gcc

test: $(OBJS) fs_test.cpp
	$(gcc) -g -std=c++11 -c fs_test.cpp 
	$(gcc) -g -o test fs_test.o $(OBJS)  $(LDFLAGS)

%.o: %.cpp %.h
	$(gcc) $(CFLAGS) $*.cpp

clean:
	rm -fv *.o
	rm -fv test
