#pragma once
#include "alg_ringindex.hpp"

#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace rqdq {
namespace log {

extern alg::RingIndex<1024> ring;

extern std::vector<std::string> lines;


inline int FrontIdx() {
	return ring.FrontIdx(); }


inline void PopFront() {
	ring.PopFront(); }

	
inline std::string at(int idx) {
	return lines[idx]; }


inline bool Loaded() {
	return ring.Loaded(); }


inline void Reserve() {
	for (auto& line : lines) {
		line.reserve(80); }}

inline void info(const char* fmt, ...) {
	using namespace std;
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	// std::printf("%s\n", buf);

	lines[ring.BackIdx()].assign(buf);
	ring.PushBack(); }


}  // namespace log
}  // namespace rqdq
