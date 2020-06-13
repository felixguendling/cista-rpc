#pragma once

#include <functional>
#include <vector>

namespace crpc {

using cb_t = std::function<void(std::vector<unsigned char> const&)>;

}  // namespace crpc