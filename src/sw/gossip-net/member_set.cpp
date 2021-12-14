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

#include "member_set.h"
#include <random>

namespace sw::gossip {

MemberSet::MemberSet() :
    _iter(_members.end()),
    _random_key_prefix(_gen_random_key_prefix()) {
}

std::optional<Node> MemberSet::try_update(Node node) {
    auto key = _build_key(node.id);

    auto iter = _members.find(key);
    if (iter == _members.end()) {
        return std::optional<Node>(std::move(node));
    }

    auto &member = iter->second;
    if (member < node) {
        // Should update node info
        // Remove it from stable member set.
        if (iter == _iter) {
            _iter = _members.erase(iter);
        } else {
            _members.erase(iter);
        }

        return std::optional<Node>(std::move(node));
    }

    // No need to update node info.
    return std::nullopt;
}

void MemberSet::add(Node node) {
    auto key = _build_key(node.id);

    assert(_members.find(key) == _members.end());

    _members.emplace(key, std::move(node));
}

std::vector<Node> MemberSet::fetch(std::size_t num) {
    if (num > _members.size()) {
        return _fetch_all();
    } else {
        return _fetch(num);
    }
}

std::vector<Node> MemberSet::_fetch_all() {
    std::vector<Node> result;
    result.reserve(_members.size());
    for (const auto &ele : _members) {
        result.push_back(ele.second);
    }

    return result;
}

std::vector<Node> MemberSet::_fetch(std::size_t num) {
    if (num == 0) {
        return {};
    }

    assert(num < _members.size());

    std::vector<Node> result;
    result.reserve(num);
    for (auto idx = 0U; idx != num; ++idx) {
        if (_iter == _members.end()) {
            _iter = _members.begin();
        }

        result.push_back(_iter->second);
        ++_iter;
    }

    return result;
}

std::string MemberSet::_gen_random_key_prefix() {
    std::random_device rd;

    return std::to_string(rd());
}

}
