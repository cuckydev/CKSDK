#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char *argv[])
{
	if (argc < 3)
		return 0;

	std::ifstream in(argv[1], std::ios::binary | std::ios::ate);
	if (!in)
		return -1;

	size_t in_size = in.tellg();
	std::vector<char> in_data(in_size);
	in.seekg(0);
	in.read(in_data.data(), in_size);

	std::ofstream out(argv[2]);
	if (!out)
		return -1;
	for (auto &i : in_data)
		out << (int)i << ',';

	return 0;
}