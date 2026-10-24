all: lab2 lab2_tsan

lab2:
	g++ -O2 -std=c++20 -pthread src/*.cpp -o lab2

lab2_tsan:
	clang++ -fsanitize=thread -g -O1 -std=c++20 -pthread src/*.cpp -o lab2_tsan

clean:
	rm lab2 lab2_tsan
