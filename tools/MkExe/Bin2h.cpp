/*
	[ CKSDK ]
	Copyright 2023 Regan "CKDEV" Green

	Permission to use, copy, modify, and/or distribute this software for any
	purpose with or without fee is hereby granted, provided that the above
	copyright notice and this permission notice appear in all copies.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
	WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
	ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
	WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
	ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
	OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

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