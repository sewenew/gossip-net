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
        _run(args, _net);
    } catch (const Error &err) {
        // TODO: reply with error
    }
}

// ping self id ip port version [rumor id ip port version status]
void PingCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    cmd_args.rumors.push_back(cmd_args.self);
    net.update(cmd_args.rumors);

    net.ack(cmd_args.self);
}

PingCommand::Args PingCommand::_parse_args(const RespRequest::Args &args) const {
    Args cmd_args;

    auto first = args.begin();
    auto last = args.end();
    std::tie(cmd_args.self, first) = utils::parse_node("self", first, last);

    cmd_args.rumors = utils::parse_rumors(first, last);

    return cmd_args;
}

// ping_req self id ip port version peer id ip port version [rumor id ip port version status]
void PingReqCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    cmd_args.rumors.push_back(cmd_args.self);
    net.update(cmd_args.rumors);

    // Forward ping.
    net.ping(cmd_args.peer);

    auto task = std::make_unique<PingReqTask>(cmd_args.peer.id, cmd_args.self);
    net.add(std::move(task));
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

// ack self id ip port version [rumor id ip port version status]
void AckCommand::_run(const RespRequest::Args &args, GossipNet &net) {
    auto cmd_args = _parse_args(args);

    cmd_args.rumors.push_back(cmd_args.self);
    net.update(cmd_args.rumors);

    net.do_task(cmd_args.self.id);
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
