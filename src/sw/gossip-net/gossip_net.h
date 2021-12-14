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

#ifndef SW_GOSSIP_NET_GOSSIP_NET_H
#define SW_GOSSIP_NET_GOSSIP_NET_H

#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include "udp_server.h"
#include "utils.h"
#include "pending_lists.h"
#include "member_set.h"
#include "recently_updated_set.h"

namespace sw::gossip {

struct GossipNetOptions {
    UdpServerOptions server_options;

    std::size_t lambda;

    std::size_t max_rumor_num;
};

class GossipNet {
public:
    explicit GossipNet(const GossipNetOptions &opts);

    ~GossipNet();

    void start();

    void stop();

    void join(const std::string &ip, int port);

    void update(std::vector<Node> rumors)

    void ping_req(const Node &node);

    void ack(const Node &dest);

    void ack(const Node &dest, const Node &self);

    void ping(const Node &dest);

    void add(const std::string &id, std::unique_ptr<Task> task);

    void do_task(const std::string &id);

private:
    void _append_node(RespReplyBuilder &builder,
            const std::string &type,
            const Node &node,
            bool append_status = true);

    std::vector<Node> build_rumors();

    UdpServer _server;

    Node _self;

    // alive and suspected members including itself.
    MemberSet _members;

    // Members whose states are recently changed to alive or suspected.
    RecentlyUpdatedSet _recently_updated_members;

    GossipNetOptions _opts;

    std::thread _server_thread;

    std::mutex _mtx;

    PendingLists<Task> _tasks;
};

}

#endif // end SW_GOSSIP_NET_GOSSIP_NET_H
