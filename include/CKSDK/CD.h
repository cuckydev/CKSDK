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

/// @file CKSDK/CD.h
/// @brief CKSDK CD API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK CD namespace
	namespace CD
	{
		// CD types
		/// @brief CD command result
		/// @details This is simply an array of 8 bytes
		struct Result
		{
			private:
				uint8_t b[8];

			public:
				uint8_t &operator[](int i) { return b[i]; }
				const uint8_t &operator[](int i) const { return b[i]; }
		};

		/// @brief BCD encoded byte
		struct BCD
		{
			/// @brief BCD encoded byte
			uint8_t b;
			
			/// @brief Decode BCD encoded byte
			/// @return	Decoded byte
			uint8_t Dec() const
			{
				return b - 6 * (b >> 4);
			}
			/// @brief Encode byte to BCD
			/// @param dec	Byte to encode
			/// @return	BCD encoded byte
			static BCD Enc(uint8_t dec)
			{
				BCD bcd;
				bcd.b = ((dec / 10) << 4) | (dec % 10);
				return bcd;
			}
		};

		/// @brief CD location
		struct Loc
		{
			/// @brief BCD encoded location
			union
			{
				/// @brief BCD encoded MSF
				struct
				{
					/// @brief BCD encoded minute
					BCD minute;
					/// @brief BCD encoded second
					BCD second;
					/// @brief BCD encoded sector (75 sectors per second)
					BCD sector;
				} bcd;
				/// @brief BCD encoded location as array
				uint8_t param[3];
			};

			/// @brief Decode BCD encoded location
			/// @return Decoded location
			uint32_t Dec() const
			{
				return (
					(uint32_t(bcd.minute.Dec()) * (75 * 60)) +
					(uint32_t(bcd.second.Dec()) * 75) +
					(uint32_t(bcd.sector.Dec()))
				) - (75 * 2);
			}
			/// @brief Encode location to BCD
			/// @param dec Location to encode
			static Loc Enc(uint32_t dec)
			{
				Loc loc;
				dec += (75 * 2);
				loc.bcd.minute = BCD::Enc(dec / (75 * 60));
				loc.bcd.second = BCD::Enc((dec / 75) % 60);
				loc.bcd.sector = BCD::Enc(dec % 75);
				return loc;
			}
		};

		/// @brief CD file
		struct File
		{
			/// @brief File location
			Loc loc;
			/// @brief File size in bytes
			size_t size;
			
			/// @brief Get file size in sectors
			size_t Sectors() const { return (size + 0x7FF) >> 11; }
			/// @brief Get file size on disc in bytes
			size_t Size() const { return Sectors() << 11; }
		};

		// CD enums
		/// @brief CD region
		enum class Region
		{
			/// @brief Japan
			Japan     = 0,
			/// @brief North America
			America   = 1,
			/// @brief Europe
			Europe    = 2,
			/// @brief Worldwide (Net Yaroze)
			Worldwide = 3,
			/// @brief Debug
			Debug     = 4
		};

		/// @brief IRQ status
		enum class IRQStatus : uint8_t
		{
			/// @brief No IRQs
			NoIRQ       = 0,
			/// @brief Data ready
			DataReady   = 1,
			/// @brief Command complete
			Complete    = 2,
			/// @brief Command acknowledged
			Acknowledge = 3,
			/// @brief End of data
			DataEnd     = 4,
			/// @brief Disc error
			DiscError   = 5
		};

		/// @brief CD commands
		enum class Command : uint8_t
		{
			/// @brief No-operation
			Nop        = 0x01,
			/// @brief Set location
			SetLoc     = 0x02,
			/// @brief Play digital audio
			Play       = 0x03,
			/// @brief Fast-forward digital audio
			Forward    = 0x04,
			/// @brief Rewind digital audio
			Backward   = 0x05,
			/// @brief Read with retry
			ReadN      = 0x06,
			/// @brief Standby CD drive
			Standby    = 0x07,
			/// @brief Stop CD drive
			Stop       = 0x08,
			/// @brief Pause reading
			Pause      = 0x09,
			/// @brief Initialize CD drive
			Init       = 0x0A,
			/// @brief Mute digital audio
			Mute       = 0x0B,
			/// @brief Unmute digital audio
			Unmute     = 0x0C,
			/// @brief Set XA filter
			SetFilter  = 0x0D,
			/// @brief Set mode
			SetMode    = 0x0E,
			/// @brief Get parameter
			GetParam   = 0x0F,
			/// @brief Get logical location
			GetLocL    = 0x10,
			/// @brief Get physical location
			GetLocP    = 0x11,
			/// @brief Set session
			SetSession = 0x12,
			/// @brief Get first and last track number for this session
			GetTN      = 0x13,
			/// @brief Get track location for this session
			GetTD      = 0x14,
			/// @brief Seek to logical location
			SeekL      = 0x15,
			/// @brief Seek to physical location
			SeekP      = 0x16,
			/// @brief Test commands
			/// @see https://psx-spx.consoledev.net/cdromdrive/#cdrom-test-commands
			Test       = 0x19,
			/// @brief Get ID
			GetID      = 0x1A,
			/// @brief Read without retry
			ReadS      = 0x1B,
			/// @brief Reset CD drive
			Reset      = 0x1C,
			/// @brief Get Q subchannel
			GetQ       = 0x1D,
			/// @brief Read TOC
			ReadTOC    = 0x1E
		};

		/// @brief CD mode flags
		enum Mode
		{
			/// @brief Play digital audio
			DigitalAudio = 1 << 0,
			/// @brief Auto pause at end of track
			AutoPause = 1 << 1,
			/// @brief Digital audio report in ready callback
			/// @see DAReport
			Report = 1 << 2,
			/// @brief Only use XA filter
			XAFilter = 1 << 3,
			/// @brief Ignore sector size and SetLoc position
			/// @note What?
			Ignore = 1 << 4,
			/// @brief Read full 2340 byte sector
			Size = 1 << 5,
			/// @brief Send XA to SPU audio input
			XAInput = 1 << 6,
			/// @brief Read at 2x speed
			Speed = 1 << 7
		};

		/// @brief CD status flags
		enum Status
		{
			/// @brief Error occurred
			Error     = 1 << 0,
			/// @brief Drive is in standby
			Standby   = 1 << 1,
			/// @brief Seek error occured
			SeekError = 1 << 2,
			/// @brief ID error occured
			IdError   = 1 << 3,
			/// @brief Shell open
			ShellOpen = 1 << 4,
			/// @brief Reading
			Read      = 1 << 5,
			/// @brief Seeking
			Seek      = 1 << 6,
			/// @brief Playing digital audio
			Play      = 1 << 7
		};

		// CD globals
		/// @brief CD drive region
		extern Region g_region;

		// CD functions
		/// @brief Initialize CD drive
		/// @note For internal use only
		void Init();

		/// @brief CD callback type
		/// @param status IRQ status
		/// @param result Result of command
		typedef OS::Function<void, IRQStatus, const Result&> Callback;
		/// @brief Issue a command to the CD drive
		/// @param com Command to issue
		/// @param complete_cb Complete callback
		/// @param ready_cb Ready callback
		/// @param end_cb End callback
		/// @param param Command parameters
		/// @param length Length of command parameters
		void Issue(Command com, Callback complete_cb, Callback ready_cb, Callback end_cb, const uint8_t *param, unsigned length);

		/// @brief Read sector from CD drive
		/// @param addr Address to read to
		/// @param size Size of sector to read in words
		/// @note This function may only be called from ready callback
		/// @note You must read all 2048 bytes of the sector for each ready callback, or the drive will fall out of sync
		/// @see Issue
		void GetSector(void *addr, size_t size);

		/// @brief Wait for CD command queue to be empty
		/// @param result Result of last command
		/// @return Last IRQ status
		/// @details This function will wait for the command queue to be empty, but not for the last command to complete
		IRQStatus QueueSync(uint8_t *result);
		/// @brief Wait for CD to be idle
		/// @param result Result of last command
		/// @return Last IRQ status
		/// @details This function will wait for both the command queue to be empty, and for the last command to complete
		IRQStatus Sync(uint8_t *result);

		/// @brief Digital audio report result
		/// @see PlayTrack
		union DAReport
		{
			struct
			{
				/// @brief CD status
				uint8_t stat;
				/// @brief Track number
				uint16_t track;
				/// @brief Index number
				uint16_t index;
				/// @brief Minutes (BCD)
				BCD minute;
				/// @brief Seconds (BCD)
				/// @details Most significant bit is set if the MSF is local to the current track
				BCD second;
				/// @brief Sectors (BCD)
				BCD sector;
				/// @brief Peak low
				uint8_t peak_lo;
				/// @brief Peak high
				uint8_t peak_hi;
			} result;
			/// @brief Result
			uint8_t b[8];
		};
		/// @brief Play a track
		/// @param report_cb Digital audio report callback
		/// @param end_cb Track end callback
		/// @param track Track to play
		void PlayTrack(Callback report_cb, Callback end_cb, uint8_t track);

		/// @brief Read callback type
		/// @param addr Address read to
		/// @param sectors Number of sectors read
		typedef OS::Function<void, void*, size_t> ReadCallback;
		/// @brief Read sectors from CD drive
		/// @param cb Read completed callback
		/// @param addr Address to read to
		/// @param loc Location to read from
		/// @param sectors Number of sectors to read
		/// @param mode Mode
		void ReadSectors(ReadCallback cb, void *addr, const CD::Loc &loc, size_t sectors, uint8_t mode);
		/// @brief Read sectors from CD drive
		/// @param cb Read completed callback
		/// @param addr Address to read to
		/// @param file File to read from
		/// @param mode Mode
		/// @overload
		void ReadSectors(ReadCallback cb, void *addr, const CD::File &file, uint8_t mode);
		/// @brief Wait for read to complete
		void ReadSync();
	}
}
