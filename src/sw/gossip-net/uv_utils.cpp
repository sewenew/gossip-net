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

#include "uv_utils.h"
#include "errors.h"

namespace sw::gossip {

UvError::UvError(int err, const std::string &msg) : Error(uv::err_msg(err) + ", " + msg) {}
    
void LoopDeleter::operator()(uv_loop_t *loop) const {
    if (loop == nullptr) {
        return;
    }

    // How to correctly close an event loop:
    // https://stackoverflow.com/questions/25615340/closing-libuv-handles-correctly
    if (uv_loop_close(loop) == 0) {
        delete loop;

        return;
    }

    uv_walk(loop,
            [](uv_handle_t *handle, void *) {
                if (handle != nullptr) {
                    // We don't need to release handle's memory in close callback,
                    // since we'll release the memory in EventLoop's destructor.
                    uv_close(handle, nullptr);
                }
            },
            nullptr);

    // Ensure uv_walk's callback to be called.
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);

    delete loop;
}

namespace uv {

namespace detail {

TcpUPtr make_tcp_server(uv_loop_t &loop, bool is_ipv6);

bool is_ipv6(const std::string &ip);

void enable_reuseport(uv_tcp_t &server);

void enable_nodelay(uv_tcp_t &server);

void bind_server(uv_tcp_t &server, const SockAddr &addr);

void bind_server(uv_udp_t &server, const SockAddr &addr);

}

LoopUPtr make_loop() {
    auto loop = std::make_unique<uv_loop_t>();
    auto err = uv_loop_init(loop.get());
    if (err != 0) {
        throw UvError(err, "failed to make uv loop");
    }

    return LoopUPtr(loop.release());
}

AsyncUPtr make_async(uv_loop_t &loop, uv_async_cb callback, void *data) {
    auto uv_async = std::make_unique<uv_async_t>();
    auto err = uv_async_init(&loop, uv_async.get(), callback);
    if (err != 0) {
        throw UvError(err, "failed to make uv async");
    }

    set_data(uv_async.get(), data);

    return uv_async;
}

TimerUPtr make_timer(uv_loop_t &loop,
        uv_timer_cb callback,
        const std::chrono::milliseconds &timeout,
        const std::chrono::milliseconds &repeat,
        void *data) {
    auto timer = std::make_unique<uv_timer_t>();
    auto err = uv_timer_init(&loop, timer.get());
    if (err != 0) {
        throw UvError(err, "failed to make uv timer");
    }

    err = uv_timer_start(timer.get(), callback, timeout.count(), repeat.count());
    if (err != 0) {
        throw UvError(err, "failed to start uv timer");
    }

    set_data(timer.get(), data);

    return timer;
}

TcpUPtr make_tcp_server(uv_loop_t &loop,
            const TcpOptions &options,
            uv_connection_cb on_connect,
            void *data) {
    auto server = detail::make_tcp_server(loop, detail::is_ipv6(options.ip));

    detail::enable_reuseport(*server);

    if (options.nodelay) {
        detail::enable_nodelay(*server);
    }

    detail::bind_server(*server, SockAddr(options.ip, options.port));

    auto err = uv_listen(to_stream(server.get()), options.backlog, on_connect);
    if (err != 0) {
        throw UvError(err, "failed to listen to port");
    }

    if (data != nullptr) {
        set_data(server.get(), data);
    }

    return server;
}

TcpUPtr make_tcp_client(uv_loop_t &loop, void *data) {
    auto client = std::make_unique<uv_tcp_t>();
    uv_tcp_init(&loop, client.get());

    set_data(client.get(), data);

    return client;
}

WriteUPtr make_write(uv_loop_t &loop, void *data) {
    auto w = std::make_unique<uv_write_t>();
    set_data(w.get(), data);

    return w;
}

UdpUPtr make_udp_server(uv_loop_t &loop,
        const UdpOptions &options,
        void *data) {
    auto server = std::make_unique<uv_udp_t>();
    auto err = uv_udp_init(&loop, server.get());
    if (err != 0) {
        throw UvError(err, "failed to initialize udp server");
    }

    detail::bind_server(*server, SockAddr(options.ip, options.port));

    set_data(server.get(), data);

    return server;
}

namespace detail {

TcpUPtr make_tcp_server(uv_loop_t &loop, bool is_ipv6) {
    unsigned int flags = AF_INET;
    if (is_ipv6) {
        flags = AF_INET6;
    }

    auto server = std::make_unique<uv_tcp_t>();
    uv_tcp_init_ex(&loop, server.get(), flags);

    return server;
}

bool is_ipv6(const std::string &ip) {
    return ip.find(":") != std::string::npos;
}

void enable_reuseport(uv_tcp_t &server) {
    int fd = 0;
    auto err = uv_fileno(to_handle(&server), &fd);
    if (err != 0) {
        throw UvError(err, "failed to get underlying socket");
    }

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) != 0) {
        throw Error("failed to set SO_REUSEPORT option");
    }
}

void enable_nodelay(uv_tcp_t &server) {
    auto err = uv_tcp_nodelay(&server, 1);
    if (err != 0) {
        throw UvError(err, "failed to enable tcp nodelay");
    }
}

void bind_server(uv_tcp_t &server, const SockAddr &addr) {
    auto err = uv_tcp_bind(&server, addr.addr(), 0);
    if (err != 0) {
        throw UvError(err, "failed to bind to port");
    }
}

void bind_server(uv_udp_t &server, const SockAddr &addr) {
    auto err = uv_udp_bind(&server, addr.addr(), UV_UDP_REUSEADDR);
    if (err != 0) {
        throw UvError(err, "failed to bind to port");
    }
}

}

}

SockAddr::SockAddr(const std::string &ip, int port) {
    if (uv::detail::is_ipv6(ip)) {
        sockaddr_in6 addr;
        auto err = uv_ip6_addr(ip.data(), port, &addr);
        if (err != 0) {
            throw UvError(err, "invalid ipv6 address");
        }

        _addr = addr;
    } else {
        sockaddr_in addr;
        auto err = uv_ip4_addr(ip.data(), port, &addr);
        if (err != 0) {
            throw UvError(err, "invalid ipv4 address");
        }

        _addr = addr;
    }
}

const sockaddr* SockAddr::addr() const {
    if (std::holds_alternative<sockaddr_in>(_addr)) {
        return reinterpret_cast<const sockaddr *>(&(std::get<sockaddr_in>(_addr)));
    } else {
        return reinterpret_cast<const sockaddr *>(&(std::get<sockaddr_in6>(_addr)));
    }
}

}
