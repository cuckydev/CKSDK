#include <iostream>
#include <fstream>
#include <vector>

#include "elf.h"

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
		Elf32_Addr seg_addr;
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
		if (min_addr - seg_addr < 0x10000)
		{
			std::cerr << "Input ELF overrides kernel" << std::endl;
			return 1;
		}
		if (max_addr - min_addr > 0x200000)
		{
			std::cerr << "Input ELF is larger than 2MB" << std::endl;
			return 1;
		}

		// Read program segments into buffer
		buffer.resize(max_addr - min_addr);
		for (auto &i : phdrs)
		{
			if (i.p_flags == PF_R)
				continue;
			elf.seekg(i.p_offset);
			elf.read(&buffer[i.p_vaddr - min_addr], i.p_filesz);
		}
	}

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
		psexe.exec.t_addr = min_addr;
		psexe.exec.t_size = max_addr - min_addr;
		psexe.exec.pc0 = ehdr.e_entry;

		// Write EXE data
		exe.write((char*)&psexe, sizeof(psexe));
		exe.seekp(2048);
		exe.write(buffer.data(), buffer.size());
	}

	return 0;
}