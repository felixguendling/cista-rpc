#pragma once

#include "net/web_server/web_server.h"

#include "cista/serialization.h"

#include "crpc/ws_message.h"

namespace crpc {

template <typename Server>
struct ws_server {
  ws_server(boost::asio::io_service& ios, std::string host, std::string port,
            Server& s)
      : host_{std::move(host)},
        port_{std::move(port)},
        ws_server_{ios},
        s_{s} {}

  void listen() {
    ws_server_.on_ws_msg([&](net::ws_session_ptr const& session,
                             std::string const& msg, net::ws_msg_type type) {
      auto const req = cista::deserialize<ws_message>(msg);
      auto const response_buf = s_.call(
          req->fn_idx_,
          std::vector<unsigned char>{begin(req->payload_), end(req->payload_)});

      ws_message res;
      res.payload_ = {begin(response_buf), end(response_buf)};
      res.ticket_ = req->ticket_;

      auto const res_buf = cista::serialize(res);
      auto const lock = session.lock();
      if (lock) {
        lock->send(std::string{begin(res_buf), end(res_buf)},
                   net::ws_msg_type::BINARY,
                   [](boost::system::error_code, size_t) {});
      }
    });
    boost::system::error_code ec;
    ws_server_.init(host_, port_, ec);
    if (!ec) {
      ws_server_.run();
    }
  }

  std::string host_, port_;
  net::web_server ws_server_;
  Server& s_;
};

template <typename Server>
ws_server(boost::asio::io_service&, std::string, std::string, Server&)
    -> ws_server<Server>;

}  // namespace crpc