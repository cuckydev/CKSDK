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

/// @file CKSDK/GPU.h
/// @brief CKSDK GPU API

#pragma once

#include <CKSDK/CKSDK.h>

#include <CKSDK/OS.h>

#include <CKSDK/GTE.h>

/// @brief CKSDK namespace
namespace CKSDK
{
	/// @brief CKSDK GPU namespace
	namespace GPU
	{
		// GPU globals
		/// @brief Set to `true` if the GPU is PAL
		extern bool g_pal;

		// GPU types
		/// @brief GPU word
		typedef uint32_t Word;

		/// @brief Tag for each packet for GP0 DMA which points to the next packet
		struct Tag
		{
			/// @brief Constructor
			Tag() {}

			/// @brief Constructor
			/// @param ptr Pointer to the next packet
			/// @param words Number of words in the packet excluding the tag itself
			Tag(uintptr_t ptr, size_t words) : tag((ptr & 0x00FFFFFF) | (words << 24)) {}

			/// @brief Constructor
			/// @param ptr Pointer to the next packet
			/// @param words Number of words in the packet excluding the tag itself
			/// @overload
			Tag(void *ptr, size_t words) : tag((uintptr_t(ptr) & 0x00FFFFFF) | (words << 24)) {}
			
			/// @brief Word
			Word tag;

			/// @brief Get the pointer to the next packet
			/// @return Pointer to the next packet
			void *Ptr() { return (void*)(tag & 0x00FFFFFF); }

			/// @brief Get the number of words in the packet excluding the tag itself
			/// @return Number of words in the packet excluding the tag itself
			size_t Words() { return (tag >> 24); }
		};

		/// @brief Color is an RGB color with an extra byte
		union Color
		{
			/// @brief Struct
			struct
			{
				/// @brief Red component
				uint8_t r;
				/// @brief Green component
				uint8_t g;
				/// @brief Blue component
				uint8_t b;
				/// @brief Unused
				uint8_t x;
			} s;
			/// @brief Word
			Word w = 0;

			/// @brief Constructor
			Color() {}

			/// @brief Constructor
			/// @param r Red component
			/// @param g Green component
			/// @param b Blue component
			Color(uint8_t r, uint8_t g, uint8_t b) : w(((Word)r << 0) | ((Word)g << 8) | ((Word)b << 16)) {}

			/// @brief Assignment operator
			/// @param c Color
			/// @return Reference to this
			Color &operator=(const Color &c) { w = (c.w & 0xFFFFFF) | ((Word)this->s.x << 24); return *this; }
		};

		/// @brief 2D screen coordinates
		union ScreenCoord
		{
			/// @brief Struct
			struct
			{
				/// @brief X coordinate
				int16_t x;
				/// @brief Y coordinate
				int16_t y;
			} s;
			/// @brief Word
			Word w = 0;

			/// @brief Constructor
			ScreenCoord() {}

			/// @brief Constructor
			/// @param x X coordinate
			/// @param y Y coordinate
			ScreenCoord(int16_t x, int16_t y) : w(((Word)x << 0) | ((Word)y << 16)) {}
		};

		/// @brief 2D sceren dimensions
		union ScreenDim
		{
			/// @brief Struct
			struct
			{
				/// @brief Width
				uint16_t w;
				/// @brief Height
				uint16_t h;
			} s;
			/// @brief Word
			Word w = 0;

			/// @brief Constructor
			ScreenDim() {}

			/// @brief Constructor
			/// @param w Width
			/// @param h Height
			ScreenDim(uint16_t w, uint16_t h) : w(((Word)w << 0) | ((Word)h << 16)) {}
		};

		/// @brief 2D texture coordinate with an extra half word
		union TexCoord
		{
			/// @brief Struct
			struct
			{
				/// @brief U coordinate
				uint8_t u;
				/// @brief V coordinate
				uint8_t v;
				/// @brief Unused
				uint16_t x;
			} s;
			/// @brief Word
			Word w = 0;

			/// @brief Constructor
			TexCoord() {}

			/// @brief Constructor
			/// @param u U coordinate
			/// @param v V coordinate
			TexCoord(uint8_t u, uint8_t v) : w(((Word)u << 0) | ((Word)v << 8)) {}
		};

		/// @brief Semi transparency modes
		enum SemiMode
		{
			/// @brief Background 50% + Foreground 50%
			SemiMode_Blend,
			/// @brief Background 100% + Foreground 100%
			SemiMode_Add,
			/// @brief Background 100% - Foreground 100%
			SemiMode_Sub,
			/// @brief Background 25% + Foreground 75%
			SemiMode_AddQuarter,
		};

		/// @brief Bit depth modes
		enum BitDepth
		{
			/// @brief 4-bit
			BitDepth_4Bit,
			/// @brief 8-bit
			BitDepth_8Bit,
			/// @brief 15-bit
			BitDepth_15Bit,
			/// @brief 24-bit (not supported)
			BitDepth_24Bit,
		};

		/// @brief Texture page
		struct TexPage
		{
			/// @brief Word
			uint16_t tpage = 0;

			/// @brief Constructor
			TexPage() {}

			/// @brief Constructor
			/// @param x X coordinate
			/// @param y Y coordinate
			/// @param semi Semi transparency mode
			/// @param bpp Texture bit depth
			TexPage(uint16_t x, uint16_t y, uint16_t semi, uint16_t bpp) : tpage((x << 0) | (y << 4) | (semi << 5) | (bpp << 7)) {}
		};

		/// @brief Clut page
		struct Clut
		{
			/// @brief Word
			uint16_t clut = 0;

			/// @brief Constructor
			Clut() {}

			/// @brief Constructor
			/// @param x X coordinate
			/// @param y Y coordinate
			Clut(uint16_t x, uint16_t y) : clut((x << 0) | (y << 6)) {}
		};

		// GP0 commands
		/// @brief GP0 command types
		enum GP0_CmdTypes
		{
			/// @brief Misc commands
			GP0_Misc   = (0 << 5),
			/// @brief Polygon commands
			GP0_Poly   = (1 << 5),
			/// @brief Line commands
			GP0_Line   = (2 << 5),
			/// @brief Rectangle commands
			GP0_Rect   = (3 << 5),
			/// @brief From VRAM commands
			GP0_FrVRAM = (4 << 5),
			/// @brief To VRAM commands
			GP0_ToVRAM = (5 << 5),
			/// @brief To GPU commands
			GP0_ToCPU  = (6 << 5),
			/// @brief Environment commands
			GP0_Env    = (7 << 5),
		};

		// GP0_Misc
		enum GP0_MiscCmds
		{
			/// @brief No operation
			GP0_Nop = GP0_Misc | 0,
			/// @brief Flush the GPU cache
			GP0_FlushCache = GP0_Misc | 1,
			/// @brief Fill a rectangle directly to VRAM
			GP0_FillRect = GP0_Misc | 2,
		};

		// GP0_Poly
		enum GP0_PolyCmds
		{
			/// @brief Each vertex has a color
			GP0_Poly_Grad = (1 << 4),
			/// @brief Polygon is a quad
			GP0_Poly_Quad = (1 << 3),
			/// @brief Polygon is textured
			GP0_Poly_Tex  = (1 << 2),
			/// @brief Semi-transparency is enabled
			GP0_Poly_Semi = (1 << 1),
			/// @brief Modulation is disabled
			GP0_Poly_Raw  = (1 << 0),
		};

		/// @cond INTERNAL
		/// Vertex contains vertex data depending on if it's textured or has colors
		template<bool Grad, bool Tex>
		struct PolyVertex;

		template<>
		struct PolyVertex<false, false>
		{ ScreenCoord xy; };

		template<>
		struct PolyVertex<false, true>
		{ ScreenCoord xy; TexCoord uv; };

		template<>
		struct PolyVertex<true, false>
		{ Color c; ScreenCoord xy; };

		template<>
		struct PolyVertex<true, true>
		{ Color c; ScreenCoord xy; TexCoord uv; };

		/// PolySet contains the vertex data for a polygon depending on if it's a quad or triangle
		template<bool Quad, typename V0, typename V>
		struct PolySet;

		template<typename V0, typename V>
		struct PolySet<false, V0, V>
		{ V0 v0; V v1, v2; };

		template<typename V0, typename V>
		struct PolySet<true, V0, V>
		{ V0 v0; V v1, v2, v3; };
		/// @endcond

		/// @brief Polygon primitive template structure
		/// @tparam Grad `true` if each vertex has a color
		/// @tparam Quad `true` if the polygon is a quad
		/// @tparam Tex `true` if textured
		/// 
		/// @details The structure will contain v0 through v2, or v0 through v3 if `Quad` is `true`.
		/// @details Each vertex will contain \link ScreenCoord xy\endlink, as well as \link TexCoord uv\endlink (if `Tex` is `true`), and \link Color c\endlink (if `Grad` is `true`).
		/// @details The color of the polygon, if `Grad` is `false`, is set by `v0.c`.
		template<bool Grad, bool Quad, bool Tex>
		struct PolyPrim :
		/// @cond INTERNAL
			public PolySet<Quad, PolyVertex<true, Tex>, PolyVertex<Grad, Tex>>
		/// @endcond
		{
			PolyPrim()
			{
				this->v0.c.w = (GP0_Poly |
					(Grad ? GP0_Poly_Grad : 0) |
					(Quad ? GP0_Poly_Quad : 0) |
					(Tex ? GP0_Poly_Tex : 0)) << 24;
			}

			/// @brief Sets semi-transparency flag
			/// @param semi `true` to enable semi-transparency
			void SetSemi(bool semi)
			{
				if (semi)
					this->v0.c.w |= (GP0_Poly_Semi << 24);
				else
					this->v0.c.w &= ~(GP0_Poly_Semi << 24);
			}

			/// @brief Sets raw flag
			/// @param semi `true` to disable modulation
			void SetRaw(bool raw)
			{
				if (raw)
					this->v0.c.w |= (GP0_Poly_Raw << 24);
				else
					this->v0.c.w &= ~(GP0_Poly_Raw << 24);
			}
			
			/// @brief Returns the clut
			/// @return Clut
			Clut &clut() { return *((Clut*)(&this->v1.uv.s.x)); }
			/// @brief Returns the clut
			/// @return Clut
			/// @overload
			const Clut &clut() const { return clut(); }

			/// @brief Returns the texture page
			/// @return TexPage
			TexPage &tpage() { return *((TexPage*)(&this->v2.uv.s.x)); }
			/// @brief Returns the texture page
			/// @return TexPage
			/// @overload
			const TexPage &tpage() const { return tpage(); }
		};

		// GP0_Rect
		enum GP0_RectCmds
		{
			/// @brief Rectangle is 1x1
			GP0_Rect_1x1   = (1 << 3),
			/// @brief Rectangle is 8x8
			GP0_Rect_8x8   = (2 << 3),
			/// @brief Rectangle is 16x16
			GP0_Rect_16x16 = (3 << 3),
			/// @brief Rectangle is textured
			GP0_Rect_Tex   = (1 << 2),
			/// @brief Semi-transparency is enabled
			GP0_Rect_Semi  = (1 << 1),
			/// @brief Modulation is disabled
			GP0_Rect_Raw   = (1 << 0),
		};

		enum class GP0_RectSize
		{
			Variable = 0,
			Fixed1x1 = 1,
			Fixed8x8 = 2,
			Fixed16x16 = 3,
		};

		/// @cond INTERNAL
		/// RectData contains the header data for a rectangle
		template <bool Tex, bool Dim>
		struct RectData;

		template <>
		struct RectData<false, false>
		{ Color c; ScreenCoord xy; };

		template <>
		struct RectData<true, false>
		{ Color c; ScreenCoord xy; TexCoord uv; };

		template <>
		struct RectData<false, true>
		{ Color c; ScreenCoord xy; ScreenDim wh; };

		template <>
		struct RectData<true, true>
		{ Color c; ScreenCoord xy; TexCoord uv; ScreenDim wh; };
		/// @endcond

		/// @brief Rectangle primitive template structure
		/// @tparam Tex `true` if textured
		/// @tparam Size Size of the rectangle (Variable by default, set size using `wh`)
		template <bool Tex, GP0_RectSize Size = GP0_RectSize::Variable>
		struct RectPrim :
		/// @cond INTERNAL
			public RectData<Tex, Size == GP0_RectSize::Variable>
		/// @endcond
		{
			RectPrim()
			{
				this->c.w = (GP0_Rect |
					((uint32_t)Size << 3) |
					(Tex ? GP0_Rect_Tex : 0)) << 24;
			}

			/// @brief Sets semi-transparency flag
			/// @param semi `true` to enable semi-transparency
			void SetSemi(bool semi)
			{
				if (semi)
					this->c.w |= (GP0_Rect_Semi << 24);
				else
					this->c.w &= ~(GP0_Rect_Semi << 24);
			}

			/// @brief Sets raw flag
			/// @param semi `true` to disable modulation
			void SetRaw(bool raw)
			{
				if (raw)
					this->c.w |= (GP0_Rect_Raw << 24);
				else
					this->c.w &= ~(GP0_Rect_Raw << 24);
			}
		};

		// Common rect types
		/// @brief Fill primitive type
		/// @tparam Size Size of the rectangle
		template <GP0_RectSize Size = GP0_RectSize::Variable>
		using FillPrim = RectPrim<false, Size>;

		/// @brief Sprite primitive type
		/// @tparam Size Size of the rectangle
		template <GP0_RectSize Size = GP0_RectSize::Variable>
		using SpritePrim = RectPrim<true, Size>;

		// GP0_Env
		enum GP0_EnvCmds
		{
			/// @brief Set draw mode
			GP0_DrawMode = GP0_Env | 1,
			/// @brief Set framebuffer top left coordinates in VRAM
			GP0_DrawTL = GP0_Env | 3,
			/// @brief Set framebuffer bottom right coordinates in VRAM
			GP0_DrawBR = GP0_Env | 4,
			/// @brief Set draw offset coordinates in VRAM
			GP0_DrawOffset = GP0_Env | 5,
		};

		/// @brief DrawMode is a struct which contains the draw mode register
		/// 
		/// @param tpage Texture page index
		/// @param semi Semi Transparency (see SemiMode)
		/// @param bpp Texture bit depth (see BitDepth)
		/// @param dither Dither 24bit to 15bit
		/// @param draw_enable Drawing to display area
		/// @param tex_disable Texture Disable
		struct DrawModePrim
		{
			Word mode;

			DrawModePrim() {}
			DrawModePrim(Word tpage, Word semi, Word bpp, Word dither, Word draw_enable, Word tex_disable)
				: mode((GP0_DrawMode << 24) | (tpage & 0x1F) | ((semi & 3) << 5) | ((bpp & 3) << 7) | ((dither & 1) << 9) | ((draw_enable & 1) << 10) | ((tex_disable & 1) << 11))
			{}

			/// @brief Returns the texture page X base
			/// @return Texture page X base
			Word X() const { return mode & 0xF; }
			/// @brief Returns the texture page Y base
			/// @return Texture page Y base
			Word Y() const { return (mode >> 4) & 1; }

			/// @brief Returns the semi transparency mode
			/// @return Semi transparency mode
			Word Semi() const { return (mode >> 5) & 3; }
			/// @brief Returns the texture bit depth
			/// @return Texture bit depth
			Word Bpp() const { return (mode >> 7) & 3; }
			/// @brief Returns dither enabled
			/// @return Dither enabled
			Word Dither() const { return (mode >> 9) & 1; }

			/// @brief Returns draw enabled
			/// @return Draw enabled
			Word DrawEnable() const { return (mode >> 10) & 1; }
			/// @brief Returns texture disabled
			/// @return Texture disabled
			Word TexDisable() const { return (mode >> 11) & 1; }

			/// @brief Returns X-Flip enabled
			/// @return X-Flip enabled
			Word XFlip() const { return (mode >> 12) & 1; }
			/// @brief Returns Y-Flip enabled
			/// @return Y-Flip enabled
			Word YFlip() const { return (mode >> 13) & 1; }
		};
		
		// GP1 commands
		enum GP1_Cmds
		{
			/// @brief Reset the GPU
			GP1_Reset = 0x00,
			/// @brief Flush the GPU command buffer
			GP1_Flush = 0x01,
			/// @brief Mask or unmask the screen
			GP1_DisplayEnable = 0x03,
			/// @brief Set the DMA direction
			GP1_DMADirection = 0x04,
			/// @brief Set the display coordinates
			GP1_DisplayVRAM = 0x05,
			/// @brief Set the display horizontal span
			GP1_DisplayHSpan = 0x06,
			/// @brief Set the display vertical span
			GP1_DisplayVSpan = 0x07,
			/// @brief Set the display mode
			GP1_DisplayMode = 0x08,
		};

		// GPU types
		/// @brief 32-bit 3D vector
		struct Vector
		{
			int32_t x, y, z;

			/// @brief Constructor
			Vector() {}

			/// @brief Constructor
			/// @param x X component
			/// @param y Y component
			/// @param z Z component
			Vector(int32_t x, int32_t y, int32_t z) : x(x), y(y), z(z) {}
		};
		/// @brief 16-bit 3D vector
		struct SVector
		{
			int16_t x, y, z, pad;
		};
		
		/// @brief 16-bit rect
		struct Rect
		{
			int16_t x, y, w, h;
		};
		
		/// @brief 3x4 Matrix
		struct Matrix
		{
			int16_t m[3][3];
			Vector t;

			static Matrix Identity()
			{
				return {{{0x1000, 0, 0}, {0, 0x1000, 0}, {0, 0, 0x1000}}, {0, 0, 0}};
			}
		};

		// GPU buffers
		/// @brief GPU buffer
		struct Buffer
		{
			// Screen
			/// @brief Screen GP0 packet
			struct GP0
			{
				/// @brief GP0_DrawTL
				Word tl;
				/// @brief GP0_DrawBR
				Word br;
				/// @brief GP0_DrawOffset
				Word off;
				/// @brief GP0_DisplayMode
				DrawModePrim mode;
			} gp0;
			/// @brief GP1_DisplayVRAM
			Word gp1_vram;
			/// @brief GP1_DisplayHSpan
			Word gp1_hspan;
			/// @brief GP1_DisplayVSpan
			Word gp1_vspan;
			/// @brief GP1_DisplayMode
			Word gp1_mode;

			// Buffers
			/// @brief Buffer pointer
			Word *buffer;
			/// @brief Ordering table size
			size_t ot_size;
			/// @brief Primitive buffer pointer
			Word *prip;

			/// @brief Gets ordering table tag at index
			/// @param i Index of tag
			/// @return Tag at index
			Tag &GetOT(size_t i)
			{ return *((Tag*)(&buffer[1 + i])); }

			/// @brief Initializes primitive buffer and ordering table
			void Init()
			{
				// Set primitive pointer
				prip = (Word*)(&GetOT(ot_size));

				// Initialize ordering tables
				OS::DmaCtrl(OS::DMA::OTC).madr = uint32_t(&GetOT(ot_size - 1));
				OS::DmaCtrl(OS::DMA::OTC).bcr  = (ot_size + 1) & 0xFFFF;
				OS::DmaCtrl(OS::DMA::OTC).chcr = 0x11000002;
				while ((OS::DmaCtrl(OS::DMA::OTC).chcr & (1 << 24)) != 0);
			}
		};

		/// @brief Current GPU buffer
		/// @note For internal use only
		extern Buffer *g_bufferp;
		
		// GPU functions
		/// @brief Initializes GPU
		/// @note For internal use only
		void Init();

		/// @brief Set GPU buffer
		/// @param buffer Buffer to set
		/// @param size Size of buffer in words
		/// @param ot_size Size of ordering table in entries
		/// @details This splits the given buffer into two for double buffering
		/// @details The ordering table is placed at the beginning of each buffer
		/// @details ot_size represents the addressable size of the ordering table, the terminator is added automatically
		void SetBuffer(Word *buffer, size_t size, size_t ot_size);

		/// @brief Sets GPU framebuffers
		/// @param w Width of framebuffers
		/// @param h Height of framebuffers
		/// @param ox X draw offset
		/// @param oy Y draw offset
		/// @param x0 X coordinate of framebuffer 0 in VRAM
		/// @param y0 Y coordinate of framebuffer 0 in VRAM
		/// @param x1 X coordinate of framebuffer 1 in VRAM
		/// @param y1 Y coordinate of framebuffer 1 in VRAM
		/// @details Valid widths are 256, 320, 368, 512, and 640
		/// @details If height exceeds 256, the GPU will be set to interlaced mode
		/// @details Changes will apply on the next call to Flip()
		void SetScreen(uint32_t w, uint32_t h, uint32_t ox, uint32_t oy, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);

		/// @brief Flips and displays GPU buffers
		void Flip();

		/// @brief Waits for next VBlank
		void VBlankSync();
		/// @brief Waits for GPU command queue to be empty
		void QueueSync();

		/// @brief Resets GPU command queue
		/// @note For internal use only
		void QueueReset();

		/// @brief DMA image to VRAM
		/// @param addr Address of image data
		/// @param xy X and Y coordinate in VRAM
		/// @param wh Width and height in VRAM
		/// @param bcr DMA block count value
		void DMAImage(const void *addr, uint32_t xy, uint32_t wh, uint32_t bcr);

		/// @brief Loads image to VRAM
		/// @param addr Address of image data
		/// @param x X coordinate in VRAM
		/// @param y Y coordinate in VRAM
		/// @param w Width in VRAM
		/// @param h Height in VRAM
		inline void LoadImage(const void *addr, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
		{
			uint32_t bcr = w * h;
			// if (bcr & 1)
			// 	ExScreen::Abort("LoadImage w * h is odd");
			bcr >>= 1;

			uint32_t bs = 1;
			while (((bcr & 1) == 0) && (bs < 16))
			{
				bs <<= 1;
				bcr >>= 1;
			}

			// if (bcr >= 65536)
			// 	ExScreen::Abort("LoadImage bcr too large");
			bcr <<= 16;
			bcr |= bs;
			DMAImage(addr, (x << 0) | (y << 16), (w << 0) | (h << 16), bcr);
		}

		/// @brief Flip callback type
		typedef OS::Function<void> FlipCallback;

		/// @brief Sets flip callback
		/// @param cb FlipCallback
		/// @returns Previous FlipCallback
		FlipCallback SetFlipCallback(FlipCallback cb);

		/// @brief Gets flip callback
		/// @returns FlipCallback
		FlipCallback GetFlipCallback();

		/// @brief VBlank callback type
		typedef OS::Function<void> VBlankCallback;

		/// @brief Sets VBlank callback
		/// @param cb VBlankCallback
		/// @returns Previous VBlankCallback
		VBlankCallback SetVBlankCallback(VBlankCallback cb);

		/// @brief Gets VBlank callback
		/// @returns VBlankCallback
		VBlankCallback GetVBlankCallback();

		/// @brief Queue callback type
		typedef OS::Function<void> QueueCallback;

		/// @brief Sets queue callback
		/// @param cb QueueCallback
		/// @returns Previous QueueCallback
		QueueCallback SetQueueCallback(QueueCallback cb);

		/// @brief Gets queue callback
		/// @returns QueueCallback
		QueueCallback GetQueueCallback();

		// GPU packet functions
		/// @brief Allocates and links a packet of a given size
		/// @param ot Ordering table index
		/// @param words Packet size
		/// @return Packet
		/// @details Allocates and links a packet of a given size onto the given ordering table
		/// @details Packets are linked in reverse order, but the primitives within the packet will run in the order they are written
		inline Word *AllocPacket(size_t ot, size_t words)
		{
			Tag *otp = (Tag*)&g_bufferp->GetOT(ot);
			Word *prip = g_bufferp->prip;

			new(prip) Tag(otp->Ptr(), words);
			new(otp) Tag(prip, 0);

			g_bufferp->prip = prip + words + 1;
			return prip + 1;
		}

		/// @brief Allocates and links a packet of a given type
		/// @tparam T Packet type
		/// @param ot Ordering table index
		/// @return Packet
		/// @details Allocates, constructs, and links a packet of a given size onto the given ordering table
		/// @details Packets are linked in reverse order, but the primitives within the packet will run in the order they are written
		/// @note The packet type must not be larger than 16 words, as this will overflow the GPU's FIFO
		template <typename T>
		inline T &AllocPacket(size_t ot)
		{
			static_assert((sizeof(T) / sizeof(Word)) <= 16, "Packet type too big");

			T &packet = *((T*)AllocPacket(ot, sizeof(T) / sizeof(Word)));
			new (&packet) T();
			return packet;
		}
		
		/// @brief Wait until GPU is ready to receive command word
		inline void CmdSync() { while ((OS::GpuGp1() & (1 << 26)) == 0); }
		/// @brief Wait until GPU is ready to receive DMA block
		inline void DataSync() { while ((OS::GpuGp1() & (1 << 28)) == 0); }
		/// @brief Wait until GPU DMA is finished
		inline void CHCRSync() { while ((OS::DmaCtrl(OS::DMA::GPU).chcr & (1 << 24)) != 0); }
		
		/// @brief Sends a word to the GP0 port after waiting for command ready
		/// @param cmd Word
		inline void GP0_Cmd(Word cmd)
		{
			CmdSync();
			OS::GpuGp0() = cmd;
		}
		
		/// @brief Sends a word to the GP0 port after waiting for data ready
		/// @param cmd Word
		inline void GP0_Data(Word cmd)
		{
			DataSync();
			OS::GpuGp0() = cmd;
		}

		/// @brief Sends a packet to the GP0 port
		/// @tparam T Packet type
		/// @param packet Packet
		template <typename T>
		inline void GP0_Packet(const T &packet)
		{
			Word *wordp = (Word*)&packet;
			Word *worde = wordp + (sizeof(T) / sizeof(Word));

			CmdSync();
			for (; wordp != worde; wordp++)
			{
				DataSync();
				OS::GpuGp0() = *wordp;
			}
			CmdSync();
		}

		/// @brief Sends a word to the GP1 port
		/// @param cmd Word
		inline void GP1_Cmd(Word cmd)
		{
			OS::GpuGp1() = cmd;
		}
	}
}
