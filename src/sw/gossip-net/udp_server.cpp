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

#include "udp_server.h"
#include <cassert>
#include <iostream>

namespace sw::gossip {

void UdpServer::_on_alloc(uv_handle_t *handle, size_t /*suggested_size*/, uv_buf_t *buf) {
    assert(handle != nullptr && buf != nullptr);

    auto *server = uv::get_data<UdpServer>(handle);
    assert(server != nullptr);

    buf->base = server->_buffer.data();
    buf->len = server->_buffer.size();
}

void UdpServer::_on_read(uv_udp_t *req, ssize_t nread,
        const uv_buf_t *buf, const sockaddr *addr, unsigned /*flags*/) {
    assert(req != nullptr);

    auto *server = uv::get_data<UdpServer>(req);
    assert(server != nullptr);

    if (nread == 0) {
        return;
    } else if (nread < 0) {
        std::cerr << "read error: " << uv::err_msg(nread) << std::endl;
        uv::handle_close(req, nullptr);
        // TODO: recreate udp socket
    } else {
        std::cout << "on read: " << std::string(buf->base, nread) << std::endl;
        assert(buf != nullptr && buf->base != nullptr && addr != nullptr);

        server->_handle(std::string_view(buf->base, nread));
    }
}

void UdpServer::_on_send(uv_udp_send_t *req, int status) {
    assert(req != nullptr);

    if (status != 0) {
        std::cerr << "failed to do send: " << uv::err_msg(status) << std::endl;
    }

    auto *ctx = uv::get_data<SendContext>(req);

    delete ctx;
    delete req;
}

void UdpServer::_on_event(uv_async_t *handle) {
    assert(handle != nullptr);

    auto *server = uv::get_data<UdpServer>(handle);
    assert(server != nullptr);

    std::vector<Event> events;
    {
        std::lock_guard<std::mutex> lock(server->_mtx);
        events.swap(server->_events);
    }

    for (auto &event : events) {
        server->_send(std::move(event));
    }
}

UdpServer::UdpServer(const UdpServerOptions &opts) :
    _loop(uv::make_loop()),
    _buffer(opts.buffer_size) {
    UdpOptions udp_opts = {opts.ip, opts.port};

    _server = uv::make_udp_server(*_loop, udp_opts, this);
    _async = uv::make_async(*_loop, _on_event, this);
}

void UdpServer::register_command(CommandUPtr command) {
    if (!command) {
        throw Error("null command");
    }

    auto name = command->name();
    if (!_commands.emplace(name, std::move(command)).second) {
        throw Error(name + " has already been registered");
    }
}

void UdpServer::start() {
    uv_udp_recv_start(_server.get(), _on_alloc, _on_read);

    uv_run(_loop.get(), UV_RUN_DEFAULT);
}

void UdpServer::send(const std::string &ip, int port, std::string data) {
    Event event = {ip, port, std::move(data)};

    {
        std::lock_guard<std::mutex> lock(_mtx);
        _events.push_back(std::move(event));
    }

    uv_async_send(_async.get());
}

void UdpServer::_handle(const std::string_view &buf) {
    try {
        auto [requests, bytes_parsed] = RespRequestParser{}.parse(buf);
        if (requests.empty()) {
            // TODO: should we reply with error?
            return;
        }

        // In fact, there's only a single requests so far
        for (const auto &request : requests) {
            auto iter = _commands.find(std::string(request.name));
            if (iter == _commands.end()) {
                std::cerr << "no match command: " << request.name << std::endl;
                // TODO: reply with error
                continue;
            }
            auto &command = iter->second;
            assert(command);
            command->run(request.args);
        }
    } catch (const Error &err) {
        std::cerr << "failed to handle request: " << err.what() << std::endl;
    }
}

void UdpServer::_send(Event event) {
    SockAddr addr(event.ip, event.port);
    auto req = std::make_unique<uv_udp_send_t>();
    auto ctx = std::make_unique<SendContext>(std::move(event.data));
    auto err = uv_udp_send(req.get(), _server.get(),
            &(ctx->buf), 1, addr.addr(), _on_send);
    if (err != 0) {
        std::cerr << "failed to do send: " << uv::err_msg(err) << std::endl;
        // TODO: should we close handle?
        return;
    } else {
        uv::set_data(req.release(), ctx.release());
    }
}

}
