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

/// @file CKSDK/DLL.h
/// @brief CKSDK DLL API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>
#include <CKSDK/ELF.h>

#include <memory>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK DLL namespace
	namespace DLL
	{
		// DLL types
		/// @brief DLL class
		class DLL
		{
			private:
				/// @brief DLL data pointer
				std::unique_ptr<char[]> ptr;
				uintptr_t PtrInt() { return (uintptr_t)ptr.get(); }

				/// @brief DLL data size
				size_t size;

				/// @brief ELF hash pointer
				const uint32_t *hash;
				/// @brief ELF GOT pointer
				uint32_t *got;
				/// @brief ELF symtab pointer
				ELF::Elf32_Sym *symtab;
				/// @brief ELF strtab pointer
				const char *strtab;
				/// @brief ELF symbol count
				uint32_t symbol_count;

				/// @brief ELF first GOT symbol
				uint32_t first_got_symbol;
				/// @brief ELF local GOT symbol count
				uint32_t got_local_count;
				/// @brief ELF external GOT symbol count
				uint32_t got_extern_count;

				// @brief Resolver callback
				static void Resolver(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3);

			public:
				/// @brief DLL from data blob
				/// @param _ptr Data pointer
				/// @param _size Data size
				DLL(std::unique_ptr<char[]> _ptr, size_t _size);
				/// @brief Destructor
				/// @note This does not free the data blob or restore it to its original state
				~DLL();

				/// @brief Get symbol address
				/// @param name Symbol name
				/// @return Symbol address
				void *GetSymbol(const char *name);
		};

		// DLL Functions
		/// @brief Initialize DLL API
		/// @note For internal use only
		void Init();

		/// @brief Symbol callback type
		/// @param name Symbol name
		/// @return Symbol address
		/// @details This callback should return the address of the symbol with the given name
		typedef OS::Function<void*, const char*> SymbolCallback;

		/// @brief Set symbol callback
		/// @param cb Symbol callback
		/// @return Previous symbol callback
		SymbolCallback SetSymbolCallback(SymbolCallback cb);
		/// @brief Get symbol callback
		/// @return Symbol callback
		SymbolCallback GetSymbolCallback();
	}
}
