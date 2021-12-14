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

#ifndef SW_GOSSIP_NET_RECENTLY_UPDATED_SET
#define SW_GOSSIP_NET_RECENTLY_UPDATED_SET

#include <unordered_map>
#include <string>
#include "utils.h"

namespace sw::gossip {

class RecentlyUpdatedSet {
public:
    RecentlyUpdatedSet() = default;

    void add(const Node &node);

    // Fetch N recently updated members, and increase its counter.
    // Also returns members that already been spreaded at least `max_spreaded_num` times,
    // i.e. the stable nodes.
    auto fetch(std::size_t n, std::size_t max_spreaded_num)
        -> std::pair<std::vector<Node>, std::vector<Node>>;

private:
    auto _fetch_all(std::size_t n, std::size_t max_spreaded_num)
        -> std::pair<std::vector<Node>, std::vector<Node>>;

    auto _fetch(std::size_t max_spreaded_num)
        -> std::pair<std::vector<Node>, std::vector<Node>>;

    struct Member {
        Node node;

        // Count of times that the node has been fetched (spreaded).
        std::size_t counter = 0;
    };

    std::unordered_map<std::string, Member> _members;

    using MemberIter = std::unordered_map<std::string, Member>::iterator;

    struct MemberCmp {
        bool operator()(const MemberIter &lhs, const MemberIter &rhs) const {
            return rhs->second.counter < lhs->second.counter;
        }
    };

};

}

#endif // end SW_GOSSIP_NET_RECENTLY_UPDATED_SET
