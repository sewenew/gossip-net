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

#include "gossip_net.h"
#include <cmath>
#include "command.h"

namespace sw::gossip {

GossipNet::GossipNet(const GossipNetOptions &opts) :
    _server(opts.server_options),
    _opts(opts) {
    _server.register_command(std::make_unique<PingCommand>(*this));
    _server.register_command(std::make_unique<PingReqCommand>(*this));
    _server.register_command(std::make_unique<AckCommand>(*this));
}

GossipNet::~GossipNet() {
    if (_server_thread.joinable()) {
        _server_thread.join();
    }
}

void GossipNet::start() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_server_thread.joinable()) {
        throw Error("already started");
    }

    _server_thread = std::thread([this]() {
                            _server.start();
                        });
}

void GossipNet::stop() {
    std::lock_guard<std::mutex> lock(_mtx);
    if (_server_thread.joinable()) {
        _server.stop();
        _server_thread.join();
    }
}

void GossipNet::join(const std::string &ip, int port) {
}

void GossipNet::update(std::vector<Node> rumors) {
    for (auto &rumor : rumors) {
        auto node = _members.try_update(std::move(rumor));
        if (node) {
            _recently_updated_members.add(*node);
        }
    }
}

void GossipNet::ping_req(const Node &node) {
}

void GossipNet::ack(const Node &dest) {
    ack(dest, _self);
}

void GossipNet::ack(const Node &dest, const Node &self) {
    auto rumors = _build_rumors();

    RespReplyBuilder builder;
    builder.append_array(1 + 5 + rumors.size() * 6);
    builder.append_simple_string("ack");
    _append_node(builder, "self", self, false);

    for (const auto &rumor : rumors) {
        _append_node(builder, "rumor", rumor);
    }

    _server.send(dest.ip, dest.port, std::move(builder.data()));
}

void GossipNet::ping(const Node &dest) {
    auto rumors = _build_rumors();

    RespReplyBuilder builder;
    builder.append_array(1 + 5 + rumors.size() * 6);
    builder.append_simple_string("ping");

    _append_node(builder, "self", _self, false);

    for (const auto &rumor : rumors) {
        _append_node(builder, "rumor", rumor);
    }

    _server.send(dest.ip, dest.port, std::move(builder.data()));
}

void GossipNet::do_task(const std::string &id) {
    auto tasks = _tasks.fetch(id);
    for (auto &task : tasks) {
        assert(task);

        auto next_task = task.run();
        if (next_task) {
            _tasks.add(std::move(next_task));
        }
    }
}

void GossipNet::_append_node(RespReplyBuilder &builder,
        const std::string &type,
        const Node &node,
        bool append_status) const {
    builder.append_simple_string(type)
        .append_simple_string(node.id)
        .append_simple_string(node.ip)
        .append_simple_string(std::to_string(node.port))
        .append_simple_string(std::to_string(version));

    if (append_status) {
        const char *status = nullptr;
        switch (node.status) {
        case NodeStatus::ALIVE:
            status = utils::ALIVE;
            break;

        case NodeStatus::SUSPECTED:
            status = utils::SUSPECTED;
            break;

        case NodeStatus::FAILED:
            status = utils::FAILED;
            break;

        default:
            throw Error("unknow status");
        }

        builder.append_simple_string(status);
    }
}

void GossipNet::add_task(const std::string &id, std::unique_ptr<Task> task) {
    _tasks.add(id, std::move(task));
}

std::vector<Node> GossipNet::_build_rumors() {
    auto max_rumor_num = _opts.max_rumor_num;

    // Each new member info will be spread max_spreaded_num times before stable.
    auto max_spreaded_num = static_cast<std::size_t>(_opts.lambda * std::log(_members.size()
                + _recently_updated_members.size())) + 1;

    auto [rumors, stable_rumors] = _recently_updated_members.fetch(max_rumor_num, max_spreaded_num);

    for (auto &rumor : stable_rumors) {
        _members.add(std::move(rumor));
    }

    if (rumors.size() == max_rumor_num) {
        return rumors;
    }

    assert(rumors.size() < max_rumor_num);

    auto n = max_rumor_num - rumors.size();
    auto temp = _members.fetch(n);
    rumors.insert(rumors.end(), temp.begin(), temp.end());

    return rumors;
}

}
