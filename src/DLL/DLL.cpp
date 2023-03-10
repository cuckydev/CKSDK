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

/*
	HUGE thanks goes to spicyjpeg, who wrote the original DLL loader
*/

#include <CKSDK/DLL.h>

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>
#include <CKSDK/STL.h>
#include <CKSDK/CD.h>
#include <CKSDK/ISO.h>

#include <CKSDK/Util/Hash.h>

namespace CKSDK
{
	namespace DLL
	{
		// DLL globals
		static SymbolCallback symbol_callback;
		
		// DLL resolver
		void Resolver(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
		{
			// We placed a pointer to the DLL in got[1] in DLL::DLL
			// gp-7FF0h points to the start of the .got section,
			// so we simply index straight there
			register uint32_t gp asm("gp");
			register uint32_t index asm("t8");
			DLL *dll = (DLL*)&((uint32_t*)(gp - 0x7FF0))[1];

			// Get symbol in DLL
			ELF::Elf32_Sym *sym = &(dll->symtab[index]);
			const char *name = &(dll->strtab[sym->st_name]);
			void *addr = symbol_callback(name);
			if (addr == nullptr)
			{
				ExScreen::Abort("Resolve GetSymbol failed");
				return;
			}

			// Patch the GOT entry to "cache" the resolved address
			dll->got[dll->got_local_count + (index - dll->first_got_symbol)] = uint32_t(addr);

			// Call symbol
			void (*func)(uint32_t, uint32_t, uint32_t, uint32_t) = (void(*)(uint32_t, uint32_t, uint32_t, uint32_t))addr;
			func(a0, a1, a2, a3);
		}

		// DLL class
		DLL::DLL(void *_ptr, size_t _size): ptr(_ptr), size(_size)
		{
			// Process segments
			for (ELF::Elf32_Dyn *dyn = (ELF::Elf32_Dyn*)ptr; dyn->d_tag != ELF::DT_NULL; dyn++)
			{
				switch (dyn->d_tag)
				{
					// Offset of .got section
					case ELF::DT_PLTGOT:
						got = (uint32_t*)(uintptr_t(ptr) + dyn->d_un.d_val);
						break;

					// Offset of .hash section
					case ELF::DT_HASH:
						hash = (const uint32_t*)(uintptr_t(ptr) + dyn->d_un.d_val);
						break;

					// Offset of .dynstr (NOT .strtab) section
					case ELF::DT_STRTAB:
						strtab = (const char*)(uintptr_t(ptr) + dyn->d_un.d_val);
						break;

					// Offset of .dynsym (NOT .symtab) section
					case ELF::DT_SYMTAB:
						symtab = (ELF::Elf32_Sym*)(uintptr_t(ptr) + dyn->d_un.d_val);
						break;

					// Length of each .dynsym entry
					case ELF::DT_SYMENT:
						// Only 16-byte symbol table entries are supported
						if (dyn->d_un.d_val != sizeof(ELF::Elf32_Sym))
							ExScreen::Abort("Invalid DLL symtab size");
						break;

					// MIPS ABI (?) version
					case ELF::DT_MIPS_RLD_VERSION:
						// Versions other than 1 are unsupported
						if (dyn->d_un.d_val != 1)
							ExScreen::Abort("Invalid DLL version");
						break;

					// DLL/ABI flags
					case ELF::DT_MIPS_FLAGS:
						// Shortcut pointers are not supported
						if (dyn->d_un.d_val & ELF::RHF_QUICKSTART)
							ExScreen::Abort("Invalid DLL flags");
						break;

					// Number of local (not to resolve) GOT entries
					case ELF::DT_MIPS_LOCAL_GOTNO:
						got_local_count = dyn->d_un.d_val;
						break;

					// Base address DLL was compiled for
					case ELF::DT_MIPS_BASE_ADDRESS:
						// Base addresses other than zero are not supported. It would
						// be easy enough to support them, but why?
						if (dyn->d_un.d_val)
							ExScreen::Abort("Invalid DLL base address");
						break;

					// Number of symbol table entries
					case ELF::DT_MIPS_SYMTABNO:
						symbol_count = dyn->d_un.d_val;
						break;

					// Index of first symbol table entry which has a matching GOT entry
					case ELF::DT_MIPS_GOTSYM:
						first_got_symbol = dyn->d_un.d_val;
						break;
				}
			}

			// Calculate number of GOT entries
			got_extern_count = symbol_count - first_got_symbol;

			// Relocate the DLL by adding its base address to all pointers in the local
			// section of the GOT except the first two, which are reserved. The first
			// entry in particular is a pointer to the lazy resolver, called by
			// auto-generated stubs when a function is first used. got[1] is normally
			// unused, but here we'll set it to the DLL so we can look that up when
			// resolving functions
			got[0] = uint32_t(&Resolver);
			got[1] = uint32_t(this);

			for (uint32_t i = 2; i < got_local_count; i++)
				got[i] += uint32_t(ptr);
			
			// Relocate all pointers in the symbol table and populate the global
			// section of the GOT.
			uint32_t *_got = &(got[got_local_count - first_got_symbol]);

			for (uint32_t i = 0; i < symbol_count; i++)
			{
				// Resolve symbol
				ELF::Elf32_Sym *sym = &symtab[i];
				const char *_name = &strtab[sym->st_name];
				sym->st_value = (void*)(uintptr_t(sym->st_value) + uintptr_t(ptr));

				switch (sym->st_shndx)
				{
					// SHN_MIPS_DATA seems to represent a table of pointers, but I can't
					// find much documentation on it
					case ELF::SHN_MIPS_DATA:
					{
						// Resolve table
						uint32_t *resp = ((uint32_t*)sym->st_value) + 2; // The first two entries seem to be reserved
						uint32_t *rese = (uint32_t*)(uintptr_t(sym->st_value) + sym->st_size);
						for (; resp < rese; resp++)
							*resp += uintptr_t(ptr);
						break;
					}
				}

				// Resolve GOT entry
				if (i < first_got_symbol)
					continue;
				
				if (sym->st_shndx == ELF::SHN_UNDEF)
				{
					// Resolve GOT entry using imported symbol
					void *sym_ptr = symbol_callback(_name);
					if (sym_ptr == nullptr)
					{
						TTY::Out(_name);
						TTY::Out("\n");
						ExScreen::Abort("GOT failed get boot symbol");
					}
					_got[i] = uint32_t(sym_ptr);
				}
				else
				{
					// Resolve GOT entry using pointer
					_got[i] += uint32_t(ptr);
				}
			}

			// Flush I-cache for our modified executables
			OS::DisableIRQ();
			OS::FlushICache();
			OS::EnableIRQ();

			// Run DLL constructors
			const uint32_t *ctor_list = (const uint32_t*)GetSymbol("__CTOR_LIST__");
			if (ctor_list != nullptr)
			{
				for (uint32_t i = ctor_list[0]; i != 0; i--)
				{
					OS::Function<void> ctor = (void(*)())(uintptr_t(ptr) + ctor_list[i]);
					ctor();
				}
			}
		}

		DLL::~DLL()
		{
			// Run DLL destructors
			const uint32_t *dtor_list = (const uint32_t*)GetSymbol("__DTOR_LIST__");
			if (dtor_list != nullptr)
			{
				for (uint32_t i = dtor_list[0]; i != 0; i--)
				{
					OS::Function<void> dtor = (void(*)())(uintptr_t(ptr) + dtor_list[i]);
					dtor();
				}
			}
		}

		void *DLL::GetSymbol(const char *name)
		{
			uint32_t nbucket = hash[0];
			uint32_t nchain = hash[1];
			const uint32_t *bucket = &(hash[2]);
			const uint32_t *chain = &(hash[2 + nbucket]);

			// Go through the hash table's chain until the symbol name matches the one
			// provided.
			for (uint32_t i = bucket[ELF::ElfHash(name) % nbucket]; i != 0xFFFFFFFF; i = chain[i])
			{
				// if (i >= nchain)
				// 	ExScreen::Abort("GetSymbol index OoB");
				ELF::Elf32_Sym *sym = &symtab[i];
				const char *_name = &strtab[sym->st_name];
				if (STL::String::Compare(name, _name) == 0)
					return sym->st_value;
			}

			return nullptr;
		}

		// DLL Functions
		void Init()
		{
			
		}
		
		SymbolCallback SetSymbolCallback(SymbolCallback cb)
		{
			SymbolCallback old = symbol_callback;
			symbol_callback = cb;
			return old;
		}

		SymbolCallback GetSymbolCallback()
		{
			return symbol_callback;
		}
	}
}
