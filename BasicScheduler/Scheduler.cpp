#include "Scheduler.h"
#include <thread>
#include <chrono>

#include <iostream>

Scheduler::Scheduler(unsigned int maxThreads) : MAX_THREADS(maxThreads)
{
}

void Scheduler::AddTask(const std::function<void()>& task, int seconds)
{
	TaskData newTask(task, seconds);

	mtx.lock();
	pendingTasks.push(newTask);
	mtx.unlock();

	// Can be directly load to an available thread 
	if (AreAvailableThreads())
	{
		auto newThread = std::thread(&Scheduler::ExecuteTasks, this);
		threadPool[newThread.get_id()] = std::move(newThread);
	}
	// All threads used and this task must happen before already loaded tasks
	else if(IsHighPriority(newTask))
	{
		mtx.lock();
		// Unload lowest priority task (return to pending)
		auto lowPriorityTask = *loadedTasks.rbegin();
		pendingTasks.push(lowPriorityTask);
		std::cout << "FORCE delete task" << std::endl;
		loadedTasks.erase(*loadedTasks.rbegin());

		// Erase the thread associated to unloaded task
		threadPool[lowPriorityTask.executionThreadId].~thread();			// TODO: Find a way to delete thread or prevent invoke
		threadPool.erase(lowPriorityTask.executionThreadId);
		mtx.unlock();

		// Open new thread (which will load the highest priority task)
		auto newThread = std::thread(&Scheduler::ExecuteTasks, this);
		threadPool[newThread.get_id()] = std::move(newThread);
	}
}

void Scheduler::ExecuteTasks()
{
	mtx.lock();
	unsigned int pendingTasksAmmount = pendingTasks.size();
	mtx.unlock();

	while (pendingTasksAmmount > 0)
	{
		mtx.lock();
		// Load next task
		auto newTask = pendingTasks.top();
		newTask.executionThreadId = std::this_thread::get_id();
		loadedTasks.insert(std::move(newTask));
		pendingTasks.pop();
		std::cout << "Task loaded ( L " << loadedTasks.size() << " + P " << pendingTasks.size() << " )" << std::endl;
		mtx.unlock();

		// Wait for specified time and execute
		long timeDelay = difftime(newTask.scheduledTime, time(0));
		if (timeDelay > 0.f)
		{
			std::this_thread::sleep_for(std::chrono::seconds(timeDelay));
		}
		
		std::cout << "Start Task" << std::endl;
		std::invoke(newTask.task);
		

		// Delete completed task
		mtx.lock();
		loadedTasks.erase(loadedTasks.begin());
		std::cout << "Erased Task ( L " << loadedTasks.size() << " + P " << pendingTasks.size() << " )" << std::endl;
		pendingTasksAmmount = pendingTasks.size();
		mtx.unlock();
	}

	std::cout << "DESTROY thread" << std::endl;
}

bool Scheduler::AreAvailableThreads()
{
	return threadPool.size() < MAX_THREADS;
}

bool Scheduler::IsHighPriority(const TaskData& newTask)
{

	if (loadedTasks.size() < MAX_THREADS)
	{
		// Even if all threads are created, the tasks could not been assigned yet, 
		// so we can add it to pending and let the thread pick it up from pending 
		return false;
	}

	mtx.lock();
	time_t targetTime = loadedTasks.begin()->scheduledTime;
	mtx.unlock();
	return newTask.scheduledTime < targetTime;
}

void Scheduler::Clear()
{
	for (auto& threadContainer : threadPool)
	{
		threadContainer.second.join();
	}

	threadPool.clear();
}