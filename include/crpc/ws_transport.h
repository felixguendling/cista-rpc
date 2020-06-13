#pragma once

#include <atomic>

#include "net/ws_client.h"

#include "crpc/cb_t.h"
#include "crpc/fn.h"
#include "crpc/ws_message.h"

namespace crpc {

struct ws_transport {
  using ticket_t = uint64_t;

  explicit ws_transport(boost::asio::io_service& ios,
                        std::string const& server_address,
                        std::string const& server_port)
      : ws_{std::make_unique<net::ws_client>(ios, server_address,
                                             server_port)} {}

  void run(std::function<void()> cb) {
    ws_->on_msg([&](std::string const& s, bool const data) {
      auto const res = *cista::deserialize<ws_message>(s);
      tickets_.at(res.ticket_)({begin(res.payload_), end(res.payload_)});
    });
    ws_->run([cb = std::move(cb)](boost::system::error_code const ec) {
      if (!ec) {
        cb();
      }
    });
    ws_->on_fail([](boost::system::error_code const ec) {
      std::cout << "fail: " << ec.message() << "\n";
    });
  }

  void send(fn_idx_t fn_idx, std::vector<unsigned char> const& params,
            cb_t cb) {
    auto const msg =
        ws_message{ticket_++, fn_idx,
                   data::vector<unsigned char>{begin(params), end(params)}};
    auto const msg_buf = cista::serialize(msg);
    ws_->send({begin(msg_buf), end(msg_buf)}, true);
    tickets_.emplace(msg.ticket_, std::move(cb));
  }

  std::atomic<ticket_t> ticket_{0U};
  std::unique_ptr<net::ws_client> ws_;
  cista::raw::hash_map<ticket_t, cb_t> tickets_;
};

}  // namespace crpc