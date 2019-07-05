#pragma once
#include <dos.h>   // enable/disable
#include <dpmi.h>  // go32_dpmi_seginfo, {get,set}_protected_mode_interrupt_vector

namespace rqdq {
namespace pc {

using ISRPtr = _go32_dpmi_seginfo;
using ISRFunc = void (*)();


inline void EnableInterrupts() {
	_enable(); }


inline void DisableInterrupts() {
	_disable(); }


class CriticalSection {
public:
	CriticalSection() {
		_disable(); }
	~CriticalSection() {
		_enable(); }
private:
	CriticalSection& operator=(const CriticalSection&);  // non-copyable
	CriticalSection(const CriticalSection&); };          // non-copyable


inline void SetVect(int isrNum, ISRPtr func) {
	_go32_dpmi_set_protected_mode_interrupt_vector(isrNum, &func); }


inline ISRPtr GetVect(int isrNum) {
	ISRPtr out;
	_go32_dpmi_get_protected_mode_interrupt_vector(isrNum, &out);
	return out; }


}  // namespace pc
}  // namespace rqdq
