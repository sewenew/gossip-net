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

#ifndef SW_GOSSIP_NET_UTILS_H
#define SW_GOSSIP_NET_UTILS_H

#include <charconv>
#include <iterator>
#include <string>
#include <vector>
#include <string_view>
#include "errors.h"

namespace sw::gossip {

enum class NodeStatus {
    ALIVE = 0,
    SUSPECTED,
    FAILED,
    UNKNOWN
};

struct Node {
    std::string id;
    std::string ip;
    int port;
    uint64_t version = 0;
    NodeStatus status = NodeStatus::ALIVE;
};

bool operator<(const Node &lhs, const Node &rhs);

namespace utils {

constexpr auto *ALIVE = "ALIVE";
constexpr auto *SUSPECTED = "SUSPECTED";
constexpr auto *FAILED = "FAILED";

inline void to_str(const std::string_view &sv, std::string &s) {
    s.assign(sv.data(), sv.size());
}

template <typename Num>
inline void to_num(const std::string_view &sv, Num &num) {
    auto [ptr, err] = std::from_chars(sv.data(), sv.data() + sv.size(), num);
    if (err != std::errc()) {
        throw Error("expect a number");
    }
}

NodeStatus parse_status(const std::string_view &sv);

template <typename T>
auto parse_node(const std::string_view &type, T first, T last) {
    auto dist = std::distance(first, last);
    if (dist < 5) {
        throw Error("invalid node info");
    }

    if (*first++ != type) {
        throw Error("invalid node type");
    }

    Node node;
    to_str(*first++, node.id);
    to_str(*first++, node.ip);
    to_num(*first++, node.port);
    to_num(*first++, node.version);

    if (dist > 5) {
        auto status = parse_status(*(first + 1));
        if (status != NodeStatus::UNKNOWN) {
            node.status = status;
        } else {
            ++first;
        }
    }

    return std::make_pair(node, first);
}

template <typename T>
auto parse_rumors(T first, T last) {
    std::vector<Node> rumors;
    while (first != last) {
        Node node;
        std::tie(node, first) = parse_node("rumor", first, last);
        rumors.push_back(std::move(node));
    }

    return rumors;
}

}

}

#endif // end SW_GOSSIP_NET_UTILS_H
