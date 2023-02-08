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
		// OS registers
		#define MMIO(T, x) (*((volatile T*)(0xBF800000 | (x))))
		#define MMIO8(x)  MMIO(uint8_t,  x)
		#define MMIO16(x) MMIO(uint16_t, x)
		#define MMIO32(x) MMIO(uint32_t, x)

		// GPU
		#define GPU_GP0 MMIO32(0x1810)
		#define GPU_GP1 MMIO32(0x1814)

		// CD drive
		#define CD_STAT MMIO8(0x1800);
		#define CD_CMD  MMIO8(0x1801);
		#define CD_DATA MMIO8(0x1802);
		#define CD_IRQ  MMIO8(0x1803);
		
		#define CD_REG(i) MMIO8(0x1800 + (i))

		// SPU
		#define SPU_MASTER_VOL_L MMIO16(0x1D80)
		#define SPU_MASTER_VOL_R MMIO16(0x1D82)
		#define SPU_REVERB_VOL_L MMIO16(0x1D84)
		#define SPU_REVERB_VOL_R MMIO16(0x1D86)

		#define SPU_KEY_ON1 MMIO16(0x1D88)
		#define SPU_KEY_ON2 MMIO16(0x1D8A)

		#define SPU_KEY_OFF1 MMIO16(0x1D8C)
		#define SPU_KEY_OFF2 MMIO16(0x1D8E)

		#define SPU_FM_MODE1     MMIO16(0x1D90)
		#define SPU_FM_MODE2     MMIO16(0x1D92)
		#define SPU_NOISE_MODE1  MMIO16(0x1D94)
		#define SPU_NOISE_MODE2  MMIO16(0x1D96)
		#define SPU_REVERB_ON1   MMIO16(0x1D98)
		#define SPU_REVERB_ON2   MMIO16(0x1D9A)
		#define SPU_CHAN_STATUS1 MMIO16(0x1D9C)
		#define SPU_CHAN_STATUS2 MMIO16(0x1D9E)

		#define SPU_REVERB_ADDR MMIO16(0x1DA2)
		#define SPU_IRQ_ADDR    MMIO16(0x1DA4)
		#define SPU_ADDR        MMIO16(0x1DA6)
		#define SPU_DATA        MMIO16(0x1DA8)

		#define SPU_CTRL     MMIO16(0x1DAA)
		#define SPU_DMA_CTRL MMIO16(0x1DAC)
		#define SPU_STAT     MMIO16(0x1DAE)

		#define SPU_CD_VOL_L      MMIO16(0x1DB0)
		#define SPU_CD_VOL_R      MMIO16(0x1DB2)
		#define SPU_EXT_VOL_L     MMIO16(0x1DB4)
		#define SPU_EXT_VOL_R     MMIO16(0x1DB6)
		#define SPU_CURRENT_VOL_L MMIO16(0x1DB8)
		#define SPU_CURRENT_VOL_R MMIO16(0x1DBA)

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
		#define SPU_VOICE_CTRL(i) MMIO(::CKSDK::OS::SPU_VOICE_CTRL_t, 0x1C00 + (unsigned)(i) * 16)

		// MDEC
		#define MDEC0 = MMIO32(0x1820)
		#define MDEC1 = MMIO32(0x1824)

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
		#define SIO_CTRL(i) MMIO(::CKSDK::OS::SIO_CTRL_t, 0x1040 + unsigned(i) * 16)

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

		#define IRQ_STAT MMIO16(0x1070)
		#define IRQ_MASK MMIO16(0x1074)

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

		static constexpr uint32_t DPCR_Set(uint32_t dpcr, DMA dma, uint32_t priority)
		{
			unsigned i = unsigned(dma);
			return (dpcr & ~(15 << (i << 2))) |
				(priority << (i << 2)) |
				(8 << (i << 2));
		}

		#define DMA_DPCR MMIO32(0x10F0)
		#define DMA_DICR MMIO32(0x10F4)

		struct DMA_CTRL_t
		{
			uint32_t madr;
			uint32_t bcr;
			uint32_t chcr;
			uint32_t pad;
		};
		static_assert(sizeof(DMA_CTRL_t) == 16);
		#define DMA_CTRL(i) MMIO(::CKSDK::OS::DMA_CTRL_t, 0x1080 + unsigned(i) * 16)
		
		// Timers
		struct TIMER_CTRL_t
		{
			uint32_t value;
			uint32_t ctrl;
			uint32_t reload;
			uint32_t pad;
		};
		static_assert(sizeof(TIMER_CTRL_t) == 16);
		#define TIMER_CTRL(i) MMIO(::CKSDK::OS::TIMER_CTRL_t, 0x1100 + unsigned(i) * 16)

		// Memory control
		#define EXP1_ADDR       MMIO32(0x1000)
		#define EXP2_ADDR       MMIO32(0x1004)

		#define EXP1_DELAY_SIZE MMIO32(0x1008)
		#define EXP3_DELAY_SIZE MMIO32(0x100C)
		#define BIOS_DELAY_SIZE MMIO32(0x1010)
		#define SPU_DELAY_SIZE  MMIO32(0x1014)
		#define CD_DELAY_SIZE   MMIO32(0x1018)
		#define EXP2_DELAY_SIZE MMIO32(0x101C)

		#define COM_DELAY_CFG   MMIO32(0x1020)
		#define RAM_SIZE_CFG    MMIO32(0x1060)

		// DUART
		/// @brief DUART mode
		#define DUART_MODE MMIO8(0x2020)
		#define DUART_SRA  MMIO8(0x2021)
		#define DUART_CRA  MMIO8(0x2022)
		#define DUART_HRA  MMIO8(0x2023)

		// Clocks
		/// @brief CPU clock rate
		static constexpr uint32_t F_CPU = 33868800UL;
		/// @brief GPU clock rate
		static constexpr uint32_t F_GPU = 53222400UL;

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
