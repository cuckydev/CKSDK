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

#include <CKSDK/SPI.h>

#include <CKSDK/OS.h>
#include <CKSDK/TTY.h>

namespace CKSDK
{
	namespace SPI
	{
		// SPI constants
		static constexpr unsigned SYNC_TIMEOUT = 0x10000;

		static constexpr unsigned CONFIGURE_ATTEMPTS = 10;

		// SPI globals
		Pad g_pad[2];

		static bool error = false;

		static struct
		{
			PadResponse pad;
			unsigned configure = 0;	
		} pad[2];
		static auto *padp = &pad[0];

		// SPI functions
		void Init()
		{
			// Reset SIO
			OS::SioCtrl(0).ctrl = 0x0000;
			OS::SioCtrl(0).mode = 0x000D;
			OS::SioCtrl(0).baud = 0x0088;
		}

		static void StartPort(unsigned i)
		{
			// Caution: After slot selection (via Bits 1,13),
			// one should issue a delay before sending the the first data byte:
			// Digital Joypads may work without delay, Dualshock and Mouse
			// require at least some small delay, and older Analog Joypads
			// require a huge delay (around 500 clock cycles for SCPH-1150),
			// official kernel waits more than 2000 cycles (which is much more than needed).
			OS::SioCtrl(0).ctrl = 0x0000;
			OS::WaitCycles(1000);
			OS::SioCtrl(0).ctrl = 0x0007 | (i << 13);
			OS::WaitCycles(1000);
			padp = &pad[i];
		}

		static uint8_t Exchange(uint8_t com)
		{
			// Flush SPI
			while (OS::SioCtrl(0).stat & (1 << 1))
				OS::SioCtrl(0).fifo[0];
			
			// Send TX to SPI
			error = false;
			OS::SioCtrl(0).fifo[0] = com;

			// Wait for exchange to complete
			for (unsigned i = SYNC_TIMEOUT; i != 0; i--)
			{
				if ((OS::SioCtrl(0).stat & (1 << 1)) && (OS::SioCtrl(0).stat & (1 << 2)))
				{
					// Retrieve RX from SPI
					OS::WaitCycles(680 * 3); // TODO: I added this line and the bit2 check to fix digital controllers not connecting
					// ... I'm not sure which fixed it
					uint8_t rx = OS::SioCtrl(0).fifo[0];
					return rx;
				}
			}

			TTY::Out("SIO::Exchange timeout\n");
			error = true;
			return 0xFF;
		}

		static bool ExchangePacket(unsigned i, const void *packet, size_t size)
		{
			const uint8_t *packetp = (const uint8_t*)packet;
			const uint8_t *packete = packetp + size;

			// Start packet exchange
			StartPort(i);

			uint8_t hiz = Exchange(*packetp++);
			if (error)
			{
				padp->pad.Reset();
				return true;
			}
			
			// Start packet
			padp->pad.id = Exchange(*packetp++);
			padp->pad.prefix = Exchange(*packetp++);
			if (padp->pad.GetPadID() == PadID::None)
			{
				padp->pad.Reset();
				return true;
			}

			// Exchange packet
			uint8_t *payloadp = padp->pad.payload;
			uint8_t *payloade = payloadp + sizeof(padp->pad.payload);
			for (unsigned h = padp->pad.GetLen() << 1; h != 0; h--)
			{
				if (packetp != packete)
					*payloadp++ = Exchange(*packetp++);
				else
					*payloadp++ = Exchange(0xFF);
				if (error)
				{
					padp->pad.Reset();
					return true;
				}
			}
			// for (; payloadp != payloade; payloadp++)
			// 	*payloadp++ = 0x00;
			return false;
		}

		static void PollPad(unsigned i)
		{
			// Configure packets
			struct ConfigPacket
			{
				uint8_t addr;
				uint8_t cmd;
				uint8_t pad;
				uint8_t arg[2] = {};
			};
			static const ConfigPacket config_packets[] = {
				{ 0x01, Command::PadConfigMode,     0x00, 0x01, 0x00 },
				{ 0x01, Command::PadSetAnalog,      0x00, 0x01, 0x02 },
				{ 0x01, Command::PadInitPressure,   0x00, 0x00, 0x00 }, // Ignored by DualShock 1
				{ 0x01, Command::PadRequestConfig,  0x00, 0x00, 0x01 },
				{ 0x01, Command::PadResponseConfig, 0x00, 0xFF, 0xFF }, // Ignored by DualShock 1
				{ 0x01, Command::PadConfigMode,     0x00, 0x00, 0x00 },
			};

			// Send pad read packet
			PadPacket pad_packet;
			pad_packet.addr = 0x01;
			pad_packet.cmd = SPI::Command::PadRead;
			pad_packet.tap_mode = 0x00;
			pad_packet.motor_l = 0x00;
			pad_packet.motor_r = 0x00;
			ExchangePacket(i, &pad_packet, sizeof(pad_packet));

			// Attempt to configure analog controllers
			const ConfigPacket *config_packetp = config_packets;
			const ConfigPacket *config_packete = config_packets + std::size(config_packets);

			switch (padp->pad.GetPadID())
			{
				case PadID::Digital:
				{
					// Start configure controller
					if (padp->configure >= CONFIGURE_ATTEMPTS)
						break;
					padp->configure++;
					
					uint8_t oid = padp->pad.id;
					uint8_t oprefix = padp->pad.prefix;
					if (ExchangePacket(i, config_packetp, sizeof(ConfigPacket)))
					{
						// Controller didn't respond properly, restore state
						padp->pad.id = oid;
						padp->pad.prefix = oprefix;
					}
					break;
				}
				case PadID::ConfigMode:
				{
					// Run through configure packets
					padp->configure = CONFIGURE_ATTEMPTS;
					for (config_packetp++; config_packetp != config_packete; config_packetp++)
					{
						uint8_t oid = padp->pad.id;
						uint8_t oprefix = padp->pad.prefix;
						if (ExchangePacket(i, config_packetp, sizeof(ConfigPacket)))
						{
							// Controller didn't respond properly (DS2 commands on DS1?), restore state
							padp->pad.id = oid;
							padp->pad.prefix = oprefix;
						}
					}
					break;
				}
				case PadID::None:
				{
					// Clear configure flag
					padp->configure = 0;
					break;
				}
				default:
					break;
			}
		}

		void PollPads()
		{
			// Poll both pads
			for (unsigned i = 0; i < 2; i++)
			{
				PollPad(i);
				switch (padp->pad.GetPadID())
				{
					case PadID::Digital:
					case PadID::neGcon:
					case PadID::Jogcon:
						g_pad[i].SetDigital(padp->pad);
						break;
					case PadID::Analog:
					case PadID::AnalogStick:
						g_pad[i].SetAnalog(padp->pad);
						break;
					default:
						g_pad[i].Reset();
						break;
				}
			}
		}
	}
}
