/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#if !defined(O_BINARY)
#define O_BINARY 0
#endif

#define kBufSizeInBytes (4096)

void exe2aout(char* exePath) {
	unsigned short header[3];
	int fdIn;
	int fdOut;
	char buf[kBufSizeInBytes];
	int rbytes;

	char *dot = strrchr(exePath, '.');
	if (!dot || strlen(dot) != 4
			|| tolower(dot[1]) != 'e'
			|| tolower(dot[2]) != 'x'
			|| tolower(dot[3]) != 'e') {
		fprintf(stderr, "%s: Arguments MUST end with a .exe extension\n", exePath);
		return; }

	fdIn = open(exePath, O_RDONLY|O_BINARY);
	if (fdIn < 0) {
		perror(exePath);
		return; }

	read(fdIn, header, sizeof(header));
	if (header[0] != 0x5a4d) {
		fprintf(stderr, "`%s' is not an .EXE file\n", exePath);
		return; }

	long header_offset = (long)header[2]*512L;
	if (header[1])
		header_offset += (long)header[1] - 512L;
	lseek(fdIn, header_offset, 0);
	header[0] = 0;
	read(fdIn, header, sizeof(header));
	if ((header[0] != 0x010b) && (header[0] != 0x014c)) {
		fprintf(stderr, "`%s' does not have a COFF/AOUT program appended to it\n", exePath);
		return; }

	lseek(fdIn, header_offset, 0);


	char coffPath[1024];
	int idx = 0;
	for (; exePath+idx!=dot; idx++) {
	  coffPath[idx] = exePath[idx]; }
	coffPath[idx++] = '.';
	coffPath[idx++] = 'c';
	coffPath[idx++] = 'o';
	coffPath[idx++] = 'f';
	coffPath[idx++] = 'f';
	coffPath[idx++] = 0;
	// printf("output to [%s]\n", coffPath);

	fdOut = open(coffPath, O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666);
	if (fdOut < 0) {
		perror(coffPath);
		return; }

	while ((rbytes=read(fdIn, buf, kBufSizeInBytes)) > 0) {
		int wb = write(fdOut, buf, rbytes);
		if (wb < 0) {
			perror(coffPath);
			break; }
		if (wb < rbytes) {
			fprintf(stderr, "`%s': disk full\n", coffPath);
			exit(1); }}

	close(fdIn);
	close(fdOut); }


int main(int argc, char **argv) {
	int i;
	for (i=1; i<argc; i++) {
		exe2aout(argv[i]); }
	return 0; }
