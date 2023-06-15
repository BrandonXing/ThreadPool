// SparrowThreadPool.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <iostream>

#include <list>
#include <vector>

class Task
{
public:
	virtual void doIt()
	{
		std::cout << "Handle a task..." << std::endl;
	}

	virtual ~Task()
	{
		std::cout << "Destruct a task..." << std::endl;
	}
};

class TaskPool
{
public:
	TaskPool();
	~TaskPool();
	TaskPool(const TaskPool& rhs) = delete;
	TaskPool& operator=(const TaskPool& rhs) = delete;
public:
	void init(int numThread = 5);
	void stop();

	void addTask(Task* task);
	void removeAllTask();
private:
	void threaFunc();
private:
	std::list<std::shared_ptr<Task>>			m_taskList;
	std::mutex									m_mutexList;
	std::condition_variable						m_cv;
	std::vector<std::shared_ptr<std::thread>>	m_threads;
	bool										m_bRunning;

};

inline TaskPool::TaskPool():m_bRunning(false)
{
	
}

inline TaskPool::~TaskPool()
{
	removeAllTask();
}

inline void TaskPool::init(int numThread)
{
	if (numThread <= 0)
		numThread = 5;

	m_bRunning = true;

	for(int i = 0;i<numThread;++i)
	{
		std::shared_ptr<std::thread> spThred;
		spThred.reset(new std::thread(std::bind(&TaskPool::threaFunc, this)));
		m_threads.push_back(spThred);
	}

}

inline void TaskPool::threaFunc()
{
	std::shared_ptr<Task> spTask;
	while(true)
	{
		{
			std::unique_lock<std::mutex> guard(m_mutexList);
			while (m_taskList.empty())
			{
				if(!m_bRunning)
					break;

				m_cv.wait(guard);
			}

			if (!m_bRunning)
				break;

			spTask = m_taskList.front();
			m_taskList.pop_front();
		}

		if (spTask == nullptr)
			continue;

		spTask->doIt();
		spTask.reset();
	}

	std::cout << "exit thread, threadID:" << std::this_thread::get_id() << std::endl;

}

inline void TaskPool::stop()
{
	m_bRunning = false;
	m_cv.notify_all();

	for(auto& iter_thread:m_threads)
	{
		if (iter_thread->joinable())
			iter_thread->join();
	}
}

inline void TaskPool::addTask(Task* task)
{
	std::shared_ptr<Task> spTask;
	spTask.reset(task);

	{
		std::lock_guard<std::mutex> guard(m_mutexList);
		m_taskList.push_back(spTask);
		std::cout << "Add a task." << std::endl;
	}

	m_cv.notify_all();
}

inline void TaskPool::removeAllTask()
{
	{
		std::lock_guard<std::mutex> guard(m_mutexList);
		for (auto& iter : m_taskList)
			iter.reset();

		m_taskList.clear();
	}
}
