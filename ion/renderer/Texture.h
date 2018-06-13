///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		Texture.h
// Date:		19th December 2011
// Authors:		Matt Phillips
// Description:	Texture map
///////////////////////////////////////////////////

#pragma once

#include "core/Types.h"
#include "maths/Vector.h"
#include "io/Archive.h"
#include "renderer/Colour.h"

#include <string>

namespace ion
{
	namespace render
	{
		class Texture
		{
		public:
			enum Format
			{
				eRGB,
				eBGR,
				eRGBA,
				eBGRA,
				eRGBA_DXT5
			};

			enum BitsPerPixel
			{
				eBPP8 = 8,
				eBPP16 = 16,
				eBPP24 = 24,
				eBPP32 = 32
			};

			enum Filter
			{
				eFilterNearest,
				eFilterLinear,
				eFilterMipMapLinear
			};

			enum Wrapping
			{
				eWrapClamp,
				eWrapRepeat,
				eWrapMirror
			};

			static Texture* Create();
			static Texture* Create(u32 width, u32 height);
			static Texture* Create(u32 width, u32 height, Format sourceFormat, Format destFormat, BitsPerPixel bitsPerPixel, bool generateMipmaps, bool generatePixelBuffer, const u8* data);

			virtual ~Texture();

			u32 GetWidth() const;
			u32 GetHeight() const;

			virtual bool Load(u32 width, u32 height, Format sourceFormat, Format destFormat, BitsPerPixel bitsPerPixel, bool generateMipmaps, bool generatePixelBuffer, const u8* data) { return false; }
			void SetImageFilename(const std::string& filename);

			virtual void SetMinifyFilter(Filter filter) = 0;
			virtual void SetMagnifyFilter(Filter filter) = 0;
			virtual void SetWrapping(Wrapping wrapping) = 0;

			virtual void SetPixel(const ion::Vector2i& position, const Colour& colour) = 0;
			virtual void SetPixels(Format sourceFormat, u8* data) = 0;
			virtual void GetPixels(const ion::Vector2i& position, const ion::Vector2i& size, Format format, BitsPerPixel bitsPerPixel, u8* data) const = 0;

			//Serialise
			static void RegisterSerialiseType(io::Archive& archive);
			void Serialise(io::Archive& archive);

		protected:
			Texture();
			Texture(u32 width, u32 height);
			Texture(u32 width, u32 height, Format sourceFormat, Format destFormat, BitsPerPixel bitsPerPixel, bool generateMipmaps, bool generatePixelBuffer, const u8* data);

			virtual bool Load() { return false; }
			virtual void Unload() { }

			u32 m_width;
			u32 m_height;

			std::string m_imageFilename;
		};
	}
}