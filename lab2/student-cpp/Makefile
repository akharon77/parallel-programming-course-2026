all: lab2 lab2_tsan stress ministress

lab2:
	g++ -O2 -std=c++20 -pthread src/*.cpp -o lab2

lab2_tsan:
	clang++ -fsanitize=thread -g -O1 -std=c++20 -pthread src/*.cpp -o lab2_tsan

stress:
	g++ -O2 -std=c++20 -pthread extra/stress.cpp -o stress

ministress:
	g++ -O2 -std=c++20 -pthread extra/ministress.cpp -o ministress

clean:
	rm lab2 lab2_tsan stress ministress
