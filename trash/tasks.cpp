#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

enum class TaskStatus {
    NEW,
    IN_PROGRESS,
    TESTING,
    DONE
};

using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
   public:
    const TasksInfo& GetPersonTasksInfo(const string& person) const {
        return person_tasks_.at(person);
    }

    void AddNewTask(const string& person) {
        person_tasks_[person][TaskStatus::NEW]++;
    }

    tuple<TasksInfo, TasksInfo> PerformPersonTasks(const string& person, int task_count) {
        TasksInfo updated_tasks, untouched_tasks;

        TasksInfo& tasks = person_tasks_[person];
        for (TaskStatus status = TaskStatus::NEW; status != TaskStatus::DONE;
             status = static_cast<TaskStatus>(static_cast<int>(status) + 1)) {
            updated_tasks[static_cast<TaskStatus>(static_cast<int>(status) + 1)] = min(task_count, tasks[status]);
            if (task_count > tasks[status]) {
                task_count -= tasks[status];
            } else {
                untouched_tasks[status] = tasks[status] - task_count;
                task_count = 0;
            }
        }

        for (TaskStatus status = TaskStatus::NEW; status != TaskStatus::DONE;
             status = static_cast<TaskStatus>(static_cast<int>(status) + 1)) {
            tasks[status] -= updated_tasks[static_cast<TaskStatus>(static_cast<int>(status) + 1)];
            tasks[static_cast<TaskStatus>(static_cast<int>(status) + 1)] += updated_tasks[static_cast<TaskStatus>(static_cast<int>(status) + 1)];
        }

        RemoveEmptyStatuses(updated_tasks);
        RemoveEmptyStatuses(untouched_tasks);

        return {updated_tasks, untouched_tasks};
    }

   private:
    static void RemoveEmptyStatuses(TasksInfo& tasks) {
        vector<TaskStatus> empty_statuses;
        for (auto& [status, count] : tasks) {
            if (count == 0) {
                empty_statuses.push_back(status);
            }
        }
        for (auto status : empty_statuses) {
            tasks.erase(status);
        }
    }

    map<string, TasksInfo> person_tasks_;
};
