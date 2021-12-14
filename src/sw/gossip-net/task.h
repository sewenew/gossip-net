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

#ifndef SW_GOSSIP_NET_TASK_H
#define SW_GOSSIP_NET_TASK_H

#include <memory>
#include "utils.h"

namespace sw::gossip {

enum class TaskType {
    PING = 0,
    PING_REQ
    SUSPECTED,
    PASSIVE_PING
};

class Task {
public:
    explicit Task(const std::string &id) : _id(id) {}

    virtual ~Task() = default;

    virtual std::unique_ptr<Task> run() const = 0;

    const std::string& id() const {
        return _id;
    }

private:
    std::string _id;
};

using TaskUPtr = std::unique_ptr<Task>;

class PingReqTask : public Task {
public:
    PingReqTask(const std::string &id, const Node &node, GossipNet &net);

    virtual std::unique_ptr<Task> run() const override;

private:
    Node _dest;

    Node _peer;

    GossipNet &_net;
};

}

#endif // end SW_GOSSIP_NET_TASK_H
