all: Scheduler

Scheduler: Scheduler.cpp
	g++ -O3 -o Scheduler Scheduler.cpp -I/usr/local/cuda/include -lrt -lpthread -lc -lOpenCL

clean:
	rm Scheduler
