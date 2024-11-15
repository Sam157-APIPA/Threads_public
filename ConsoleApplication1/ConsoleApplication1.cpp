#include "ThreadPool.h"
#include<iostream>

void print() {
	for(int i=0;i<100;i++){
	printf("xd\n");
	this_thread::sleep_for(std::chrono::microseconds(30));
	}
}
void print2() {
	for (int i = 0; i < 100; i++) {
		printf("apipa\n");
		this_thread::sleep_for(std::chrono::microseconds(30));
	}
}

int main() {

	threadsPool pool;
	pool.addToQueue(print);
	pool.addToQueue(print2);
	this_thread::sleep_for(std::chrono::seconds(30));
}