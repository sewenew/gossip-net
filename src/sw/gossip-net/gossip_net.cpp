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
#include "command.h"

namespace sw::gossip {

GossipNet::GossipNet(const GossipNetOptions &opts) : _server(opts.server_options) {
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

void GossipNet::update(const std::vector<Node> &rumors) {
}

void GossipNet::ping_req(const Node &node) {
}

void GossipNet::ack(const Node &node) {
    auto rumors = _build_rumors();

    RespReplyBuilder builder;
    builder.append_array(1 + 5 + 6 * rumors.size());

    _server.send(node.ip, node.port, );
}

void GossipNet::ping(const Node &node) {
}

void GossipNet::ping_ack(const Node &node) {
}

std::vector<Node> GossipNet::_build_rumors() {
}

}
