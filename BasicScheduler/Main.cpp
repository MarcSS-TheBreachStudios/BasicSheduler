// Written by Marc Solis (2022)

#pragma once
#include <iostream>
#include "Scheduler.h"
#include <chrono>


std::chrono::steady_clock::time_point timeStart;

void testFunc1()
{
	std::chrono::duration<float> interval = std::chrono::steady_clock::now() - timeStart;
	std::cout << "Func1 at " << interval.count() << std::endl;
}

void testFunc2()
{
	std::chrono::duration<float> interval = std::chrono::steady_clock::now() - timeStart;
	std::cout << "Func2 at " << interval.count() << std::endl;
}

void testFunc3()
{
	std::chrono::duration<float> interval = std::chrono::steady_clock::now() - timeStart;
	std::cout << "Func3 at " << interval.count() << std::endl;
}

void testFunc4()
{
	std::chrono::duration<float> interval = std::chrono::steady_clock::now() - timeStart;
	std::cout << "Func4 at " << interval.count() << std::endl;
}

int main()
{
	std::cout << "Start program" << std::endl;

	Scheduler scheduler(2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	timeStart = std::chrono::steady_clock::now();

	scheduler.AddTask(testFunc4, 8);
	scheduler.AddTask(testFunc2, 5);
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	scheduler.AddTask(testFunc3, 2);
	scheduler.AddTask(testFunc1, 2);
	
	std::this_thread::sleep_for(std::chrono::seconds(10));

	scheduler.Clear();
	std::cout << "End program" << std::endl;

	return 0;
}