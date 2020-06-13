#pragma once

#include "cista/containers/vector.h"

#include "crpc/fn.h"

namespace crpc {

namespace data = cista::offset;

struct ws_message {
  uint64_t ticket_{0U};
  fn_idx_t fn_idx_{0U};
  data::vector<unsigned char> payload_;
};

}  // namespace crpc