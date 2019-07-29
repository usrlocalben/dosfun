#include "log.hpp"

#include "algorithm.hpp"

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace rqdq {
namespace log {

alg::RingIndex<1024> ring;

std::vector<std::string> lines(1024);


}  // namespace log
}  // namespace rqdq
