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

#ifndef SW_GOSSIP_NET_PENDING_LISTS_H
#define SW_GOSSIP_NET_PENDING_LISTS_H

#include <chrono>
#include <map>
#include <string>
#include <unordered_map>
#include "task.h"

namespace sw::gossip {

class PendingLists {
public:
    explicit PendingLists(const std::chrono::milliseconds &timeout) : _timeout(timeout) {}

    void add(TaskUPtr task);

    std::vector<TaskUPtr> fetch(const std::string &id);

    std::vector<Item> timeout_tasks();

private:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock,
                                  std::chrono::milliseconds>;

    TimePoint _now() const;

    std::unordered_map<std::string, std::vector<std::pair<TaskUPtr, TimePoint>>> _task_index;

    std::map<TimePoint, std::vector<std::string>> _timeout_index;

    std::chrono::milliseconds _timeout;
};

}

#endif // end SW_GOSSIP_NET_PENDING_LISTS_H
