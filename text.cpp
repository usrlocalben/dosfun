#include "text.hpp"

#include <cctype>
#include <cstdio>
#include <string_view>
#include <vector>

namespace rqdq {
namespace text {

std::string_view JsonStringify(std::string_view text) {
	static std::vector<char> buf;

	// maximum size of serialized output (all hex repr) plus quotes and null
	buf.reserve(text.size() * 6 + 3);

	buf.clear();
	char* end = buf.data();
	const char ESC = '\\';
	const auto PUSH = [&](char ch) { *end++ = ch; };

	PUSH('"');
	for (const auto c : text) {
		     if (c == '"')  { PUSH(ESC); PUSH('"'); }
		else if (c == '\\') { PUSH(ESC); PUSH(ESC); }
		else if (c == '/')  { PUSH(ESC); PUSH('/'); }
		else if (c == '\b') { PUSH(ESC); PUSH('b'); }
		else if (c == '\f') { PUSH(ESC); PUSH('f'); }
		else if (c == '\n') { PUSH(ESC); PUSH('n'); }
		else if (c == '\r') { PUSH(ESC); PUSH('r'); }
		else if (c == '\t') { PUSH(ESC); PUSH('t'); }
		else if (std::isprint(c)) { PUSH(c); }
		else { end += sprintf(end, "\\u%04x", c); }}
	PUSH('"');
	PUSH(0);

	return { buf.data(), std::size_t(end-buf.data()) }; }


}  // namespace text
}  // namespace rqdq
