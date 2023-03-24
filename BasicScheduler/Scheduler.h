#pragma once
#include <functional>
#include <thread>
#include <mutex> 

#include <time.h>
#include <map>
#include <queue>
#include <set>


struct TaskData
{
	std::function<void()> task;
	time_t scheduledTime;
	std::thread::id executionThreadId;
	

	TaskData(const std::function<void()>& task, int seconds) : task(task)
	{
		scheduledTime = time(0) + seconds;
	}

	TaskData(const TaskData& newTask) : task(newTask.task), scheduledTime(newTask.scheduledTime) {}

	constexpr bool operator()(const TaskData& lhs, const TaskData& rhs) const
	{
		return lhs.scheduledTime > rhs.scheduledTime;
	}
};

struct TaskCompare
{
	bool operator()(const TaskData& lhs, const TaskData& rhs) const
	{
		return lhs.scheduledTime > rhs.scheduledTime;
	}
};

class Scheduler
{

public:
	Scheduler(unsigned int maxThreads = 2);

	void AddTask(const std::function<void()>& task, int seconds);
	void Clear();

private:

	void ExecuteTasks();
	bool AreAvailableThreads();
	bool IsHighPriority(const TaskData& newTask);


	const unsigned int MAX_THREADS;
	std::mutex mtx;

	std::map<std::thread::id,std::thread> threadPool;
	std::priority_queue<TaskData, std::vector<TaskData>, TaskCompare> pendingTasks;
	std::multiset<TaskData, TaskCompare> loadedTasks;
};

