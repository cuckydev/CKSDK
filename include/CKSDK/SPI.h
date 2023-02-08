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

/// @file CKSDK/SPI.h
/// @brief CKSDK SPI driver API

#pragma once

#include <CKSDK/CKSDK.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK SPI driver namespace
	namespace SPI
	{
		// SPI enums
		/// @brief Pad buttons
		enum PadButton
		{
			Select   = (1 << 0),
			L3       = (1 << 1),
			R3       = (1 << 2),
			Start    = (1 << 3),
			Up       = (1 << 4),
			Right    = (1 << 5),
			Down     = (1 << 6),
			Left     = (1 << 7),
			L2       = (1 << 8),
			R2       = (1 << 9),
			L1       = (1 << 10),
			R1       = (1 << 11),
			Triangle = (1 << 12),
			Circle   = (1 << 13),
			Cross    = (1 << 14),
			Square   = (1 << 15)
		};

		/// @brief SPI commands
		enum Command
		{
			/// @brief Read pad state (exchange poll request/response)
			PadRead = 'B',
			/// @brief Toggle DualShock configuration mode
			PadConfigMode = 'C',
			
			/// @brief Initialize DS2 button pressure sensors (in config mode)
			/// @note This is a DualShock 2 command. The DualShock 1 will simply ignore it
			PadInitPressure = '@',
			/// @brief Set analog mode/LED state (in config mode)
			PadSetAnalog = 'D',
			/// @brief Get analog mode/LED state (in config mode)
			PadGetAnalog = 'E',
			/// @brief Get information about a vibration motor (in config mode)
			PadGetMotorInfo = 'F',
			/// @brief Get list of all vibration motors (in config mode)
			PadGetMotorList = 'G',
			/// @brief Get current state of vibration motors (in config mode)
			PadGetMotorState = 'H',
			/// @brief Get list of supported controller modes? (in config mode)
			PadGetMode = 'L',
			/// @brief Configure poll request format (in config mode)
			PadRequestConfig = 'M',
			/// @brief Configure poll response format (in config mode)
			/// @note This is a DualShock 2 command. The DualShock 1 will simply ignore it
			PadResponseConfig = 'O',

			/// @brief Read 128-byte sector
			CardReadSector = 'R',
			/// @brief Retrieve ID and card size information
			/// @note This command is only supported by official Sony cards
			CardIdentify = 'S',
			/// @brief Erase and write 128-byte sector
			CardWriteSector = 'W'
		};

		/// @brief Card status codes
		enum CardStatus
		{
			/// @brief Card is OK
			CardOk = 'G',
			/// @brief Card checksum failed
			CardBadChecksum = 'N',
			/// @brief Card bad sector read
			CardBadSector = 0xFF
		};

		/// @brief Card status flags
		enum CardStatusFlag
		{
			/// @brief Last write command failed
			CardWriteError = 1 << 2,
			/// @brief No writes have been issued yet
			CardNotWritten = 1 << 3,
			/// @brief Unknown
			/// @note Might be set on third-party cards
			CardUnknown = 1 << 4
		};

		/// @brief Pad IDs
		enum PadID
		{
			/// @brief Sony PS1 mouse
			Mouse = 0x1,
			/// @brief Namco neGcon
			/// @note Currently unsupported
			neGcon = 0x2,
			/// @brief "Konami" lightgun without composite video passthrough
			/// @note Currently unsupported
			IRQ10Gun = 0x3,
			/// @brief Digital pad or Dual Analog/DualShock in digital mode
			Digital = 0x4,
			/// @brief Flight stick or Dual Analog in green LED mode
			AnalogStick = 0x5,
			/// @brief Namco Guncon (lightgun with composite video passthrough)
			/// @note Currently unsupported
			Guncon = 0x6,
			/// @brief Dual Analog/DualShock in analog (red LED) mode
			Analog = 0x7,
			/// @brief Multitap adapter
			/// @note Currently unsupported
			Multitap = 0x8,
			/// @brief Namco Guncon2 (lightgun with composite video passthrough)
			/// @note Currently unsupported
			Jogcon = 0xE,

			/// @brief Dual Analog/DualShock in config mode
			ConfigMode = 0xF,
			/// @brief No pad connected
			None = 0x10 // No pad connected (if len == 0xF)
		};

		/// @brief Pad request packet
		struct PadPacket
		{
			/// @brief Must be 0x01 (or 02/03/04 for multitap pads)
			uint8_t addr;
			/// @brief Command
			uint8_t cmd;
			/// @brief 0x01 to enable multitap response
			uint8_t tap_mode;
			/// @brief Right motor control (on/off)
			uint8_t motor_r;
			/// @brief Left motor control (PWM)
			uint8_t motor_l;
			uint8_t pad[4];
		};
		static_assert(sizeof(PadPacket) == 9);

		/// @brief Pad response packet
		struct PadResponse
		{
			/// @brief Pad ID
			uint8_t id;
			/// @brief Must be 0x5A
			uint8_t prefix;

			union
			{
				/// @brief Digital controller
				struct
				{
					/// @brief Button state
					uint16_t btn;
				} digital;
				/// @brief Analog controller / DualShock
				struct
				{
					/// @brief Button state
					uint16_t btn;
					/// @brief Right stick X
					uint8_t rs_x;
					/// @brief Right stick Y
					uint8_t rs_y;
					/// @brief Left stick X
					uint8_t ls_x;
					/// @brief Left stick Y
					uint8_t ls_y;
					/// @brief Button pressure (DualShock 2 only)
					uint8_t press[12];
				} analog;
				/// @brief Mouse
				struct
				{
					/// @brief Button state
					uint16_t btn;
					/// @brief X movement of mouse
					int8_t x_mov;
					/// @brief Y movement of mouse
					int8_t y_mov;
				} mouse;
				/// @brief neGcon
				struct
				{
					/// @brief Button state
					uint16_t btn;
					/// @brief Controller twist
					uint8_t twist;
					/// @brief I button value
					uint8_t btn_i;
					/// @brief II button value
					uint8_t btn_ii;
					/// @brief L trigger value
					uint8_t trg_l;
				} neg;
				/// @brief Jogcon
				struct
				{
					/// @brief Button state
					uint16_t btn;
					/// @brief Jog rotation
					uint16_t rot;
				} jog;
				/// @brief Guncon
				struct
				{
					/// @brief Button state
					uint16_t btn;
					/// @brief Gun X position in dotclocks
					uint16_t x;
					/// @brief Gun Y position in scanlines
					uint16_t y;
				} gun;
				/// @brief Payload
				uint8_t payload[32];
			};

			// Methods
			/// @brief Reset pad state
			void Reset()
			{
				id = 0xFF;
				prefix = 0xFF;
			}

			/// @brief Get length of response from ID
			/// @return Response length
			uint8_t GetLen()
			{
				uint8_t len = id & 0xF;
				if (len == 0)
					return 0x10;
				return len;
			}
			/// @brief Get type of response from ID
			/// @return Response type
			uint8_t GetType()
			{
				return id >> 4;
			}

			/// @brief Get pad ID from response
			/// @return PadID
			PadID GetPadID()
			{
				uint8_t len = GetLen();
				uint8_t type = GetType();
				if (prefix == 0x5A || prefix == 0x00)
				{
					if (type < 0x0F)
						return PadID(type);
					if (id == 0xF3)
						return PadID::ConfigMode;
				}
				return PadID::None;
			}
		};
		static_assert(sizeof(PadResponse) == 34);

		/// @brief General pad state
		struct Pad
		{
			// Pad state
			/// @brief Pad ID
			PadID id;
			/// @brief Buttons held
			/// @see PadButton
			uint16_t held;
			/// @brief Buttons pressed since last PollPads()
			/// @see PadButton
			uint16_t press;
			/// @brief Buttons released since last PollPads()
			/// @see PadButton
			uint16_t release;
			/// @brief Left stick X
			uint8_t ls_x;
			/// @brief Left stick Y
			uint8_t ls_y;
			/// @brief Right stick X
			uint8_t rs_x;
			/// @brief Right stick Y
			uint8_t rs_y;

			// Methods
			/// @brief Reset pad state
			void Reset()
			{
				id = PadID::None;
				held = press = release = 0x0000;
				ls_x = ls_y = rs_x = rs_y = 0x00;
			}

			/// @brief Set pad state from digital response
			/// @param pad Digital pad response
			void SetDigital(PadResponse &pad)
			{
				id = PadID::Digital;
				uint16_t nheld = ~pad.digital.btn;
				press = (nheld ^ held) & nheld;
				release = (nheld ^ held) & held;
				held = nheld;
				ls_x = ls_y = rs_x = rs_y = 0x00;
			}
			/// @brief Set pad state from analog response
			/// @param pad Analog pad response
			void SetAnalog(PadResponse &pad)
			{
				id = PadID::Analog;
				uint16_t nheld = ~pad.digital.btn;
				press = (nheld ^ held) & nheld;
				release = (nheld ^ held) & held;
				held = nheld;
				ls_x = pad.analog.ls_x - 0x80;
				ls_y = pad.analog.ls_y - 0x80;
				rs_x = pad.analog.rs_x - 0x80;
				rs_y = pad.analog.rs_y - 0x80;
			}
		};

		/// @brief Global pad states
		extern Pad g_pad[2];

		// SPI functions
		/// @brief Initialize SPI driver
		/// @note For internal use only
		void Init();

		/// @brief Poll pad states and update global pad states
		/// @see g_pad
		void PollPads();
	}
}
