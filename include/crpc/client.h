#pragma once

#include "cista/reflection/member_index.h"
#include "cista/serialization.h"

#include "crpc/cb_t.h"
#include "crpc/fn.h"

namespace crpc {

template <typename Transport, typename Interface>
struct client : public Transport {
  template <typename... Args>
  client(Args&&... args)  // NOLINT
      : Transport{std::forward<Args>(args)...} {}

  template <typename ReturnType, typename Fn, typename... Args>
  void call(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& cb,
            Args&&... args) {
    auto send_cb = [acb = std::forward<Fn>(cb)](
                       std::vector<unsigned char> const& response) {
      if constexpr (std::is_same_v<ReturnType, void>) {
        acb();
      } else {
        acb(*cista::deserialize<ReturnType>(response));
      }
    };
    if constexpr (sizeof...(Args) == 0U) {
      Transport::send(cista::member_index(member_ptr), {}, std::move(send_cb));
    } else {
      auto const params = cista::tuple{std::forward<Args>(args)...};
      Transport::send(cista::member_index(member_ptr), cista::serialize(params),
                      std::move(send_cb));
    }
  }
};

}  // namespace crpc