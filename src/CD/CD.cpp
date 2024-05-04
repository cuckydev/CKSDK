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

#include <CKSDK/CD.h>

#include <CKSDK/OS.h>
#include <CKSDK/ExScreen.h>

#include <CKSDK/TTY.h>

#include <CKSDK/Util/Queue.h>

namespace CKSDK
{
	namespace CD
	{
		// CD constants
		static constexpr unsigned COMPLETE_TIMEOUT = 0x100000;

		// CD globals
		Region g_region;
		
		static uint8_t last_command = 0;
		static uint8_t last_status = 0;
		static uint8_t last_error = 0;

		static Callback complete_callback = nullptr;
		static Callback ready_callback = nullptr;
		static Callback end_callback = nullptr;

		static struct
		{
			IRQStatus last_irq_status = IRQStatus::NoIRQ;
			Result last_result = {};
			
			void Handle(IRQStatus irq_status, Result result)
			{
				last_irq_status = irq_status;
				last_result = result;
			}
		} handle_acknowledge, handle_complete, handle_ready, handle_end;

		static volatile bool complete_pending = false, dispatch_pending = false;
		static volatile bool media_changed = false;

		// CD command map
		enum CommandFlag
		{
			Status       = 1 << 0,
			Blocking     = 1 << 1,
			SectorBuffer = 1 << 2
		};

		// https://problemkaputt.de/psx-spx.htm#cdromcontrollercommandsummary
		static const uint8_t command_flag[] = {
			0,
			CommandFlag::Status,                             // Nop
			CommandFlag::Status,                             // SetLoc
			CommandFlag::Status,                             // Play
			0,
			CommandFlag::Status,                             // Forward
			CommandFlag::Status,                             // Backward
			CommandFlag::Status | CommandFlag::SectorBuffer, // ReadN
			CommandFlag::Status | CommandFlag::Blocking,     // Standby

			CommandFlag::Status | CommandFlag::Blocking,     // Stop
			CommandFlag::Status | CommandFlag::Blocking,     // Pause
			CommandFlag::Status | CommandFlag::Blocking,     // Init
			CommandFlag::Status,                             // Mute
			0,
			CommandFlag::Status,                             // Unmute
			CommandFlag::Status,                             // SetFilter
			CommandFlag::Status,                             // SetMode
			CommandFlag::Status,                             // GetParam
			0,
			0,                                               // GetLocL
			0,                                               // GetLocP
			CommandFlag::Status | CommandFlag::Blocking,     // SetSession
			CommandFlag::Status,                             // GetTN
			0,
			CommandFlag::Status,                             // GetTD
			CommandFlag::Status | CommandFlag::Blocking,     // SeekL
			CommandFlag::Status | CommandFlag::Blocking,     // SeekP
			0,
			0,
			0,
			0,                                               // Test
			CommandFlag::Status | CommandFlag::Blocking,     // GetID
			CommandFlag::Status | CommandFlag::SectorBuffer, // ReadS

			CommandFlag::Status,                             // Reset
			CommandFlag::Status | CommandFlag::Blocking,     // GetQ
			CommandFlag::Status | CommandFlag::Blocking      // ReadTOC
		};

		// CD command queue
		struct CommandQueueArgs
		{
			Command com;
			Callback complete_cb;
			Callback ready_cb;
			Callback end_cb;
			uint8_t param[8];
			unsigned length;
		};

		static Queue::Queue<CommandQueueArgs, 16> command_queue;

		static bool CommandQueue_Issue(const CommandQueueArgs &args)
		{
			uint8_t com = uint8_t(args.com);

			// Set callbacks
			complete_callback = args.complete_cb;
			ready_callback = args.ready_cb;
			end_callback = args.end_cb;

			// Issue command
			last_command = com;

			complete_pending = true;
			dispatch_pending = true;

			// Reset parameter buffer
			while (OS::CdStat() & (1 << 7)); // Wait for command/parameter transfer to clear

			OS::CdStat() = 0x01;
			OS::CdIrq() = 0x40;
			// OS::WaitCycles(50); // Wait for parameter buffer reset

			// Wait for CD drive to be ready
			while (OS::CdStat() & ((1 << 7) | (1 << 6) | (1 << 5)));

			// Acknowledge all IRQs
			OS::CdStat() = 0x01;
			OS::CdIrq() = 0x1F;

			// Write parameters
			OS::CdStat() = 0;

			unsigned length = args.length;
			if (length != 0)
			{
				const volatile uint8_t *pp = args.param;
				for (; length != 0; length--)
				{
					// OS::CdStat() = 0;
					OS::CdData() = *pp++;
				}
			}

			// Write command
			OS::CdStat() = 0;
			OS::CdCmd() = com;

			return true;
		}

		// CD IRQ
		static void UpdateStatus(uint8_t status)
		{
			// Update last status
			uint8_t last = last_status;
			last_status = status;

			// Check status error bits
			if (!(last & Status::Error) && (status & Status::Error))
			{
				TTY::Out("CD drive error\n");
				last_error = last_status;
			}
			if (!(last & Status::ShellOpen) && (status & Status::ShellOpen))
			{
				TTY::Out("CD media changed\n");
				media_changed = true;
			}
		}

		enum HandleIRQ
		{
			HandleAcknowledge = (1 << 0),
			HandleComplete    = (1 << 1),
			HandleDataReady   = (1 << 2),
			HandleDataEnd     = (1 << 3)
		};
		static unsigned IRQ_HandleIRQ()
		{
			// Request interrupt status
			OS::CdStat() = 0x01;
			IRQStatus irq_status = IRQStatus(OS::CdIrq() & 0x07);
			if (irq_status == IRQStatus::NoIRQ)
				return 0;
			while (irq_status != IRQStatus(OS::CdIrq() & 0x07))
				irq_status = IRQStatus(OS::CdIrq() & 0x07);
			
			// Request result
			Result result;
			unsigned results = 0;

			uint8_t first_byte = 0;
			if ((OS::CdStat() & (1 << 5)) != 0)
			{
				// Read first byte (status)
				first_byte = OS::CdCmd();
				results++;

				// Read bytes to result pointer
				auto *resultp = &result[0];
				auto *resulte = &result[std::size(result.b)];
				if (resultp != nullptr)
				{
					*resultp++ = first_byte;
					while (resultp != resulte && (OS::CdStat() & (1 << 5)) != 0)
					{
						*resultp++ = OS::CdCmd();
						results++;
					}
					while (resultp != resulte)
						*resultp++ = 0;
				}

				// Flush result
				uint8_t reg;
				while (OS::CdStat() & (1 << 5))
					reg = OS::CdCmd();
			}

			// Reset and mask interrupt causes
			OS::CdStat() = 0x01;
			OS::CdIrq() = 0x07;
			OS::CdData() = 0x07;

			// Update status
			uint8_t status_error = 0;
			if (irq_status != IRQStatus::Acknowledge || (command_flag[last_command] & CommandFlag::Status) != 0)
			{
				UpdateStatus(first_byte);
				status_error = first_byte & 0x1D;
			}

			switch (irq_status)
			{
				case IRQStatus::DataReady:
				{
					// Handle IRQ
					if ((status_error != 0) && results == 1) // DataReady with just status result isn't an error
						status_error = 0;
					if (status_error != 0)
						handle_ready.Handle(IRQStatus::DiscError, result);
					else
						handle_ready.Handle(IRQStatus::DataReady, result);
					return HandleIRQ::HandleDataReady;
				}
				case IRQStatus::Complete:
				{
					// Handle IRQ
					if (status_error != 0)
						handle_complete.Handle(IRQStatus::DiscError, result);
					else
						handle_complete.Handle(IRQStatus::Complete, result);
					return HandleIRQ::HandleComplete;
				}
				case IRQStatus::Acknowledge:
				{
					// Handle IRQ
					if (status_error == 0)
					{
						// If the command only signals an acknowledgement, redirect to a complete IRQ
						if ((command_flag[last_command] & CommandFlag::Blocking) == 0)
						{
							handle_complete.Handle(IRQStatus::Complete, result);
							return HandleIRQ::HandleComplete;
						}
						else
						{
							handle_acknowledge.Handle(IRQStatus::Acknowledge, result);
							return HandleIRQ::HandleAcknowledge;
						}
					}
					else
					{
						handle_complete.Handle(IRQStatus::DiscError, result);
						return HandleIRQ::HandleComplete;
					}
				}
				case IRQStatus::DataEnd:
				{
					// Handle IRQ
					handle_end.Handle(IRQStatus::DataEnd, result);
					handle_ready.Handle(IRQStatus::DataEnd, result);
					return HandleIRQ::HandleDataEnd | HandleIRQ::HandleDataReady;
				}
				case IRQStatus::DiscError:
				{
					// Handle IRQ
					handle_complete.Handle(IRQStatus::DiscError, result);
					handle_ready.Handle(IRQStatus::DiscError, result);
					return HandleIRQ::HandleComplete | HandleIRQ::HandleDataReady;
				}
				default:
				{
					return 0;
				}
			}
		}

		static void InterruptCallback()
		{
			uint8_t reg0 = OS::CdStat();
			while (1)
			{
				// Handle IRQ
				unsigned handle = IRQ_HandleIRQ();
				if (handle == 0)
					break;

				if ((handle & HandleIRQ::HandleComplete) != 0)
				{
					dispatch_pending = false;
					complete_pending = false;
					if (complete_callback != nullptr)
						complete_callback(handle_complete.last_irq_status, handle_complete.last_result);
				}
				if ((handle & HandleIRQ::HandleDataReady) != 0)
				{
					if (ready_callback != nullptr)
						ready_callback(handle_ready.last_irq_status, handle_ready.last_result);
				}
				if ((handle & HandleIRQ::HandleDataEnd) != 0)
				{
					if (end_callback != nullptr)
						end_callback(handle_end.last_irq_status, handle_end.last_result);
				}

				// Dispatch command, only if the callbacks didn't dispatch a new command
				if (!dispatch_pending)
					command_queue.Dispatch();
			}
			OS::CdStat() = reg0 & 0x03;
		}

		// CD functions
		KEEP void Init()
		{
			// Enable CD IRQ
			OS::DisableIRQ();
			OS::SetIRQ(OS::IRQ::CDROM, InterruptCallback);
			OS::EnableIRQ();

			// Setup CD
			OS::CdBIU() = 0
				| (3 << OS::BIU_WriteDelayShift)
				| (4 << OS::BIU_ReadDelayShift)
				| OS::BIU_Recovery
				| OS::BIU_PreStrobe
				| (2 << OS::BIU_SizeShift);

			OS::CdStat() = 0x01;
			OS::CdIrq() = 0x1F; // Acknowledge all IRQs
			OS::CdData() = 0x1F; // Enable all IRQs

			OS::CdStat() = 0x00;
			OS::CdIrq() = 0x00; // Clear any pending request

			// Enable CD DMA
			OS::DmaDpcr() = OS::DpcrSet(OS::DmaDpcr(), OS::DMA::CDROM, 3);
			OS::DmaCtrl(OS::DMA::CDROM).chcr = 0x00000000;

			// Initialize CD drive
			Issue(Command::Nop, nullptr, nullptr, nullptr, nullptr, 0);
			Issue(Command::Init, nullptr, nullptr, nullptr,nullptr, 0);
			media_changed = true;

			if (Sync(nullptr) == IRQStatus::DiscError)
				ExScreen::Abort("CD init failed");

			Issue(Command::Unmute, nullptr, nullptr, nullptr, nullptr, 0);

			// Request the required region from the CD drive
			{
				uint8_t param[1] = {0x22};

				g_region = Region::America;
				Issue(Command::Test, +[](IRQStatus status, const Result &result){
					// The CD drive will give us a string that represents which region it's designed for
					if (result[1] == 0x10)
						g_region = Region::Japan; // SPCH-1000 will return an error
					else if (!__builtin_memcmp(&result[0], "for Japan", 9))
						g_region = Region::Japan;
					else if (!__builtin_memcmp(&result[0], "for U/C", 7))
						g_region = Region::America;
					else if (!__builtin_memcmp(&result[0], "for Europe", 10))
						g_region = Region::Europe;
					else if (!__builtin_memcmp(&result[0], "for NETNA", 9) || !__builtin_memcmp(&result[0], "for NETEU", 9))
						g_region = Region::Worldwide;
					else if (!__builtin_memcmp(&result[0], "for US/AEP", 10))
						g_region = Region::Debug;
				}, nullptr, nullptr, param, sizeof(param));
				Sync(nullptr);
			}
		}

		KEEP void Issue(Command com, Callback complete_cb, Callback ready_cb, Callback end_cb, const uint8_t *param, unsigned length)
		{
			// Enqueue command
			CommandQueueArgs args;
			args.com = com;
			args.complete_cb = complete_cb;
			args.ready_cb = ready_cb;
			args.end_cb = end_cb;
			for (unsigned i = 0; i < length; i++)
				args.param[i] = param[i];
			args.length = length;
			command_queue.Enqueue(CommandQueue_Issue, args);
		}
		
		KEEP void GetSector(void *addr, size_t size)
		{
			// Unlock sector buffer
			OS::CdStat() = 0x00;
			OS::CdIrq() = 0x80;

			// Prepare sector buffer DMA
			OS::DmaCtrl(OS::DMA::CDROM).madr = (uint32_t)addr;
			OS::DmaCtrl(OS::DMA::CDROM).bcr  = size | (1 << 16);

			while ((OS::CdStat() & (1 << 6)) == 0); // Wait for sector buffer to be ready

			// Start sector buffer DMA
			OS::DmaCtrl(OS::DMA::CDROM).chcr = 0x11000000;

			// Wait for sector buffer DMA to finish
			while (OS::DmaCtrl(OS::DMA::CDROM).chcr & (1 << 24));
		}

		KEEP IRQStatus QueueSync(uint8_t *result)
		{
			// Wait for command queue to clear up
			command_queue.Sync();

			// Return status
			if (result != nullptr)
				result[0] = last_status;
			return handle_complete.last_irq_status;
		}

		KEEP IRQStatus Sync(uint8_t *result)
		{
			// Wait for command queue to clear up
			command_queue.Sync();
			
			// Wait for sync to clear
			for (unsigned i = COMPLETE_TIMEOUT; i != 0; i--)
			{
				if (!complete_pending)
				{
					// Return status
					if (result != nullptr)
						result[0] = last_status;
					return handle_complete.last_irq_status;
				}
			}
			
			TTY::Out("CD sync timeout\n");
			return IRQStatus::NoIRQ;
		}

		// Play track
		KEEP void PlayTrack(uint8_t track, Callback report_cb, Callback end_cb)
		{
			// Play track
			{
				uint8_t param[1] = { Mode::AutoPause };
				if (report_cb != nullptr)
					param[0] |= Mode::Report;
				Issue(Command::SetMode, nullptr, nullptr, nullptr, param, 1);
			}
			{
				uint8_t param[1] = { track };
				Issue(Command::Play, nullptr, report_cb, end_cb, param, 1);
			}
		}

		// Read sectors
		static ReadCallback read_callback = nullptr;

		static void *read_start_addr = nullptr, *read_addr = nullptr;
		static size_t read_start_sectors = 0;
		static volatile size_t read_sectors = 0;

		static void ReadyCallback_Read(IRQStatus status, const Result &result)
		{
			// Check status
			if (status != CD::IRQStatus::DataReady)
				return;
			
			// Get sector from CD drive
			CD::GetSector(read_addr, 2048 / 4);
			read_addr = (char*)read_addr + 2048;

			// Decrement sectors
			uint32_t sectors = read_sectors;
			read_sectors = --sectors;

			if (sectors == 0)
			{
				// Pause and call read callback
				Issue(Command::Pause, nullptr, nullptr, nullptr, nullptr, 0);
				if (read_callback != nullptr)
					read_callback(read_start_addr, read_start_sectors);
			}
		}

		KEEP void ReadSectors(ReadCallback cb, void *addr, const CD::Loc &loc, size_t sectors, uint8_t mode)
		{
			// Setup read
			ReadSync();

			read_callback = cb;

			read_start_addr = addr;
			read_start_sectors = sectors;

			read_sectors = sectors;
			read_addr = addr;
			
			// Start reading
			uint8_t param[1] = { mode };
			Issue(Command::SetMode, nullptr, nullptr, nullptr, param, 1);
			Issue(Command::SetLoc, nullptr, nullptr, nullptr, loc.param, sizeof(loc.param));
			Issue(Command::ReadN, nullptr, ReadyCallback_Read, nullptr, nullptr, 0);
		}
		
		KEEP void ReadSectors(ReadCallback cb, void *addr, const CD::File &file, uint8_t mode)
		{
			ReadSectors(cb, addr, file.loc, file.Sectors(), mode);
		}

		KEEP void ReadSync()
		{
			// Wait for read sectors to reach 0
			while (read_sectors != 0);
		}
	}
}

#endif
