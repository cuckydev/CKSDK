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

#ifndef CKSDK_NO_CD

#include <CKSDK/ISO.h>

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>
#include <CKSDK/STL.h>
#include <CKSDK/TTY.h>

namespace CKSDK
{
	namespace ISO
	{
		// ISO9660 globals
		static volatile uint32_t all_lba = 0;
		CD::File g_all;

		// ISO9660 types
		enum class VolumeDescriptorCode : uint8_t
		{
			BootRecord    = 0,
			Primary       = 1,
			Supplementary = 2,
			Volume        = 3,

			Terminator = 0xFF
		};

		enum FileFlags
		{
			Hidden = 1 << 0,
			Subdirectory = 1 << 1,
			AssociatedFile = 1 << 2,
			ExtendedAttribute = 1 << 3,
			ExtendedPermissions = 1 << 4,

			// ExtendedExtent = 1 << 7 // 4GiB+ files, unsupported
		};

		// The idea of the both integer formats is to allow all devices to directly address integers regardless of endianness
		// However, ISO9660 requires 2-byte aligned, but we're on hardware which requires word alignment. So close!
		struct Both16
		{
			uint16_t le, be;

			uint16_t operator()() { return le; } 
		};

		struct Both32
		{
			uint16_t le[2];
			uint16_t be[2];

			uint32_t operator()() { return ((uint32_t)le[0] << 0) | ((uint32_t)le[1] << 16); } 
		};

		struct VolumeDescriptor
		{
			VolumeDescriptorCode code;
			char cd001[5];
		};
		static_assert(sizeof(VolumeDescriptor) == 6);

		struct Directory
		{
			uint8_t length;
			uint8_t ear_length;
			Both32 extent_lba;
			Both32 extent_size;
			uint8_t time_date[7];
			uint8_t file_flags;
			uint8_t interleave_file_unit_size;
			uint8_t interleave_gap_size;
			Both16 volume_sequence_number;
			uint8_t name_length;
			char name[0];
		};

		// ISO9660 callbacks
		static void ReadCallback_ReadDirectory(void *addr, size_t sectors)
		{
			// Look for ALL in file
			char *dirp = (char*)addr;

			while (1)
			{
				// Get directory pointer
				Directory *dir = (Directory*)dirp;
				if (dir->length == 0)
					break;

				// Check name
				if (!STL::Memory::Compare(dir->name, "ALL;1", 3+2))
				{
					g_all.size = dir->extent_size();
					all_lba = dir->extent_lba();
					return;
				}

				// Increment directory pointer
				dirp += dir->length;
			}

			ExScreen::Abort("ALL not found");
			return;
		}

		static void ReadCallback_FindDirectory(void *addr, size_t sectors)
		{
			// Check if this is the primary volume descriptor
			VolumeDescriptor *volume_descriptor = (VolumeDescriptor*)addr;

			if (volume_descriptor->code == VolumeDescriptorCode::Terminator)
			{
				ExScreen::Abort("ISO no primary volume descriptor");
				return;
			}
			if (volume_descriptor->code != VolumeDescriptorCode::Primary)
				return;
			
			// Get root directory
			Directory *dir = (Directory*)((char*)addr + 0x9C);
			
			// Read directory
			CD::ReadSectors(ReadCallback_ReadDirectory, addr, CD::Loc::Enc(dir->extent_lba()), 1, CD::Mode::Speed);
		}

		// ISO9660 functions
		void Init()
		{
			// Read volume descriptors
			STL::UniquePointer<char[]> iso_buffer(new char[2048]);
			CD::ReadSectors(ReadCallback_FindDirectory, iso_buffer.Get(), CD::Loc::Enc(0x10), 1, CD::Mode::Speed);

			// Wait for ALL to be found
			while (all_lba == 0);
			CD::ReadSync();

			g_all.loc = CD::Loc::Enc(all_lba);
		}
	}
}

#endif
