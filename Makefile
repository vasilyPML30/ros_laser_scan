FLAGS=-c -std=c++11 -Wall -Wextra -Werror -I ./include

all: bin scanner
scanner: bin/main.o bin/scanner.o
	g++ bin/main.o bin/scanner.o -o scanner
bin/main.o: src/main.cpp include/scanner.h
	g++ $(FLAGS) src/main.cpp -o bin/main.o
bin/scanner.o: src/scanner.cpp include/scanner.h
	g++ $(FLAGS) src/scanner.cpp -o bin/scanner.o
clean:
	rm -rf bin scanner
bin:
	mkdir -p bin
