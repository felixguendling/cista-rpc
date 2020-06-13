#pragma once

#include <array>
#include <functional>

#include "cista/reflection/arity.h"
#include "cista/reflection/member_index.h"
#include "cista/serialization.h"

#include "crpc/fn.h"

namespace crpc {

template <typename Interface>
struct server {
  std::vector<unsigned char> call(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    return fn_.at(fn_idx)(params);
  }

  template <typename Fn, typename ReturnType, typename... Args>
  void reg(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& f) {
    fn_[cista::member_index(member_ptr)] =
        [mf = std::forward<Fn>(f)](std::vector<unsigned char> const& in)
        -> std::vector<unsigned char> {
      if constexpr (std::is_same_v<ReturnType, void>) {
        if constexpr (sizeof...(Args) == 0) {
          mf();
        } else {
          std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
        }
        return {};
      } else {
        if constexpr (sizeof...(Args) == 0) {
          auto const return_value = mf();
          return cista::serialize(return_value);
        } else {
          auto const return_value =
              std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
          return cista::serialize(return_value);
        }
      }
    };
  }

  std::array<std::function<std::vector<unsigned char>(
                 std::vector<unsigned char> const&)>,
             cista::arity<Interface>()>
      fn_;
};

}  // namespace crpc