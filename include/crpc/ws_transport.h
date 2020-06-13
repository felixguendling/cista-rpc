#pragma once

#include <atomic>

#include "net/ws_client.h"

#include "ctx/ctx.h"

#include "crpc/fn.h"
#include "crpc/ws_message.h"

namespace crpc {

struct ws_transport {
  using ticket_t = uint64_t;
  using response_future_t = ctx::future<std::vector<unsigned char>>;
  using response_future_ptr = ctx::future_ptr<std::vector<unsigned char>>;

  explicit ws_transport(ctx::scheduler<ctx::dummy_data>& scheduler,
                        std::string const& server_address,
                        std::string const& server_port)
      : ws_{std::make_unique<net::ws_client>(scheduler.runner_.ios(),
                                             server_address, server_port)},
        scheduler_{scheduler} {}

  void run(std::function<void()> f) {
    ws_->on_msg([&](std::string const& s, bool const data) {
      auto const res = *cista::deserialize<ws_message>(s);
      tickets_.at(res.ticket_)->set({begin(res.payload_), end(res.payload_)});
    });
    ws_->run([f](boost::system::error_code const ec) {
      if (!ec) {
        f();
      }
    });
    ws_->on_fail([](boost::system::error_code const ec) {
      std::cout << "fail: " << ec.message() << "\n";
    });
  }

  response_future_ptr send(fn_idx_t fn_idx,
                           std::vector<unsigned char> const& params) {
    auto const msg =
        ws_message{ticket_++, fn_idx,
                   data::vector<unsigned char>{begin(params), end(params)}};
    auto const msg_buf = cista::serialize(msg);
    auto const msg_str = std::string{begin(msg_buf), end(msg_buf)};
    ws_->send(msg_str, true);
    return tickets_
        .emplace(msg.ticket_,
                 std::make_shared<response_future_t>(ctx::op_id{"test"}))
        .first->second;
  }

  std::atomic<ticket_t> ticket_{0U};
  std::unique_ptr<net::ws_client> ws_;
  cista::raw::hash_map<ticket_t, response_future_ptr> tickets_;
  ctx::scheduler<ctx::dummy_data>& scheduler_;
};

}  // namespace crpc