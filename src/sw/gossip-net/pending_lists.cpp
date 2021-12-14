/**************************************************************************
   Copyright (c) 2021 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#include "pending_lists.h"

namespace sw::gossip {

void PendingLists::add(TaskUPtr task) {
    assert(task);

    auto cur = _now();
    auto id = task->id();
    _task_index[id].emplace_back(std::move(task), cur);
    _timeout_index[cur].push_back(id);
}

std::vector<TaskUPtr> PendingLists::fetch(const std::string &id) {
    auto iter = _task_index.find(id);
    if (iter == _task_index.end()) {
        return {};
    }

    auto &tasks = iter->second;
    std::vector<TaskUPtr> results;
    results.reserve(tasks.size());
    for (auto &task : tasks) {
        results.push_back(std::move(task.first));
    }

    return results;
}

std::vector<TaskUPtr> PendingLists::timeout_tasks() {
    auto cur = _now();
    auto last = _timeout_index.upper_bound(cur);
    std::vector<TaskUPtr> results;
    for (auto iter = _timeout_index.begin(); iter != last; ++iter) {
        for (const auto &id : iter->second) {
            auto it = _task_index.find(id);
            if (it != _task_index.end()) {
                auto &tasks = it->second;
                for (auto i = tasks.begin(); i != tasks.end(); ) {
                    if (i->second < cur) {
                        results.push_back(std::move(i->first));
                        i = tasks.erase(i);
                    } else {
                        ++i;
                    }
                }

                if (tasks.empty()) {
                    _task_index.erase(it);
                }
            } // else already removed
        }
    }

    _timeout_index.erase(_timeout_index.begin(), last);

    return results;
}

TimePoint PendingLists::_now() const {
    auto cur = std::chrono::steady_clock::now();

    return std::chrono::time_point_cast<std::chrono::milliseconds>(cur);
}

}
