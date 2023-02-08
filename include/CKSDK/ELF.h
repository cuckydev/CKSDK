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

/// @file CKSDK/ELF.h
/// @brief CKSDK ELF types

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK ELF namespace
	namespace ELF
	{
		/// @brief Implementation of hashing function used in the ELF .hash section
		/// @param str String to hash
		/// @return Hash
		/// @see https://en.wikipedia.org/wiki/PJW_hash_function
		inline uint32_t ElfHash(const char *str)
		{
			uint32_t value = 0;
			while (*str != '\0')
			{
				value <<= 4;
				value += uint32_t(*(str++));

				uint32_t nibble = value & 0xF0000000;
				if (nibble != 0)
					value ^= nibble >> 24;

				value &= ~nibble;
			}
			return value;
		}

		// ELF types
		/// @brief Dynamic tag
		enum Elf32_d_tag
		{
			/// @brief Marks end of dynamic section
			DT_NULL = 0,
			/// @brief Name of needed library
			DT_NEEDED = 1,
			/// @brief Size in bytes of PLT relocs
			DT_PLTRELSZ = 2,
			/// @brief Processor defined value
			DT_PLTGOT = 3,
			/// @brief Address of symbol hash table
			DT_HASH = 4,
			/// @brief Address of string table
			DT_STRTAB = 5,
			/// @brief Address of symbol table
			DT_SYMTAB = 6,
			/// @brief Address of Rela relocs
			DT_RELA = 7,
			/// @brief Total size of Rela relocs
			DT_RELASZ = 8,
			/// @brief Size of one Rela reloc
			DT_RELAENT = 9,
			/// @brief Size of string table
			DT_STRSZ = 10,
			/// @brief Size of one symbol table entry
			DT_SYMENT = 11,
			/// @brief Address of init function
			DT_INIT = 12,
			/// @brief Address of termination function
			DT_FINI = 13,
			/// @brief Name of shared object
			DT_SONAME = 14,
			/// @brief Library search path (deprecated)
			DT_RPATH = 15,
			/// @brief Start symbol search here
			DT_SYMBOLIC = 16,
			/// @brief Address of Rel relocs
			DT_REL = 17,
			/// @brief Total size of Rel relocs
			DT_RELSZ = 18,
			/// @brief Size of one Rel reloc
			DT_RELENT = 19,
			/// @brief Type of reloc in PLT
			DT_PLTREL = 20,
			/// @brief For debugging; unspecified
			DT_DEBUG = 21,
			/// @brief Reloc might modify .text
			DT_TEXTREL = 22,
			/// @brief Address of PLT relocs
			DT_JMPREL = 23,
			/// @brief Process relocations of object
			DT_BIND_NOW = 24,
			/// @brief Array with addresses of init fct
			DT_INIT_ARRAY = 25,
			/// @brief Array with addresses of fini fct
			DT_FINI_ARRAY = 26,
			/// @brief Size in bytes of DT_INIT_ARRAY
			DT_INIT_ARRAYSZ = 27,
			/// @brief Size in bytes of DT_FINI_ARRAY
			DT_FINI_ARRAYSZ = 28,
			/// @brief Library search path
			DT_RUNPATH = 29,
			/// @brief Flags for the object being loaded
			DT_FLAGS = 30,
			/// @brief Start of encoded range
			DT_ENCODING = 32,
			/// @brief Array with addresses of preinit fct
			DT_PREINIT_ARRAY = 32,
			/// @brief Size in bytes of DT_PREINIT_ARRAY
			DT_PREINIT_ARRAYSZ = 33,
			/// @brief Address of SYMTAB_SHNDX section
			DT_SYMTAB_SHNDX = 34,
			/// @brief Number used
			DT_NUM = 35,
			/// @brief Start of OS-specific
			DT_LOOS = 0x6000000d,
			/// @brief End of OS-specific
			DT_HIOS = 0x6ffff000,
			/// @brief Start of processor-specific
			DT_LOPROC = 0x70000000,
			/// @brief End of processor-specific
			DT_HIPROC = 0x7fffffff,

			DT_MIPS_RLD_VERSION = 0x70000001,
			DT_MIPS_FLAGS = 0x70000005,
			DT_MIPS_BASE_ADDRESS = 0x70000006,
			DT_MIPS_LOCAL_GOTNO = 0x7000000a,
			DT_MIPS_SYMTABNO = 0x70000011,
			DT_MIPS_UNREFEXTNO = 0x70000012,
			DT_MIPS_GOTSYM = 0x70000013,
			DT_MIPS_HIPAGENO = 0x70000014
		};

		/// @brief MIPS specific dynamic flags
		enum Elf32_d_MIPS_FLAGS
		{
			/// @brief No flags
			RHF_NONE = 0,
			/// @brief Use quickstart
			RHF_QUICKSTART = 1,
			/// @brief Hash size not power of 2
			RHF_NOTPOT = 2,
			/// @brief Ignore LD_LIBRARY_PATH
			RHF_NO_LIBRARY_REPLACEMENT = 4
		};

		/// @brief Dynamic symbol table entry
		struct Elf32_Dyn
		{
			/// @brief Dynamic entry type
			Elf32_d_tag	d_tag;
			union
			{
				/// @brief Integer value
				uint32_t d_val;
				/// @brief Address value
				void *d_ptr;
			} d_un;
		};

		/// @brief Symbol table entry
		struct Elf32_Sym
		{
			/// @brief Symbol name (string tbl index)
			uint32_t st_name;
			/// @brief Symbol value
			void *st_value;
			/// @brief Symbol size
			size_t st_size;
			/// @brief Symbol type and binding
			uint8_t st_info;
			/// @brief Symbol visibility
			uint8_t st_other;
			/// @brief Section index
			uint16_t st_shndx;
		};
		
		/// @brief Symbol section index
		enum Elf32_st_shndx
		{
			/// @brief Section undefined
			/// @details The symbol is undefined
			SHN_UNDEF = 0,

			/// @brief Start of processor-specific
			SHN_LOPROC          = 0xFF00,

			/// @brief Allocated common symbols
			SHN_MIPS_ACOMMON    = SHN_LOPROC + 0,
			/// @brief Allocated text symbols
			SHN_MIPS_TEXT       = SHN_LOPROC + 1,
			/// @brief Allocated data symbols
			/// @details Appears to represent a table of pointers with 2 reserved entries at the start
			SHN_MIPS_DATA       = SHN_LOPROC + 2,
			/// @brief Small common symbols
			SHN_MIPS_SCOMMON    = SHN_LOPROC + 3,
			/// @brief Small undefined symbols
			SHN_MIPS_SUNDEFINED = SHN_LOPROC + 4
		};

		#define ELF32_ST_BIND(val) ((CKSDK::ELF::Elf32_st_bind) (((uint8_t)(val)) >> 4))
		#define ELF32_ST_TYPE(val) ((CKSDK::ELF::Elf32_st_type) ((val) & 0xF))
		#define ELF32_ST_INFO(bind, type) ((((uint8_t)(bind)) << 4) + (((uint8_t)(type)) & 0xF))

		/// @brief Symbol binding
		enum Elf32_st_bind
		{
			/// @brief Local symbol
			STB_LOCAL = 0,
			/// @brief Global symbol
			STB_GLOBAL = 1,
			/// @brief Weak symbol
			STB_WEAK = 2,
			/// @brief Number of defined types
			STB_NUM = 3,
			/// @brief Start of OS-specific
			STB_LOOS = 10,
			/// @brief Unique symbol
			STB_GNU_UNIQUE = 10,
			/// @brief End of OS-specific
			STB_HIOS = 12,
			/// @brief Start of processor-specific
			STB_LOPROC = 13,
			/// @brief End of processor-specific
			STB_HIPROC = 15
		};

		/// @brief Symbol type
		enum Elf32_st_type
		{
			/// @brief Symbol type is unspecified
			STT_NOTYPE = 0,
			/// @brief Symbol is a data object
			STT_OBJECT = 1,
			/// @brief Symbol is a code object
			STT_FUNC = 2,
			/// @brief Symbol associated with a section
			STT_SECTION = 3,
			/// @brief Symbol's name is file name
			STT_FILE = 4,
			/// @brief Symbol is a common data object
			STT_COMMON = 5,
			/// @brief Symbol is thread-local data object
			STT_TLS = 6,
			/// @brief Number of defined types
			STT_NUM = 7,
			/// @brief Start of OS-specific
			STT_LOOS = 10,
			/// @brief Symbol is indirect code object
			STT_GNU_IFUNC = 10,
			/// @brief End of OS-specific
			STT_HIOS = 12,
			/// @brief Start of processor-specific
			STT_LOPROC = 13,
			/// @brief End of processor-specific
			STT_HIPROC = 15
		};
	}
}
