all: main
main: main.o
	g++ -std=c++17 $^ -L../teseo-master/build -lteseo -lnuma -levent_pthreads -levent_core -lpthread -o$@
	rm main.o

main.o: main.cpp
	g++ -std=c++17 -c -O3 $^ -I../teseo-master/include -I../teseo-master/build/include -o$@
