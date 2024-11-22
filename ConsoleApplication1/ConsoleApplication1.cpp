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

void sum4(int* mas, int size, int& res) {
	int result = 0;
	for (int i = 0; i < size; ++i)
	{
		result += mas[i];
		for (int j = i + 1; j < size; ++j) 
		{
			result += mas[j];
			for (int p = j + 1; p < size; ++p) {
				result += mas[p];
				for (int q = p + 1; q < size; ++q)
					result += mas[q] * mas[q];
			}
		}
	}
	//cout << result << endl;
	res = result;
	//cout << res << endl;
}

int main() {
	int mas[10] = {1,2,3,4,5,6,7,8,9,10};
	int size = 10;
	int res = 0;
	threadsPool pool;
	cout << pool.getCntThreads() << endl;
	//cout << sum4(mas, size);
	pool.addToQueue(bind(sum4, mas, size, std::ref(res)));
	this_thread::sleep_for(std::chrono::seconds(5));
	cout << res<<endl;
	//pool.addToQueue(print);
	//pool.addToQueue(print2);
	
}