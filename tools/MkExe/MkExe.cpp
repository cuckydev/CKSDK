#include <iostream>
#include <fstream>
#include <vector>

#include "elf.h"

#define LZ4_STATIC_LINKING_ONLY
#include <lz4hc.h>

static char bootstrap_blob[] = {
#include <Bootstrap.h>
};

void Set16(char *p, unsigned short x)
{
	p[0] = (x >> 0) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
}

void Set32(char *p, unsigned long x)
{
	p[0] = (x >> 0) & 0xFF;
	p[1] = (x >> 8) & 0xFF;
	p[2] = (x >> 16) & 0xFF;
	p[3] = (x >> 24) & 0xFF;
}

struct PsExe
{
	char ident[16] = "PS-X EXE";
	struct
	{
		uint32_t pc0 = 0;
		uint32_t gp0 = 0;
		uint32_t t_addr = 0;
		uint32_t t_size = 0;
		uint32_t d_addr = 0;
		uint32_t d_size = 0;
		uint32_t b_addr = 0;
		uint32_t b_size = 0;
		uint32_t sp_addr = 0;
		uint32_t sp_size = 0;
		uint32_t sp = 0;
		uint32_t fp = 0;
		uint32_t gp = 0;
		uint32_t ret = 0;
		uint32_t base = 0;
	} exec;
	char copyright[64] = {};
};

int main(int argc, char *argv[])
{
	// Check arguments
	if (argc < 3)
	{
		std::cout << "usage: MkExe in.elf out.exe" << std::endl;
		return 0;
	}

	// ELF data
	Elf32_Ehdr ehdr;
	
	std::vector<Elf32_Phdr> phdrs;
	Elf32_Addr min_addr = 0xFFFFFFFF;
	Elf32_Addr max_addr = 0;
	Elf32_Addr seg_addr;
	
	std::vector<char> buffer;

	// Open input ELF
	{
		std::ifstream elf(argv[1], std::ios::binary);
		if (!elf)
		{
			std::cerr << "Could not open input ELF" << std::endl;
			return 1;
		}

		// Read ELF header
		elf.read((char*)&ehdr, sizeof(ehdr));

		// Check ELF header
		if (ehdr.e_ident.e_identbytes[0] != 0x7F ||
			ehdr.e_ident.e_identbytes[1] != 'E' ||
			ehdr.e_ident.e_identbytes[2] != 'L' ||
			ehdr.e_ident.e_identbytes[3] != 'F')
		{
			std::cerr << "Input file is not an ELF" << std::endl;
			return 1;
		}

		// Check ELF type
		if (ehdr.e_type != ET_EXEC)
		{
			std::cerr << "Input ELF is not an executable" << std::endl;
			return 1;
		}

		// Check ELF machine
		if (ehdr.e_machine != EM_MIPS)
		{
			std::cerr << "Input ELF is not a MIPS executable" << std::endl;
			return 1;
		}

		// Check ELF version
		if (ehdr.e_version != EV_CURRENT)
		{
			std::cerr << "Input ELF has an invalid version" << std::endl;
			return 1;
		}

		// Check ELF word size
		if (ehdr.e_ident.e_mipsident.e_wordsize != ELFCLASS32)
		{
			std::cerr << "Input ELF is not a 32-bit executable" << std::endl;
			return 1;
		}

		// Check ELF endianness
		if (ehdr.e_ident.e_mipsident.e_endianness != ELFDATA2LSB)
		{
			std::cerr << "Input ELF is not a little-endian executable" << std::endl;
			return 1;
		}

		// Read program headers
		phdrs.resize(ehdr.e_phnum);
		elf.seekg(ehdr.e_phoff);
		elf.read((char*)phdrs.data(), sizeof(Elf32_Phdr) * ehdr.e_phnum);

		// Get program address range
		for (auto &i : phdrs)
		{
			if (i.p_flags == PF_R)
				continue;
			if (i.p_vaddr < min_addr)
				min_addr = i.p_vaddr;
			if (i.p_vaddr + i.p_filesz > max_addr)
				max_addr = i.p_vaddr + i.p_filesz;
		}

		min_addr &= ~0xFFF; // Round down to nearest page boundary
		max_addr = (max_addr + 0xFFF) & ~0xFFF; // Round up to nearest page boundary

		// Check address range against segment
		if (min_addr >= 0x80000000 && min_addr < 0xA0000000)
			seg_addr = 0x80000000;
		else if (min_addr >= 0xA0000000 && min_addr < 0xC0000000)
			seg_addr = 0xA0000000;
		else if (min_addr >= 0xC0000000 && min_addr < 0xE0000000)
			seg_addr = 0xC0000000;
		else
		{
			std::cerr << "Input ELF is not in KUSEG, KSEG0 or KSEG1" << std::endl;
			return 1;
		}
		/*
		if (min_addr - seg_addr < 0x10000)
		{
			std::cerr << "Input ELF overrides kernel" << std::endl;
			return 1;
		}
		if (max_addr - seg_addr > 0x200000)
		{
			std::cerr << "Input ELF is larger than 2MB segment" << std::endl;
			return 1;
		}
		*/

		// Read program segments into buffer
		buffer.resize(max_addr - min_addr);
		for (auto &i : phdrs)
		{
			if (i.p_flags == PF_R)
				continue;
			elf.seekg(i.p_offset);
			if (i.p_vaddr < min_addr || (i.p_vaddr - min_addr) + i.p_filesz > buffer.size())
			{
				std::cerr << "ELF segment out of range" << std::endl;
				return 1;
			}
			elf.read(&buffer[i.p_vaddr - min_addr], i.p_filesz);
		}
	}

	// Compress buffer
	size_t cmp_size = LZ4_compressBound(buffer.size());
	std::unique_ptr<char[]> cmp_data = std::make_unique<char[]>(cmp_size);
	int final_cmp_size = LZ4_compress_HC(buffer.data(), cmp_data.get(), buffer.size(), cmp_size, LZ4HC_CLEVEL_MAX);
	if (final_cmp_size == 0)
	{
		std::cerr << "Compression failed" << std::endl;
		return 1;
	}

	size_t margin = LZ4_DECOMPRESS_INPLACE_MARGIN(final_cmp_size);

	// Setup the blob
	size_t blob_addr = ((min_addr + final_cmp_size + margin + 3) / 4) * 4;
	if (blob_addr < (seg_addr + 0x10000))
		blob_addr = seg_addr + 0x10000;

	size_t dest_addr = min_addr;
	size_t blob_end = blob_addr + final_cmp_size;
	size_t blob_entry = ehdr.e_entry;

	Set32(bootstrap_blob + 0x0000, blob_addr);
	Set32(bootstrap_blob + 0x0004, dest_addr);
	Set32(bootstrap_blob + 0x0008, blob_end);
	Set32(bootstrap_blob + 0x000C, blob_entry);

	size_t bootstrap_addr = ((blob_end + 3) / 4) * 4;
	size_t bootstrap_end = bootstrap_addr + sizeof(bootstrap_blob);

	Set16(bootstrap_blob + 0x0010 + 0x0000, (bootstrap_addr >> 16) & 0xFFFF);
	Set16(bootstrap_blob + 0x0010 + 0x0004, (bootstrap_addr >> 0) & 0xFFFF);

	// Open output EXE
	{
		std::ofstream exe(argv[2], std::ios::binary);
		if (!exe)
		{
			std::cerr << "Could not open output EXE" << std::endl;
			return 1;
		}

		// Setup EXE header
		PsExe psexe;
		psexe.exec.t_addr = blob_addr;
		psexe.exec.t_size = ((bootstrap_end - blob_addr + 2047) / 2048) * 2048;
		psexe.exec.pc0 = bootstrap_addr + 0x0010;

		std::cout << "min_addr = " << std::hex << min_addr << std::endl;
		std::cout << "max_addr = " << std::hex << max_addr << std::endl;
		std::cout << "t_addr = " << std::hex << psexe.exec.t_addr << std::endl;
		std::cout << "t_size = " << std::hex << psexe.exec.t_size << std::endl;
		std::cout << "pc0 = " << std::hex << psexe.exec.pc0 << std::endl;

		// Write EXE data
		exe.write((char*)&psexe, sizeof(psexe));
		exe.seekp(2048);
		exe.write(cmp_data.get(), final_cmp_size);
		exe.seekp(2048 + bootstrap_addr - blob_addr);
		exe.write(bootstrap_blob, sizeof(bootstrap_blob));
		if ((2048 + psexe.exec.t_size) != exe.tellp())
		{
			exe.seekp(2048 + psexe.exec.t_size - 1);
			exe.put(0);
		}
	}

	return 0;
}