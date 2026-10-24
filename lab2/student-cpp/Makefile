all: lab2 lab2_tsan stress ministress

SRC=$(wildcard src/*.cpp)
HDRS=$(wildcard src/*.hpp)

lab2: $(SRC) $(HDRS)
	g++ -O2 -std=c++20 -pthread $(SRC) -o lab2

lab2_tsan: $(SRC) $(HDRS)
	clang++ -fsanitize=thread -g -O1 -std=c++20 -pthread $(SRC) -o lab2_tsan

stress: extra/stress.cpp
	g++ -O2 -std=c++20 -pthread $^ -o stress

ministress: extra/ministress.cpp
	g++ -O2 -std=c++20 -pthread $^ -o ministress

clean:
	rm lab2 lab2_tsan stress ministress
