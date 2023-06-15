// SparrowThreadPool.cpp: 定义应用程序的入口点。
//
#include <chrono>
#include "SparrowThreadPool.h"

using namespace std;

int main()
{
	TaskPool threadpool;
	threadpool.init();

	Task* task = nullptr;
	for(int i=0;i<10;++i)
	{
		task = new Task();
		threadpool.addTask(task);
	}

	std::this_thread::sleep_for(std::chrono::seconds(5));

	threadpool.stop();

	return 0;
}
