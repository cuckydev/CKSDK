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

/// @file CKSDK/OS.h
/// @brief CKSDK OS API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/TTY.h>

// C externs
extern "C"
{
	void CKSDK_OS_DisableIRQ_asm();
	void CKSDK_OS_EnableIRQ_asm();
}

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK OS namespace
	namespace OS
	{
		// Memory map I/O port
		template <typename T>
		static inline constexpr volatile T &MMIO(uintptr_t addr)
		{
			return *(volatile T*)(0xBF800000 | addr);
		}

		// GPU
		inline constexpr volatile uint32_t &GpuGp0() { return MMIO<uint32_t>(0x1810); }
		inline constexpr volatile uint32_t &GpuGp1() { return MMIO<uint32_t>(0x1814); }

		// CD drive
		inline constexpr volatile uint8_t &CdStat() { return MMIO<uint8_t>(0x1800); }
		inline constexpr volatile uint8_t &CdCmd() { return MMIO<uint8_t>(0x1801); }
		inline constexpr volatile uint8_t &CdData() { return MMIO<uint8_t>(0x1802); }
		inline constexpr volatile uint8_t &CdIrq() { return MMIO<uint8_t>(0x1803); }
		
		inline constexpr volatile uint8_t &CdReg(uint8_t i) { return (&MMIO<uint8_t>(0x1800))[i]; }

		// SPU
		inline constexpr volatile uint16_t &SpuMasterVolL() { return MMIO<uint16_t>(0x1D80); }
		inline constexpr volatile uint16_t &SpuMasterVolR() { return MMIO<uint16_t>(0x1D82); }
		inline constexpr volatile uint16_t &SpuReverbVolL() { return MMIO<uint16_t>(0x1D84); }
		inline constexpr volatile uint16_t &SpuReverbVolR() { return MMIO<uint16_t>(0x1D86); }

		inline constexpr volatile uint32_t &SpuKeyOn() { return MMIO<uint32_t>(0x1D88); }
		inline constexpr volatile uint32_t &SpuKeyOff() { return MMIO<uint32_t>(0x1D8C); }

		inline constexpr volatile uint32_t &SpuFmMode() { return MMIO<uint32_t>(0x1D90); }
		inline constexpr volatile uint32_t &SpuNoiseMode() { return MMIO<uint32_t>(0x1D94); }
		inline constexpr volatile uint32_t &SpuReverbOn() { return MMIO<uint32_t>(0x1D98); }
		inline constexpr volatile uint32_t &SpuChanStatus() { return MMIO<uint32_t>(0x1D9C); }

		inline constexpr volatile uint16_t &SpuReverbAddr() { return MMIO<uint16_t>(0x1DA2); }
		inline constexpr volatile uint16_t &SpuIrqAddr() { return MMIO<uint16_t>(0x1DA4); }
		inline constexpr volatile uint16_t &SpuAddr() { return MMIO<uint16_t>(0x1DA6); }
		inline constexpr volatile uint16_t &SpuData() { return MMIO<uint16_t>(0x1DA8); }

		inline constexpr volatile uint16_t &SpuCtrl() { return MMIO<uint16_t>(0x1DAA); }
		inline constexpr volatile uint16_t &SpuDmaCtrl() { return MMIO<uint16_t>(0x1DAC); }
		inline constexpr volatile uint16_t &SpuStat() { return MMIO<uint16_t>(0x1DAE); }

		inline constexpr volatile uint16_t &SpuCdVolL() { return MMIO<uint16_t>(0x1DB0); }
		inline constexpr volatile uint16_t &SpuCdVolR() { return MMIO<uint16_t>(0x1DB2); }
		inline constexpr volatile uint16_t &SpuExtVolL() { return MMIO<uint16_t>(0x1DB4); }
		inline constexpr volatile uint16_t &SpuExtVolR() { return MMIO<uint16_t>(0x1DB6); }
		inline constexpr volatile uint16_t &SpuCurrentVolL() { return MMIO<uint16_t>(0x1DB8); }
		inline constexpr volatile uint16_t &SpuCurrentVolR() { return MMIO<uint16_t>(0x1DBA); }

		struct SPU_VOICE_CTRL_t
		{
			uint16_t vol_l, vol_r;
			uint16_t freq;
			uint16_t addr;
			uint16_t adsr1, adsr2;
			uint16_t loop_addr;
			uint16_t pad;
		};
		static_assert(sizeof(SPU_VOICE_CTRL_t) == 16);
		
		inline constexpr volatile SPU_VOICE_CTRL_t &SpuVoiceCtrl(int i) { return (&MMIO<SPU_VOICE_CTRL_t>(0x1D80))[i]; }

		// MDEC
		inline constexpr volatile uint32_t &Mdec0() { return MMIO<uint32_t>(0x1820); }
		inline constexpr volatile uint32_t &Mdec1() { return MMIO<uint32_t>(0x1824); }

		// SIOs
		struct SIO_CTRL_t
		{
			uint8_t fifo[4];
			uint16_t stat;
			uint16_t pad3;
			uint16_t mode;
			uint16_t ctrl;
			uint16_t reserved;
			uint16_t baud;
		};
		static_assert(sizeof(SIO_CTRL_t) == 16);

		inline constexpr volatile SIO_CTRL_t &SioCtrl(int i) { return (&MMIO<SIO_CTRL_t>(0x1040))[i]; }

		// IRQ controller
		/// @brief IRQ causes
		enum class IRQ
		{
			/// @brief VBlank
			VBLANK = 0,
			/// @brief GPU
			GPU    = 1,
			/// @brief CDROM
			CDROM  = 2,
			/// @brief DMA
			DMA    = 3,
			/// @brief Timer 0
			TIMER0 = 4,
			/// @brief Timer 1
			TIMER1 = 5,
			/// @brief Timer 2
			TIMER2 = 6,
			/// @brief SIO 0
			SIO0   = 7,
			/// @brief SIO 1
			SIO1   = 8,
			/// @brief SPU
			SPU    = 9,
			/// @brief PIO
			AUX    = 10
		};

		inline constexpr volatile uint16_t &IrqStat() { return MMIO<uint16_t>(0x1070); }
		inline constexpr volatile uint16_t &IrqMask() { return MMIO<uint16_t>(0x1074); }

		// DMA controller
		/// @brief DMA channels
		enum class DMA
		{
			/// @brief MDEC in
			MDECIN  = 0,
			/// @brief MDEC out
			MDECOUT = 1,
			/// @brief GPU
			GPU     = 2,
			/// @brief CDROM
			CDROM   = 3,
			/// @brief SPU
			SPU     = 4,
			/// @brief PIO
			PIO     = 5,
			/// @brief Ordering table (reverse)
			OTC     = 6
		};

		static constexpr uint32_t DpcrSet(uint32_t dpcr, DMA dma, uint32_t priority)
		{
			unsigned i = unsigned(dma);
			return (dpcr & ~(15 << (i << 2))) |
				(priority << (i << 2)) |
				(8 << (i << 2));
		}

		inline constexpr volatile uint32_t &DmaDpcr() { return MMIO<uint32_t>(0x10F0); }
		inline constexpr volatile uint32_t &DmaDicr() { return MMIO<uint32_t>(0x10F4); }

		struct DMA_CTRL_t
		{
			uint32_t madr;
			uint32_t bcr;
			uint32_t chcr;
			uint32_t pad;
		};
		static_assert(sizeof(DMA_CTRL_t) == 16);
		
		inline constexpr volatile DMA_CTRL_t &DmaCtrl(DMA i) { return (&MMIO<DMA_CTRL_t>(0x1080))[(int)i]; }
		
		// Timers
		struct TIMER_CTRL_t
		{
			uint32_t value;
			uint32_t ctrl;
			uint32_t reload;
			uint32_t pad;
		};
		static_assert(sizeof(TIMER_CTRL_t) == 16);

		inline constexpr volatile TIMER_CTRL_t &TimerCtrl(int i) { return (&MMIO<TIMER_CTRL_t>(0x1100))[i]; }

		// Memory control
		inline constexpr volatile uint32_t &Exp1Addr() { return MMIO<uint32_t>(0x1000); }
		inline constexpr volatile uint32_t &Exp2Addr() { return MMIO<uint32_t>(0x1004); }

		inline constexpr volatile uint32_t &Exp1DelaySize() { return MMIO<uint32_t>(0x1008); }
		inline constexpr volatile uint32_t &Exp3DelaySize() { return MMIO<uint32_t>(0x100C); }
		inline constexpr volatile uint32_t &BiosDelaySize() { return MMIO<uint32_t>(0x1010); }
		inline constexpr volatile uint32_t &SpuDelaySize() { return MMIO<uint32_t>(0x1014); }
		inline constexpr volatile uint32_t &CdDelaySize() { return MMIO<uint32_t>(0x1018); }
		inline constexpr volatile uint32_t &Exp2DelaySize() { return MMIO<uint32_t>(0x101C); }

		inline constexpr volatile uint32_t &ComDelayCfg() { return MMIO<uint32_t>(0x1020); }
		inline constexpr volatile uint32_t &RamSizeCfg() { return MMIO<uint32_t>(0x1060); }

		// DUART
		inline constexpr volatile uint8_t &DuartMode() { return MMIO<uint8_t>(0x2020); }
		inline constexpr volatile uint8_t &DuartSra() { return MMIO<uint8_t>(0x2021); }
		inline constexpr volatile uint8_t &DuartCra() { return MMIO<uint8_t>(0x2022); }
		inline constexpr volatile uint8_t &DuartHra() { return MMIO<uint8_t>(0x2023); }

		// Clocks
		/// @brief CPU clock rate
		static constexpr uint32_t CpuHz = 33868800UL;
		/// @brief GPU clock rate
		static constexpr uint32_t GpuHz = 53222400UL;

		// OS types
		/// @brief Thread context
		/// @note For internal use only
		struct Thread
		{
			uint32_t zero;
			uint32_t at;
			uint32_t v0, v1;
			uint32_t a0, a1, a2, a3;
			uint32_t t0, t1, t2, t3, t4, t5, t6, t7;
			uint32_t s0, s1, s2, s3, s4, s5, s6, s7;
			uint32_t t8, t9;
			uint32_t k0, epc;
			uint32_t gp, sp, fp;
			uint32_t ra;
			uint32_t sr, cause;
		};
		static_assert(sizeof(Thread) == (4 * 34));

		/// @brief Function pointer wrapper for shared library functions
		/// @tparam R Return type
		/// @tparam A Argument types
		template<typename R, typename... A>
		class Function
		{
			/// @cond INTERNAL
			private:
				typedef R (*F)(A...);
				F ptr;

			public:
				Function() { ptr = nullptr; }
				Function(F _ptr) { ptr = _ptr; }
				Function(Function &o) { ptr = o.ptr; }

				inline R operator()(A... arg)
				{
					// T9 must be set before calling the function
					// as it's used for local addressing for DLLs
					// It has no effect for boot functions
					INLINE_ASM("move $t9, %0;" :: "r"(ptr) : "$t9");
					return ptr(arg...);
				}
				
				bool operator==(F _ptr) { return ptr == _ptr; }
				bool operator==(Function &o) { return ptr == o.ptr; }
				bool operator!=(F _ptr) { return ptr != _ptr; }
				bool operator!=(Function &o) { return ptr != o.ptr; }
			/// @endcond INTERNAL
		};

		/// @brief Interrupt callback type
		typedef Function<void> InterruptCallback;

		// OS functions
		/// @brief Initialize the OS
		/// @note For internal use only
		void Init();

		/// @brief Set callback for an interrupt cause
		/// @param i Interrupt cause
		/// @param cb InterruptCallback
		/// @returns Previous InterruptCallback
		/// @note This function requires IRQs to be disabled
		/// @see DisableIRQ
		InterruptCallback SetIRQ(IRQ i, InterruptCallback cb);
		/// @brief Get callback for an interrupt cause
		/// @param i Interrupt cause
		/// @return InterruptCallback
		InterruptCallback GetIRQ(IRQ i);

		/// @brief Set callback for a DMA channel
		/// @param i DMA channel
		/// @param cb InterruptCallback
		/// @returns Previous InterruptCallback
		/// @note This function requires IRQs to be disabled
		/// @see DisableIRQ
		InterruptCallback SetDMA(DMA i, InterruptCallback cb);
		/// @brief Get callback for a DMA channel
		/// @param i DMA channel
		/// @return InterruptCallback
		InterruptCallback GetDMA(DMA i);

		/// @brief Disable IRQs directly using the interrupt status register
		/// @note This function is for internal use only
		/// @see DisableIRQ
		inline void DisableIRQ_SR()
		{
			uint32_t r0, r1;
			INLINE_ASM(
				"mfc0 %0, $12;"
				"li   %1, ~0x0401;"
				"and  %1, %0;"
				"mtc0 %1, $12;"
				"nop;"
				: "=r"(r0), "=r"(r1) ::
			);
		}
		/// @brief Enable IRQs directly using the interrupt status register
		/// @note This function is for internal use only
		/// @see EnableIRQ
		inline void EnableIRQ_SR()
		{
			uint32_t r0;
			INLINE_ASM(
				"mfc0 %0, $12;"
				"nop;"
				"ori  %0, 0x0401;"
				"mtc0 %0, $12;"
				"nop;"
				: "=r"(r0) : "r"(r0) :
			);
		}

		/// @brief Disable IRQs directly using the interrupt status register (to be used within an exception handler)
		/// @note This function is for internal use only
		/// @see DisableIRQ
		inline void DisableIRQ_SR_RFE()
		{
			uint32_t r0, r1;
			INLINE_ASM(
				"mfc0 %0, $12;"
				"li   %1, ~0x0404;"
				"and  %1, %0;"
				"mtc0 %1, $12;"
				"nop;"
				: "=r"(r0), "=r"(r1) ::
			);
		}
		/// @brief Disable IRQs directly using the interrupt status register (to be used within an exception handler)
		/// @note This function is for internal use only
		/// @see EnableIRQ
		inline void EnableIRQ_SR_RFE()
		{
			uint32_t r0, r1;
			INLINE_ASM(
				"mfc0 %0, $12;"
				"li   %1, ~0x0404;"
				"and  %1, %0;"
				"mtc0 %1, $12;"
				"nop;"
				: "=r"(r0), "=r"(r1) ::
			);
		}

		/// @brief Disable IRQs using a syscall
		inline void DisableIRQ() { ::CKSDK_OS_DisableIRQ_asm(); }
		/// @brief Enable IRQs using a syscall
		inline void EnableIRQ() { ::CKSDK_OS_EnableIRQ_asm(); }

		/// @brief Flush the instruction cache
		/// @note If you are modifying code in memory, you must call this function to ensure the CPU executes the new code
		void FlushICache();

		/// @brief Wait for a number of cycles
		/// @param cycles Number of cycles to wait
		/// @note Cycles waited is approximate, but will be at least the number of cycles specified
		inline void WaitCycles(uint32_t cycles)
		{
			// Cycle count is approximate, 1 cycle for setting i, 3 cycles to iterate through the loop
			for (uint32_t i = ((cycles - 1) / 3); i != 0; i--)
				INLINE_ASM("");
		}
	}
}
