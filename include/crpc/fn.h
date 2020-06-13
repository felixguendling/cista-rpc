#pragma once

namespace crpc {

template <typename ReturnType, typename... Args>
struct fn {};

using fn_idx_t = uint64_t;

}  // namespace crpc