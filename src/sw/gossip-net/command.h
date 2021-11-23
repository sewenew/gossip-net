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

#ifndef SW_GOSSIP_NET_COMMAND_H
#define SW_GOSSIP_NET_COMMAND_H

#include "resp.h"
#include "utils.h"

namespace sw::gossip {

class GossipNet;

class Command {
public:
    explicit Command(const std::string &name, GossipNet &net) :
        _name(name), _net(net) {}

    virtual ~Command() = default;

    void run(const RespRequest::Args &args);

    const std::string& name() const {
        return _name;
    }

private:
    virtual void _run(const RespRequest::Args &args, GossipNet &net) = 0;

    std::string _name;

    GossipNet &_net;
};

using CommandUPtr = std::unique_ptr<Command>;

class PingCommand : public Command {
public:
    explicit PingCommand(GossipNet &net) : Command("ping", net) {}

private:
    virtual std::vector<Node> _run(const RespRequest::Args &args, GossipNet &net) override;

    struct Args {
        Node self;
        std::vector<Node> rumors;
    };

    Args _parse_args(const RespRequest::Args &args) const;
};

class PingReqCommand : public Command {
public:
    explicit PingReqCommand(GossipNet &net) : Command("ping-req", net) {}

private:
    virtual std::vector<Node> _run(const RespRequest::Args &args, GossipNet &net) override;

    struct Args {
        Node self;
        Node peer;
        std::vector<Node> rumors;
    };

    Args _parse_args(const RespRequest::Args &args) const;
};

class AckCommand : public Command {
public:
    explicit AckCommand(GossipNet &net) : Command("ack", net) {}

private:
    virtual std::vector<Node> _run(const RespRequest::Args &args, GossipNet &net) override;

    struct Args {
        Node self;
        std::vector<Node> rumors;
    };

    Args _parse_args(const RespRequest::Args &args) const;
};

}

#endif // end SW_GOSSIP_NET_COMMAND_H
