#include <algorithm>
#include "task/ParallelTask.h"
#include "task/TaskPool.h"

TaskPool<ParallelTask> ParallelTask::pool_;

ParallelTask::ParallelTask()
	: Task()
	, std::enable_shared_from_this<ParallelTask>()
	, listTask_()
{
}

ParallelTask::~ParallelTask()
{
}

bool ParallelTask::compute(uint32_t tick)
{
	bool result = true;
	std::for_each(listTask_.begin(), listTask_.end(), [&result, tick](std::shared_ptr<Task> animator){
		result = result && animator->compute(tick);
	});
	return result;
}

void ParallelTask::stop(bool isFinishAction)
{
	std::for_each(listTask_.begin(), listTask_.end(), [isFinishAction](std::shared_ptr<Task> animator){
		animator->stop(isFinishAction);
	});
}

bool ParallelTask::isFinished()
{
	return std::any_of(listTask_.begin(), listTask_.end(), [](std::shared_ptr<Task> animator) {
		return animator->isFinished();
	});
}

void ParallelTask::reset(uint32_t tick)
{
	std::for_each(listTask_.begin(), listTask_.end(), [tick](std::shared_ptr<Task> animator){
		animator->reset(tick);
	});
}

void ParallelTask::collect()
{
	std::for_each(listTask_.begin(), listTask_.end(), [](std::shared_ptr<Task> animator){
		animator->collect();
	});
	listTask_.clear();
	pool_.collect(shared_from_this());
}

void ParallelTask::addTask(std::shared_ptr<Task> task)
{
	listTask_.push_back(task);
}

