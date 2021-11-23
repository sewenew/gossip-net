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

#ifndef SW_GOSSIP_NET_UV_UTILS_H
#define SW_GOSSIP_NET_UV_UTILS_H

#include <type_traits>
#include <memory>
#include <chrono>
#include <string>
#include <variant>
#include <uv.h>
#include "errors.h"

namespace sw::gossip {
    
class UvError : public Error {
public:
    UvError(int err, const std::string &msg);
};

struct LoopDeleter {
    void operator()(uv_loop_t *loop) const;
};

using LoopUPtr = std::unique_ptr<uv_loop_t, LoopDeleter>;

using AsyncUPtr = std::unique_ptr<uv_async_t>;

using TcpUPtr = std::unique_ptr<uv_tcp_t>;

using UdpUPtr = std::unique_ptr<uv_udp_t>;

using WriteUPtr = std::unique_ptr<uv_write_t>;

using TimerUPtr = std::unique_ptr<uv_timer_t>;

struct TcpOptions {
    std::string ip;
    int port;
    int backlog;
    std::chrono::seconds keepalive;
    bool nodelay;
};

struct UdpOptions {
    std::string ip;
    int port;
};

class SockAddr {
public:
    SockAddr(const std::string &ip, int port);

    const sockaddr* addr() const;

private:
    std::variant<sockaddr_in, sockaddr_in6> _addr;
};

namespace uv {

template <typename Handle>
inline uv_handle_t* to_handle(Handle *handle) noexcept {
    static_assert(std::is_same_v<Handle, uv_handle_t> ||
            std::is_same_v<Handle, uv_stream_t> ||
            std::is_same_v<Handle, uv_tcp_t> ||
            std::is_same_v<Handle, uv_udp_t> ||
            std::is_same_v<Handle, uv_async_t>);
    return reinterpret_cast<uv_handle_t *>(handle);
}

template <typename Stream>
inline uv_stream_t* to_stream(Stream *stream) noexcept {
    static_assert(std::is_same_v<Stream, uv_stream_t> ||
            std::is_same_v<Stream, uv_tcp_t>);
    return reinterpret_cast<uv_stream_t*>(stream);
}

template <typename Ptr>
inline void set_data(Ptr *ptr, void *data) noexcept {
    assert(ptr != nullptr);
    ptr->data = data;
}

template <typename Result, typename Ptr>
inline Result* get_data(Ptr *ptr) noexcept {
    assert(ptr != nullptr);
    auto *data = ptr->data;
    return static_cast<Result*>(data);
}

template <typename Handle>
inline void handle_close(Handle *handle, uv_close_cb callback) noexcept {
    uv_close(to_handle(handle), callback);
}

inline std::string err_msg(int err) {
    std::string name = uv_err_name(err);
    std::string msg = uv_strerror(err);

    return name + ": " + msg;
}

LoopUPtr make_loop();

AsyncUPtr make_async(uv_loop_t &loop, uv_async_cb callback, void *data = nullptr);

TimerUPtr make_timer(uv_loop_t &loop,
        uv_timer_cb callback,
        const std::chrono::milliseconds &timeout,
        const std::chrono::milliseconds &repeat,
        void *data = nullptr);

TcpUPtr make_tcp_server(uv_loop_t &loop,
            const TcpOptions &options,
            uv_connection_cb on_connect,
            void *data);

TcpUPtr make_tcp_client(uv_loop_t &loop, void *data = nullptr);

WriteUPtr make_write(uv_loop_t &loop, void *data = nullptr);

UdpUPtr make_udp_server(uv_loop_t &loop,
        const UdpOptions &options,
        void *data = nullptr);

}

}

#endif // end SW_GOSSIP_NET_UV_UTILS_H
