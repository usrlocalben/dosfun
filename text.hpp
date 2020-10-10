#pragma once
#include <string>
#include <string_view>

namespace rqdq {
namespace text {

auto JsonStringify(std::string_view text) -> std::string_view;

auto ConsumePrefix(std::string& str, const std::string& prefix) -> bool;

}  // namespace text
}  // namespace rqdq
