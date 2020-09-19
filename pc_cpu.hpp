#pragma once
#include <algorithm>
#include <stdexcept>

#include <dpmi.h>  // go32_dpmi_seginfo, {get,set}_protected_mode_interrupt_vector
#include <go32.h>

namespace rqdq {
namespace pc {

using ISRPtr = _go32_dpmi_seginfo;
using ISRFunc = void (*)();

extern int cscnt;

class PreparedISR {
	_go32_dpmi_seginfo ptr_;

public:
	PreparedISR() {
		ptr_.pm_offset = 0; }
	PreparedISR(ISRFunc func) {
		ptr_.pm_offset = (int)func;
		ptr_.pm_selector = _go32_my_cs();
		_go32_dpmi_allocate_iret_wrapper(&ptr_); }
	PreparedISR(const PreparedISR&) = delete;
	auto operator=(const PreparedISR&) -> PreparedISR& = delete;
	auto operator=(PreparedISR&& other) -> PreparedISR& {
		std::swap(ptr_, other.ptr_);
		return *this; }
	PreparedISR(PreparedISR&& other) {
		ptr_.pm_offset = 0;
		std::swap(ptr_, other.ptr_); }
	~PreparedISR() {
		if (ptr_.pm_offset != 0) {
			_go32_dpmi_free_iret_wrapper(&ptr_);
			ptr_.pm_offset = 0; }}
	auto Ptr() const -> const ISRPtr& {
		return ptr_; }};


inline
void Sleep() {
	__asm__ __volatile__("sti; hlt"); }


inline
void EnableInterrupts() {
	__asm__ __volatile__("sti"); }


inline
void DisableInterrupts() {
	__asm__ __volatile__("cli"); }


class CriticalSection {
public:
	CriticalSection() {
#if 0
		if (cscnt > 0) {
			throw std::runtime_error("nested critical section"); }
		cscnt++;
#endif
		DisableInterrupts(); }

	// non-copyable
	auto operator=(const CriticalSection&) -> CriticalSection& = delete;
	CriticalSection(const CriticalSection&) = delete;

	~CriticalSection() {
#if 0
		cscnt--;
#endif
		EnableInterrupts(); }};


inline
void SetVect(int isrNum, ISRPtr func) {
	_go32_dpmi_set_protected_mode_interrupt_vector(isrNum, &func); }


inline
void SetVect(int isrNum, const PreparedISR& func) {
	_go32_dpmi_seginfo tmp = func.Ptr();
	_go32_dpmi_set_protected_mode_interrupt_vector(isrNum, &tmp); }


inline
auto GetVect(int isrNum) -> ISRPtr {
	ISRPtr out;
	_go32_dpmi_get_protected_mode_interrupt_vector(isrNum, &out);
	return out; }


}  // namespace pc
}  // namespace rqdq
