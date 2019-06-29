#pragma once
#include <i86.h>    // _enable(), _disable()
#include <dos.h>    // _dos_setvect(), _dos_getvect()

namespace rqdq {
namespace pc {

typedef void (__interrupt * ISRPtr)();
// typedef void (*ISRPtrEx)(void* userPtr);


inline void EnableInterrupts() {
	_enable(); }


inline void DisableInterrupts() {
	_disable(); }


class CriticalSection {
public:
	CriticalSection() {
		_disable(); }
	~CriticalSection() {
		_enable(); }};


inline void SetVect(int isrNum, ISRPtr func) {
	_dos_setvect(isrNum, func); }


inline ISRPtr GetVect(int isrNum) {
	return _dos_getvect(isrNum); }


}  // namespace pc
}  // namespace rqdq
