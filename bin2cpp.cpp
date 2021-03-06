#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
	char *inputFileName = argv[1];
	char *outputFilePrefix = argv[2];
	char *namespaceName = argv[3];
	char *varName = argv[4];

	ifstream fd(argv[1], ios::in|ios::binary);

	string hppName = string(outputFilePrefix) + ".hpp";
	string cppName = string(outputFilePrefix) + ".cpp";

	ofstream hppFd(hppName.c_str());
	ofstream cppFd(cppName.c_str());

	char ch_;

	hppFd << "#pragma once\n";
	cppFd << "#include <cstdint>\n";
	hppFd << "namespace " << namespaceName << " {\n";
	hppFd << "extern std::uint8_t " << varName << "[];\n";
	hppFd << "}";

	cppFd << "#include \"" << hppName.c_str() << "\"\n";
	cppFd << "#include <cstdint>\n";
	cppFd << "namespace " << namespaceName << " {\n";
	cppFd << "std::uint8_t " << varName << "[] = {\n";
	int n = 0;
	bool first = true;
	while (fd.get(ch_)) {
		uint8_t ch = (uint8_t)ch_;
		if (first) {
			first = false; }
		else {
			cppFd << ","; }
		cppFd << "0x" << hex << (int)ch << dec;
		n++;
		if (n % 30 == 0) {
			cppFd << "\n"; }}
	cppFd << "};\n";
	cppFd << "}";

	cout << "bin2cpp " << n << " bytes\n"; }

