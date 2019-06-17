#pragma once
#include "ISubsystem.h"

class Task final
{
public:
	typedef std::function<void()> Process;

public:
	Task(Process&& process)
	{
		this->process = std::forward<Process>(process);
	}
	~Task() = default;

	Task(const Task&) = delete;
	Task& operator=(const Task&) = delete;

	void Execute()
	{
		process();
	}

private:
	Process process;
};

class Thread final : public ISubsystem
{
public:
	Thread(class Context* context);
	~Thread();

	Thread(const Thread&) = delete;
	Thread& operator= (const Thread&) = delete;

	const bool Initialize() override;

	void Invoke();

	template <typename Process>
	void AddTask(Process&& process);
private:
	std::vector<std::thread> threads;
	std::queue<std::shared_ptr<Task>> tasks;
	std::mutex taskMutex;
	std::condition_variable conditionVar;
	uint threadCount;
	bool bStopping;
};

template<typename Process>
inline void Thread::AddTask(Process && process)
{
	if (threads.empty())
	{
		LOG_WARNING("Thread::AddTask: No available threads");
		process();
		return;
	}

	std::unique_lock<std::mutex> lock(taskMutex);

	tasks.push(std::make_shared<Task>(std::bind(std::forward<Process>(process))));

	lock.unlock();

	conditionVar.notify_one();
}
