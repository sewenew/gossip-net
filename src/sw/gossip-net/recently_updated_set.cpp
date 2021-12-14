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

#include "recently_updated_set.h"
#include <cassert>
#include <queue>

namespace sw::gossip {

void RecentlyUpdatedSet::add(const Node &node) {
    const auto &id = node.id;
    auto iter = _members.find(id);
    if (iter != _members.end()) {
        // If we should update the info, reset the counter.
        auto &mem = iter->second;
        if (mem.node < node) {
            mem.node = node;
            mem.counter = 0;
        }
    } else {
        _members[id].node = node;
    }
}

auto RecentlyUpdatedSet::fetch(std::size_t n, std::size_t max_spreaded_num)
    -> std::pair<std::vector<Node>, std::vector<Node>> {
    if (n == 0) {
        return {};
    }

    if (n >= _members.size()) {
        return _fetch_all(n, max_spreaded_num);
    }

    return _fetch(n, max_spreaded_num);
}

auto RecentlyUpdatedSet::_fetch(std::size_t n, std::size_t max_spreaded_num)
    -> std::pair<std::vector<Node>, std::vector<Node>> {
    std::priority_queue<MemberIter, std::vector<MemberIter>, MemberCmp> que;
    for (auto iter = _members.begin(); iter != _members.end(); ++iter) {
        que.push(iter);
    }

    std::vector<Node> recent_nodes;
    recent_nodes.reserve(n);
    std::vector<Node> stable_nodes;
    stable_nodes.reserve(n);

    assert(n > 0 && n < _members.size());

    while (!que.empty()) {
        auto &iter = que.top();
        auto &member = iter->second;
        ++member.counter;
        if (member.counter > max_spreaded_num) {
            // Member has been spreaded many times, make it stable, and no more spreading.
            if (member.node.status != NodeStatus::FAILED) {
                stable_nodes.push_back(std::move(member.node));
            } // Simply remove FAILED node.

            _members.erase(iter);
        } else {
            recent_nodes.push_back(member.node);
            if (recent_nodes.size() == n) {
                break;
            }
        }
        que.pop();
    }

    return std::make_pair(std::move(recent_nodes), std::move(stable_nodes));
}

auto RecentlyUpdatedSet::_fetch_all(std::size_t max_spreaded_num)
    -> std::pair<std::vector<Node>, std::vector<Node>> {
    std::vector<Node> recent_nodes;
    recent_nodes.reserve(_members.size());
    std::vector<Node> stable_nodes;
    stable_nodes.reserve(_members.size());
    for (auto iter = _members.begin(); iter != _members.end(); ) {
        auto &member = iter->second;
        ++member.counter;
        if (member.counter > max_spreaded_num) {
            // Member has been spreaded many times, make it stable, and no more spreading.
            if (member.node.status != NodeStatus::FAILED) {
                stable_nodes.push_back(std::move(member.node));
            } // else simply remove FAILED node

            iter = _members.erase(iter);
        } else {
            recent_nodes.push_back(member.node);
            ++iter;
        }
    }

    return std::make_pair(std::move(recent_nodes), std::move(stable_nodes));
}

}
