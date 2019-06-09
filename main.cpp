#include <cstdint>
#include <iostream>
#include <conio.h>
#include <i86.h>
#include <dos.h>

const int VP_SEQC = 0x3c4;  // sequence controller
const int VP_CRTC = 0x3d4;  // CRT controller
const int VP_MISC = 0x3c2;
const int VP_PALETTE_INDEX = 0x3c8;
const int VP_PALETTE_DATA = 0x3c9;
const int VP_STA1 = 0x3da;  // Input Status #1 (color mode)
const int VRETRACE = 0x08;

void SetBIOSMode(int num) {
	union REGS r;
	r.x.eax = num;
	int386(0x10, &r, &r); }

void SpinUntilRetracing() {
	while (!(inp(VP_STA1) & VRETRACE)) {}}

void SpinWhileRetracing() {
	while ((inp(VP_STA1) & VRETRACE)) {}}


void SetModeX() {
	SetBIOSMode(0x13);

	outpw(VP_SEQC, 0x604);  // disable chain4
	outpw(VP_SEQC, 0x100);  // synchronous reset while switching clocks
	outp(VP_MISC, 0xe7);   // select 28 MHz dot clock & 60 Hz scanning rate

	outpw(VP_SEQC, 0x300);  // undo reset (restart sequencer)

	outp(VP_CRTC, 0x11);   // VSync End reg contains register write-protect bit
	unsigned char tmp = inp(VP_CRTC+1);  // get current VSync End register setting
	tmp &= 0x7f;           // remove write-protect on various CRTC registers
	outp(VP_CRTC+1, tmp);

	outpw(VP_CRTC, 0x0d06);  // vertical total
	outpw(VP_CRTC, 0x3e07);  // overflow (bit 8 of vertical counts)
	outpw(VP_CRTC, 0x4109);  // cell height (2 to double-scan)
	outpw(VP_CRTC, 0xea10);  // v sync start
	outpw(VP_CRTC, 0xac11);  // v sync end and protect cr0-cr7
	outpw(VP_CRTC, 0xdf12);  // vertical displayed
	outpw(VP_CRTC, 0x0014);  // turn off dword mode
	outpw(VP_CRTC, 0xe715);  // v blank start
	outpw(VP_CRTC, 0x0616);  // v blank end
	outpw(VP_CRTC, 0xe317);  // turn on byte mode

	// outpw(VP_SEQC, 0x0f02);  // enable writes to all four planes
	}

struct keyinfo {
	int scanCode;
	int ascii; };


keyinfo WaitForKey() {
	union REGS r;
	r.x.eax = 0;
	int386(0x16, &r, &r);
	keyinfo ki;
	ki.scanCode = r.h.ah;
	ki.ascii = r.h.al;
	return ki; }


/*
FarPtr GetISRAddr(int num) {
	union REGS r;
	SREGS sr;
	r.h.ah = 0x35;
	r.h.al = num;
	segread(&sr);
	int386x(0x21, &r, &r, &sr);

	FarPtr out;
	out.sel = sr.es;
	out.ptr = r.x.ebx;
	return out; }
*/

	//std::cout << "ES:" << sr.es << "\n";
	//std::cout << "EBX:" << r.x.ebx << "\n"; }


/*
void SetISRAddr(int num, FarPtr ptr) {
	REGS r;
	SREGS sr;
	segread(&sr);
	sr.ds = ptr.sel;
	r.h.ah = 0x25;
	r.h.al = num;
	r.x.edx = ptr.ptr;
	int386x(0x21, &r, &r, &sr); }
*/

/*
FarPtr make_code_farptr(void *addr) {
	SREGS sr;
	segread(&sr);
	FarPtr out;
	out.sel = sr.cs;
	out.ptr = reinterpret_cast<std::uint32_t*>(addr);
	return out; }
*/

volatile int bazCount = 0;
void (__interrupt * oldTimerISRPtr)();

void __interrupt baz() {
	bazCount++;
	// _chain_intr(oldTimerISRPtr); }
	outp(0x20, 0x20); }

const int TIMER_ISR_NUM = 0x08;



int main(int argc, char *argv[]) {
	std::cout << "Hello world" << std::endl;

	oldTimerISRPtr = _dos_getvect(TIMER_ISR_NUM);
	_dos_setvect(TIMER_ISR_NUM, &baz);

	WaitForKey();
	_dos_setvect(TIMER_ISR_NUM, oldTimerISRPtr);

	std::cout << "ticks: " << bazCount << "\n";
	WaitForKey();
	// cout << "scan code = " << r.h.ah << "\n";
	// cout << "ascii     = " << r.h.al << "\n";

	SetBIOSMode(0x13);

	{
	unsigned char* vgaPtr = (unsigned char*)0xa0000L;
	unsigned char a = 0;
	for (int y=0; y<200; y++) {
		unsigned char aa = a;
		for (int x=0; x<320; x++) {
			*vgaPtr = aa;
			vgaPtr += 1;
			aa += 1; }}
	}


	WaitForKey();
	SetModeX();
	{
	unsigned char* vgaPtr = (unsigned char*)0xa0000L;
	unsigned char a = 0;
	for (int y=0; y<240; y++) {
		unsigned char aa = a;
		for (int x=0; x<80; x++) {
			*vgaPtr = aa;
			vgaPtr += 1;
			aa += 1; }}
	}
	WaitForKey();

	SetBIOSMode(0x3);
	std::cout << "back?" << std::endl;



	return 0; }
