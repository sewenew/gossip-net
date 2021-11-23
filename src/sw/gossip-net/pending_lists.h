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

#ifndef SW_GOSSIP_NET_PENDING_LISTS_H
#define SW_GOSSIP_NET_PENDING_LISTS_H

#include <chrono>
#include <map>
#include <string>
#include <unordered_map>

namespace sw::gossip {

template <typename Item>
class PendingLists {
public:
    explicit PendingLists(const std::chrono::milliseconds &timeout) : _timeout(timeout) {}

    bool add(const std::string &key, Item item) {
        if (_item_index.emplace(key, std::move(item)).second) {
            _timeout_index.emplace(_now(), key);
            return true;
        }

        return false;
    }

    bool remove(const std::string &key) {
        return _item_index.erase(key) == 1;
    }

    std::vector<Item> timeout_items() {
        auto last = _timeout_index.upper_bound(_now());
        std::vector<Item> items;
        for (auto first = _timeout_index.begin(); first != last; ++first) {
            auto [timeout, key] = std::move(*first);
            auto iter = _item_index.find(key);
            if (iter != _item_index.end()) {
                items.push_back(std::move(iter->second));
                _item_index.erase(iter);
            } // else already removed
        }

        _timeout_index.erase(_timeout_index.begin(), last);

        return items;
    }

private:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock,
                                  std::chrono::milliseconds>;

    TimePoint _now() const {
        auto cur = std::chrono::steady_clock::now();

        return std::chrono::time_point_cast<std::chrono::milliseconds>(cur);
    }

    std::unordered_map<std::string, Item> _item_index;

    std::multimap<TimePoint, std::string> _timeout_index;

    std::chrono::milliseconds _timeout;
};

}

#endif // end SW_GOSSIP_NET_PENDING_LISTS_H
