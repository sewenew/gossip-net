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

#ifndef SW_GOSSIP_NET_UDP_SERVER_H
#define SW_GOSSIP_NET_UDP_SERVER_H

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include "uv_utils.h"
#include "resp.h"
#include "command.h"

namespace sw::gossip {

struct UdpServerOptions {
    std::string ip;
    int port;
    std::size_t buffer_size;
};

class UdpServer {
public:
    explicit UdpServer(const UdpServerOptions &opts);

    /*
    template <typename T>
    void register_timer(const std::chrono::milliseconds &timeout,
            const std::chrono::milliseconds &repeat,
            uv_timer_cb callback,
            T *data,
            bool own_data) {
        try {
            auto timer = std::make_unique<>();
        } catch () {
        }
    }
    */

    void register_command(CommandUPtr command);

    void start();

    void stop() {} // TODO: stop the server

    void send(const std::string &ip, int port, std::string data);

private:
    static void _on_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

    static void _on_read(uv_udp_t *req, ssize_t nread,
            const uv_buf_t *buf, const sockaddr *addr, unsigned flags);

    struct SendContext {
        explicit SendContext(std::string req_) : req(std::move(req_)) {
            buf.base = req.data();
            buf.len = req.size();
        }

        std::string req;
        uv_buf_t buf;
    };

    static void _on_send(uv_udp_send_t *req, int status);

    static void _on_event(uv_async_t *handle);

    void _handle(const std::string_view &buf);

    struct Event {
        std::string ip;
        int port;
        std::string data;
    };

    void _send(Event event);

    LoopUPtr _loop;

    UdpUPtr _server;

    AsyncUPtr _async;

    std::vector<char> _buffer;

    std::unordered_map<std::string, CommandUPtr> _commands;

    std::vector<Event> _events;

    std::mutex _mtx;
};

}

#endif // end SW_GOSSIP_NET_UDP_SERVER_H
