#pragma once

#include <vector>

#include "crpc/client.h"

namespace crpc {

template <typename Interface>
struct stub_transport {
  explicit stub_transport(server<Interface>& s) : s_{s} {}

  std::vector<unsigned char> send(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    return s_.call(fn_idx, params);
  }

  server<Interface>& s_;
};

template <typename Interface>
stub_transport(server<Interface>& s) -> stub_transport<Interface>;

template <typename Interface>
using stub_client = client<stub_transport<Interface>, Interface>;

}  // namespace crpc