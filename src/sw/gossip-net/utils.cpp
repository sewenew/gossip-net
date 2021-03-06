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

#include "utils.h"

namespace sw::gossip {

bool operator<(const Node &lhs, const Node &rhs) {
    assert(node.id == id);

    switch (rhs.status) {
    case NodeStatus::ALIVE:
        return (rhs.version > lhs.version) &&
            (lhs.status == NodeStatus::SUSPECTED || lhs.status == NodeStatus::ALIVE);

    case NodeStatus::SUSPECTED:
        return (lhs.status == NodeStatus::SUSPECTED && rhs.version > lhs.version) ||
            (lhs.status == ALIVE && rhs.version >= version);

    case NodeStatus::FAILED:
        return lhs.status == NodeStatus::ALIVE || lhs.status == NodeStatus::SUSPECTED;

    default:
        assert(false);
    }

    // In fact, never goes here.
    return false;
}

namespace utils {

NodeStatus parse_status(const std::string_view &sv) {
    if (sv == ALIVE) {
        return NodeStatus::ALIVE;
    } else if (sv == SUSPECTED) {
        return NodeStatus::SUSPECTED;
    } else if (sv == FAILED) {
        return NodeStatus::FAILED;
    } else {
        return NodeStatus::UNKNOWN;
    }
}

}

}
