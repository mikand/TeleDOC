all: test

main.o: main.cpp
	g++ -DOPENCV_2 -c main.cpp -o main.o

tracker.o: tracker.cpp
	g++ -DOPENCV_2 -c tracker.cpp -o tracker.o

test: main.o tracker.o
	g++ main.o tracker.o -lopencv_core -lopencv_highgui -lopencv_imgproc -o test

clean:
	rm -rf test, main.o tracker.o