#include <iostream>
#include <sstream>
#include <string_view>

#include "boost/asio/io_service.hpp"

#include "crpc/client.h"
#include "crpc/server.h"
#include "crpc/stub_transport.h"
#include "crpc/ws_server.h"
#include "crpc/ws_transport.h"

struct interface {
  crpc::fn<int, int, int> add_;
  crpc::fn<void> hello_world_;
  crpc::fn<void, int> inc_count_;
  crpc::fn<int> get_count_;
};

void server() {}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << "server|client\n";
    return 0;
  }

  auto const mode = std::string_view{argv[1]};

  if (mode == "server") {
    int count = 0;
    auto s = crpc::server<interface>{};
    std::stringstream out;
    s.reg(&interface::add_, [](int a, int b) { return a + b; });
    s.reg(&interface::hello_world_, [&]() { out << "hello world"; });
    s.reg(&interface::inc_count_, [&](int i) { return count += i; });
    s.reg(&interface::get_count_, [&]() { return count; });

    boost::asio::io_service ios;
    auto ws = crpc::ws_server{ios, "0.0.0.0", "8080", s};
    ws.listen();
    ios.run();
  } else {
    ctx::scheduler sched;
    auto c =
        crpc::client<crpc::ws_transport, interface>{sched, "localhost", "8080"};
    c.run([&]() {
      sched.enqueue_io(
          [&]() {
            std::cout << "1+2=" << c.call(&interface::add_, 1, 2).val()
                      << std::endl;
            c.call(&interface::hello_world_);
            c.call(&interface::inc_count_, 5);
            std::cout << "5==" << c.call(&interface::get_count_).val()
                      << std::endl;
            c.ws_->stop();
          },
          ctx::op_id{}, ctx::dummy_data{});
    });
    sched.run(1);
  }
}