#pragma once

#include "cista/reflection/member_index.h"
#include "cista/serialization.h"

#include "ctx/future.h"

#include "crpc/fn.h"

namespace crpc {

template <typename Transport, typename Interface>
struct client : public Transport {
  template <typename... Args>
  client(Args&&... args)  // NOLINT
      : Transport{std::forward<Args>(args)...} {}

  template <typename ReturnType, typename... Args>
  auto call(fn<ReturnType, Args...> Interface::*const member_ptr,
            Args&&... args) {
    ctx::future_ptr<std::vector<unsigned char>> response;
    if constexpr (sizeof...(Args) == 0U) {
      response = Transport::send(cista::member_index(member_ptr), {});
    } else {
      auto const params = cista::tuple{std::forward<Args>(args)...};
      response = Transport::send(cista::member_index(member_ptr),
                                 cista::serialize(params));
    }
    if constexpr (!std::is_same_v<
                      ReturnType,
                      void>) {  // NOLINT(bugprone-suspicious-semicolon)
      return response->resolve(
          [](auto&& data) { return *cista::deserialize<ReturnType>(data); });
    }
  }
};

}  // namespace crpc