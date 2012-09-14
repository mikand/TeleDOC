all: test

main.o: main.cpp
	g++ -g -O0 -DOPENCV_2 -c main.cpp -o main.o

tracker.o: tracker.cpp
	g++ -g -O0 -DOPENCV_2 -c tracker.cpp -o tracker.o

test: main.o tracker.o
	g++ -g -O0 main.o tracker.o -lopencv_core -lopencv_highgui -lopencv_imgproc -o test

clean:
	rm -rf test, main.o tracker.o