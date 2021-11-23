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

namespace sw::gossip {

struct GossipNetOptions {
    UdpServerOptions server_options;
};

class GossipNet {
public:
    explicit GossipNet(const GossipNetOptions &opts);

    ~GossipNet();

    void start();

    void stop();

    void join(const std::string &ip, int port);

    void update(const std::vector<Node> &rumors);

    void ping_req(const Node &node);

    void ack(const Node &node);

    void ping(const Node &node);

    void ping_ack(const Node &node);

private:
    std::vector<Node> build_rumors();

    UdpServer _server;

    Node self;

    std::unordered_map<std::string, Node> _members;

    std::thread _server_thread;

    std::mutex _mtx;

    PendingLists<Node> _pending_pings;
    PendingLists<Node> _pending_ping_reqs;
    PendingLists<Node> _pending_passive_pings;
};

}

#endif // end SW_GOSSIP_NET_GOSSIP_NET_H
