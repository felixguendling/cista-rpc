#include <iostream>
#include <queue>
#include <sstream>
#include <string_view>

#include "boost/asio/io_service.hpp"

#include "crpc/client.h"
#include "crpc/server.h"
#include "crpc/stub_transport.h"
#include "crpc/ws_server.h"
#include "crpc/ws_transport.h"

namespace data = cista::offset;

struct asset {
  uint32_t id_;
  data::string name_;
};

struct interface {
  crpc::fn<int, int, int> add_;
  crpc::fn<void> hello_world_;
  crpc::fn<void, int> inc_count_;
  crpc::fn<int> get_count_;
  crpc::fn<data::hash_map<uint32_t, asset>, data::vector<data::string>>
      get_assets_;
};

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
    s.reg(&interface::get_assets_,
          [&,
           store =
               data::hash_map<data::string, asset>{
                   {"hello", asset{0, "hello"}}, {"world", asset{1, "world"}}}](
              data::vector<data::string> const& assets) {
            data::hash_map<uint32_t, asset> response;
            for (auto const& a : assets) {
              auto const& s = store.at(a);
              response.emplace(s.id_, s);
            }
            return response;
          });

    boost::asio::io_service ios;
    auto ws = crpc::ws_server{ios, "0.0.0.0", "8080", s};
    ws.listen();
    ios.run();
  } else {
    boost::asio::io_service ios;
    auto c =
        crpc::client<crpc::ws_transport, interface>{ios, "localhost", "8080"};
    c.run([&]() {
      c.call(
          &interface::add_, [](int sum) { std::cout << "1+2=" << sum << "\n"; },
          1, 2);
      c.call(&interface::hello_world_, []() { std::cout << "hello server\n"; });
      c.call(
          &interface::inc_count_, []() { std::cout << "added 5\n"; }, 5);
      c.call(&interface::get_count_,
             [](int i) { std::cout << "count = " << i << "\n"; });
      c.call(
          &interface::get_assets_,
          [&](data::hash_map<uint32_t, asset> const& assets) {
            for (auto const& [id, a] : assets) {
              std::cout << id << ": " << a.name_ << "\n";
            }
          },
          data::vector<data::string>{
              {data::string{"hello"}, data::string{"world"}}});
    });
    ios.run();
  }
}