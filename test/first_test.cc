#include <sstream>

#include "doctest/doctest.h"

#include "crpc/client.h"
#include "crpc/server.h"
#include "crpc/stub_transport.h"

TEST_CASE("first_test") {
  struct interface {
    crpc::fn<int, int, int> add_;
    crpc::fn<void> hello_world_;
    crpc::fn<void, int> inc_count_;
    crpc::fn<int> get_count_;
  };

  int count = 0;
  auto s = crpc::server<interface>{};
  std::stringstream out;
  s.reg(&interface::add_, [](int a, int b) { return a + b; });
  s.reg(&interface::hello_world_, [&]() { out << "hello world"; });
  s.reg(&interface::inc_count_, [&](int i) { return count += i; });
  s.reg(&interface::get_count_, [&]() { return count; });

  auto c = crpc::stub_client<interface>{s};
  CHECK(c.call(&interface::add_, 1, 2) == 3);
  CHECK_NOTHROW(c.call(&interface::hello_world_));
  CHECK(out.str() == "hello world");
  CHECK_NOTHROW(c.call(&interface::inc_count_, 5));
  CHECK(c.call(&interface::get_count_) == 5);
}