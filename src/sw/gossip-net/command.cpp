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

#include "command.h"
#include "errors.h"
#include "gossip_net.h"

namespace sw::gossip {

void Command::run(const RespRequest::Args &args) {
    try {
        auto rumors = _run(args, _net);

        _net.update(rumors);
    } catch (const Error &err) {
        // TODO: reply with error
    }
}

std::vector<Node> PingCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    net.ack(cmd_args.self);

    return std::move(cmd_args.rumors);
}

PingCommand::Args PingCommand::_parse_args(const RespRequest::Args &args) const {
    Args cmd_args;

    auto first = args.begin();
    auto last = args.end();
    std::tie(cmd_args.self, first) = utils::parse_node("self", first, last);

    cmd_args.rumors = utils::parse_rumors(first, last);

    return cmd_args;
}

std::vector<Node> PingReqCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    net.ping_req(cmd_args.peer);

    return std::move(cmd_args.rumors);
}

PingReqCommand::Args PingReqCommand::_parse_args(const RespRequest::Args &args) const {
    Args cmd_args;

    auto first = args.begin();
    auto last = args.end();
    std::tie(cmd_args.self, first) = utils::parse_node("self", first, last);

    std::tie(cmd_args.peer, first) = utils::parse_node("peer", first, last);

    cmd_args.rumors = utils::parse_rumors(first, last);

    return cmd_args;
}

std::vector<Node> AckCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    net.ping_ack(cmd_args.self);

    return std::move(cmd_args.rumors);
}

AckCommand::Args AckCommand::_parse_args(const RespRequest::Args &args) const {
    Args cmd_args;

    auto first = args.begin();
    auto last = args.end();
    std::tie(cmd_args.self, first) = utils::parse_node("self", first, last);

    cmd_args.rumors = utils::parse_rumors(first, last);

    return cmd_args;
}

}
