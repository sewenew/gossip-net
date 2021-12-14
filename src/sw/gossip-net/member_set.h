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

#ifndef SW_GOSSIP_NET_MEMBER_SET_H
#define SW_GOSSIP_NET_MEMBER_SET_H

#include <unordered_map>
#include <string>
#include <optional>
#include "utils.h"

namespace sw::gossip {

class MemberSet {
public:
    MemberSet();

    // If try_update returns a valid node, add it to recently updated set.
    std::optional<Node> try_update(Node node);

    void add(Node node);

    std::vector<Node> fetch(std::size_t num);

private:
    std::string _gen_random_key_prefix() const;

    std::string _build_key(const std::string &key) const {
        return _random_key_prefix + key;
    }

    std::vector<Node> _fetch_all();

    std::vector<Node> _fetch(std::size_t num);

    using Map = std::unordered_map<std::string, Node>;
    Map _members;

    Map::iterator _iter;

    std::string _random_key_prefix;
};

}

#endif // end SW_GOSSIP_NET_MEMBER_SET_H
