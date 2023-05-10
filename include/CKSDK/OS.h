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
		// @brief Memory map I/O port
		template <typename T>
		inline constexpr volatile T &MMIO(uintptr_t addr)
		{
			return *(volatile T*)(0xBF800000 | addr);
		}

		// GPU
		/// @brief GPU GP0 command port
		inline constexpr volatile uint32_t &GpuGp0() { return MMIO<uint32_t>(0x1810); }
		/// @brief GPU GP1 command port
		inline constexpr volatile uint32_t &GpuGp1() { return MMIO<uint32_t>(0x1814); }

		// CD drive
		/// @brief CD drive status port
		inline constexpr volatile uint8_t &CdStat() { return MMIO<uint8_t>(0x1800); }
		/// @brief CD drive command port
		inline constexpr volatile uint8_t &CdCmd() { return MMIO<uint8_t>(0x1801); }
		/// @brief CD drive data port
		inline constexpr volatile uint8_t &CdData() { return MMIO<uint8_t>(0x1802); }
		/// @brief CD drive interrupt port
		inline constexpr volatile uint8_t &CdIrq() { return MMIO<uint8_t>(0x1803); }

		// SPU
		/// @brief SPU master volume left port
		inline constexpr volatile uint16_t &SpuMasterVolL() { return MMIO<uint16_t>(0x1D80); }
		/// @brief SPU master volume right port
		inline constexpr volatile uint16_t &SpuMasterVolR() { return MMIO<uint16_t>(0x1D82); }
		/// @brief SPU reverb volume left port
		inline constexpr volatile uint16_t &SpuReverbVolL() { return MMIO<uint16_t>(0x1D84); }
		/// @brief SPU reverb volume right port
		inline constexpr volatile uint16_t &SpuReverbVolR() { return MMIO<uint16_t>(0x1D86); }

		/// @brief SPU key on port
		inline constexpr volatile uint32_t &SpuKeyOn() { return MMIO<uint32_t>(0x1D88); }
		/// @brief SPU key off port
		inline constexpr volatile uint32_t &SpuKeyOff() { return MMIO<uint32_t>(0x1D8C); }

		/// @brief SPU FM mode port
		/// @note When FM is enabled for a channel, the frequency is modulated by the previous channel's amplitude
		inline constexpr volatile uint32_t &SpuFmMode() { return MMIO<uint32_t>(0x1D90); }
		/// @brief SPU noise mode port
		inline constexpr volatile uint32_t &SpuNoiseMode() { return MMIO<uint32_t>(0x1D94); }
		/// @brief SPU reverb on port
		inline constexpr volatile uint32_t &SpuReverbOn() { return MMIO<uint32_t>(0x1D98); }
		/// @brief SPU channel status port
		/// @note This doesn't actually reflect if the channel is playing or not
		inline constexpr volatile uint32_t &SpuChanStatus() { return MMIO<uint32_t>(0x1D9C); }

		/// @brief SPU voice frequency port
		inline constexpr volatile uint16_t &SpuReverbAddr() { return MMIO<uint16_t>(0x1DA2); }
		/// @brief SPU IRQ address port
		inline constexpr volatile uint16_t &SpuIrqAddr() { return MMIO<uint16_t>(0x1DA4); }
		/// @brief SPU address port
		inline constexpr volatile uint16_t &SpuAddr() { return MMIO<uint16_t>(0x1DA6); }
		/// @brief SPU data port
		inline constexpr volatile uint16_t &SpuData() { return MMIO<uint16_t>(0x1DA8); }

		/// @brief SPU control port
		inline constexpr volatile uint16_t &SpuCtrl() { return MMIO<uint16_t>(0x1DAA); }
		/// @brief SPU DMA control port
		inline constexpr volatile uint16_t &SpuDmaCtrl() { return MMIO<uint16_t>(0x1DAC); }
		/// @brief SPU status port
		inline constexpr volatile uint16_t &SpuStat() { return MMIO<uint16_t>(0x1DAE); }

		/// @brief SPU CD volume left port
		inline constexpr volatile uint16_t &SpuCdVolL() { return MMIO<uint16_t>(0x1DB0); }
		/// @brief SPU CD volume right port
		inline constexpr volatile uint16_t &SpuCdVolR() { return MMIO<uint16_t>(0x1DB2); }
		/// @brief SPU external volume left port (expansion port)
		inline constexpr volatile uint16_t &SpuExtVolL() { return MMIO<uint16_t>(0x1DB4); }
		/// @brief SPU external volume right port (expansion port)
		inline constexpr volatile uint16_t &SpuExtVolR() { return MMIO<uint16_t>(0x1DB6); }
		/// @brief SPU current volume left port
		/// @note Not sure what this is. Maybe the last mixed digital sample?
		inline constexpr volatile uint16_t &SpuCurrentVolL() { return MMIO<uint16_t>(0x1DB8); }
		/// @brief SPU current volume right port
		/// @note Not sure what this is. Maybe the last mixed digital sample?
		inline constexpr volatile uint16_t &SpuCurrentVolR() { return MMIO<uint16_t>(0x1DBA); }

		/// @brief SPU voice control structure
		struct SPU_VOICE_CTRL
		{
			/// @brief Voice left volume
			uint16_t vol_l;
			/// @brief Voice right volume
			uint16_t vol_r;
			/// @brief Voice frequency (0x1000 = 44100Hz)
			uint16_t freq;
			/// @brief Voice start address
			uint16_t addr;
			/// @brief Voice ADSR
			uint32_t adsr;
			/// @brief Voice loop address
			uint16_t loop_addr;
			uint16_t pad;
		};
		static_assert(sizeof(SPU_VOICE_CTRL) == 16);
		
		/// @brief SPU voice control ports (0-23)
		inline constexpr volatile SPU_VOICE_CTRL &SpuVoiceCtrl(int i) { return (&MMIO<SPU_VOICE_CTRL>(0x1D80))[i]; }

		// MDEC
		/// @brief MDEC0 control port
		inline constexpr volatile uint32_t &Mdec0() { return MMIO<uint32_t>(0x1820); }
		/// @brief MDEC1 control port
		inline constexpr volatile uint32_t &Mdec1() { return MMIO<uint32_t>(0x1824); }

		// SIOs
		/// @brief SIO control structure
		struct SIO_CTRL
		{
			/// @brief SIO data FIFO
			/// @note You can technically read ahead, but behavior is undefined and it's not recommended
			uint8_t fifo[4];
			/// @brief SIO status
			uint16_t stat;
			uint16_t pad3;
			/// @brief SIO mode
			uint16_t mode;
			/// @brief SIO control
			uint16_t ctrl;
			uint16_t reserved;
			/// @brief SIO baud rate
			uint16_t baud;
		};
		static_assert(sizeof(SIO_CTRL) == 16);

		/// @brief SIO control ports (0-1)
		inline constexpr volatile SIO_CTRL &SioCtrl(int i) { return (&MMIO<SIO_CTRL>(0x1040))[i]; }

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

		/// @brief IRQ status port
		inline constexpr volatile uint16_t &IrqStat() { return MMIO<uint16_t>(0x1070); }
		/// @brief IRQ mask port
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

		/// @brief DMA DPCR helper
		/// @param dpcr DPCR value
		/// @param dma DMA channel
		/// @param priority DMA priority
		/// @return New DPCR value
		static constexpr uint32_t DpcrSet(uint32_t dpcr, DMA dma, uint32_t priority)
		{
			unsigned i = unsigned(dma);
			return (dpcr & ~(15 << (i << 2))) |
				(priority << (i << 2)) |
				(8 << (i << 2));
		}

		/// @brief DMA DPCR port
		inline constexpr volatile uint32_t &DmaDpcr() { return MMIO<uint32_t>(0x10F0); }
		/// @brief DMA DICR port
		inline constexpr volatile uint32_t &DmaDicr() { return MMIO<uint32_t>(0x10F4); }

		/// @brief DMA control structure
		struct DMA_CTRL
		{
			/// @brief DMA MADR
			uint32_t madr;
			/// @brief DMA BCR
			uint32_t bcr;
			/// @brief DMA CHCR
			uint32_t chcr;
			uint32_t pad;
		};
		static_assert(sizeof(DMA_CTRL) == 16);
		
		/// @brief DMA control ports (0-6)
		/// @see DMA
		inline constexpr volatile DMA_CTRL &DmaCtrl(DMA i) { return (&MMIO<DMA_CTRL>(0x1080))[(int)i]; }
		
		// Timers
		/// @brief Timer control structure
		struct TIMER_CTRL
		{
			/// @brief Timer value
			uint32_t value;
			/// @brief Timer control
			uint32_t ctrl;
			/// @brief Timer reload value
			uint32_t reload;
			uint32_t pad;
		};
		static_assert(sizeof(TIMER_CTRL) == 16);

		/// @brief Timer control ports (0-2)
		inline constexpr volatile TIMER_CTRL &TimerCtrl(int i) { return (&MMIO<TIMER_CTRL>(0x1100))[i]; }

		// Memory control
		/// @brief EXP1 address port
		inline constexpr volatile uint32_t &Exp1Addr() { return MMIO<uint32_t>(0x1000); }
		/// @brief EXP3 address port
		inline constexpr volatile uint32_t &Exp2Addr() { return MMIO<uint32_t>(0x1004); }

		/// @brief EXP1 delay size port
		inline constexpr volatile uint32_t &Exp1DelaySize() { return MMIO<uint32_t>(0x1008); }
		/// @brief EXP3 delay size port
		inline constexpr volatile uint32_t &Exp3DelaySize() { return MMIO<uint32_t>(0x100C); }
		/// @brief BIOS delay size port
		inline constexpr volatile uint32_t &BiosDelaySize() { return MMIO<uint32_t>(0x1010); }
		/// @brief SPU delay size port
		inline constexpr volatile uint32_t &SpuDelaySize() { return MMIO<uint32_t>(0x1014); }
		/// @brief CD delay size port
		inline constexpr volatile uint32_t &CdDelaySize() { return MMIO<uint32_t>(0x1018); }
		/// @brief EXP2 delay size port
		inline constexpr volatile uint32_t &Exp2DelaySize() { return MMIO<uint32_t>(0x101C); }

		/// @brief Common delay configuration port
		inline constexpr volatile uint32_t &ComDelayCfg() { return MMIO<uint32_t>(0x1020); }
		/// @brief RAM size configuration port
		inline constexpr volatile uint32_t &RamSizeCfg() { return MMIO<uint32_t>(0x1060); }

		// DUART
		/// @brief DUART mode port
		inline constexpr volatile uint8_t &DuartMode() { return MMIO<uint8_t>(0x2020); }
		/// @brief DUART status port
		inline constexpr volatile uint8_t &DuartSra() { return MMIO<uint8_t>(0x2021); }
		/// @brief DUART command port
		inline constexpr volatile uint8_t &DuartCra() { return MMIO<uint8_t>(0x2022); }
		/// @brief DUART data port
		inline constexpr volatile uint8_t &DuartHra() { return MMIO<uint8_t>(0x2023); }

		// Clocks
		/// @brief CPU clock rate
		static constexpr uint32_t CpuHz = 33868800UL;
		/// @brief GPU clock rate
		static constexpr uint32_t GpuHz = 53222400UL;

		// OS types
		/// @cond INTERNAL
		/// @brief Thread context
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
			uint32_t mfhi, mflo;
		};
		static_assert(sizeof(Thread) == (4 * 36));
		/// @endcond

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
