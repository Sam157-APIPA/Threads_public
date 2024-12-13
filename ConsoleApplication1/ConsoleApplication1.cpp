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

int modernRand(int n) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, n);
	return dis(gen);
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
	res = result;
}

int divide(int n) {
	int result = 0;
	threadsPool pool;
	int capacity = pool.getCntThreads();
	cout << capacity << " treads are avaible" << endl;
	cout <<" Input size of massives:";
	int size = 0;
	cin >> size;
	cout << endl;
	for (int i = 0; i < n; i++)
	{
		int* mas = new int[size];
		int res = 0;
		for (int j = 0; j < size; j++)
		{
			mas[j] = modernRand(10);
			cout << mas[j] << ' ';
		}
		cout << endl;
		pool.addToQueue(bind(sum4, mas, size, std::ref(res)));
		//this_thread::sleep_for(std::chrono::microseconds(10));
		result += res;
	}
	return result;
}

int main() {
	//int mas[10] = {1,2,3,4,5,6,7,8,9,10};
	//int size = 10;
	//int res = 0;
	//threadsPool pool;
	//cout << pool.getCntThreads() << endl;
	////cout << sum4(mas, size);
	//pool.addToQueue(bind(sum4, mas, size, std::ref(res)));
	bool run = true;
	while (run) {
		cout << "0 -  escape"<<endl;
		cout << "Input capacity of massive:";
		int n;
		cin >> n;
		cout << endl;
		if (n==0)
			run = false;
		else
		{
			if (n < 0)
				cout << "Error" << endl;
			else {
				cout<<divide(n)<<endl;
			}
		}
	}

	//pool.addToQueue(print);
	//pool.addToQueue(print2);
	
}