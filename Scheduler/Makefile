all: Scheduler Scheduler2 Sample

Scheduler: Scheduler.cpp
	g++ -O3 -o Scheduler Scheduler.cpp -I/usr/local/cuda/include -lrt -lpthread -lc -lOpenCL

Scheduler2: Scheduler2.cpp
	g++ -O3 -o Scheduler2 Scheduler2.cpp -I/usr/local/cuda/include -lrt -lpthread -lc -lOpenCL

Sample: Sample.c
	gcc -O3 -o Sample Sample.c -lrt -lc

clean:
	rm Sample Scheduler
