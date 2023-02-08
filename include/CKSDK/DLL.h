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
			public:
				/// @brief DLL data address
				/// @note For internal use only
				void *ptr;
				/// @brief DLL data size
				/// @note For internal use only
				size_t size;

				/// @brief ELF hash address
				/// @note For internal use only
				const uint32_t *hash;
				/// @brief ELF GOT address
				/// @note For internal use only
				uint32_t *got;
				/// @brief ELF symtab address
				/// @note For internal use only
				ELF::Elf32_Sym *symtab;
				/// @brief ELF strtab address
				/// @note For internal use only
				const char *strtab;
				/// @brief ELF symbol count
				/// @note For internal use only
				uint32_t symbol_count;

				/// @brief ELF first GOT symbol
				/// @note For internal use only
				uint32_t first_got_symbol;
				/// @brief ELF local GOT symbol count
				/// @note For internal use only
				uint32_t got_local_count;
				/// @brief ELF external GOT symbol count
				/// @note For internal use only
				uint32_t got_extern_count;

			public:
				/// @brief DLL from data blob
				/// @param _ptr Data address
				/// @param _size Data size
				/// @note The data blob is not copied, so it must remain valid for the lifetime of the DLL object
				DLL(void *_ptr, size_t _size);
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
