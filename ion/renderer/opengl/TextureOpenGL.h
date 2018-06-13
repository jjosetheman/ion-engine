///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		TextureOpenGL.h
// Date:		13th December 2013
// Authors:		Matt Phillips
// Description:	OpenGL texture implementation
///////////////////////////////////////////////////

#pragma once

#include "core/Platform.h"
#include "renderer/Texture.h"
#include "OpenGLInclude.h"

#include <string>

namespace ion
{
	namespace render
	{
		class TextureOpenGL : public Texture
		{
		public:
			TextureOpenGL();
			TextureOpenGL(u32 width, u32 height);
			TextureOpenGL(u32 width, u32 height, Format sourceFormat, Format destFormat, BitsPerPixel bitsPerPixel, bool generateMipmaps, bool generatePixelBuffer, const u8* data);
			virtual ~TextureOpenGL();

			virtual bool Load(u32 width, u32 height, Format sourceFormat, Format destFormat, BitsPerPixel bitsPerPixel, bool generateMipmaps, bool generatePixelBuffer, const u8* data);
			GLuint GetTextureId() const;

			virtual void SetMinifyFilter(Filter filter);
			virtual void SetMagnifyFilter(Filter filter);
			virtual void SetWrapping(Wrapping wrapping);

			virtual void SetPixel(const ion::Vector2i& position, const Colour& colour);
			virtual void SetPixels(Format sourceFormat, u8* data);
			virtual void GetPixels(const ion::Vector2i& position, const ion::Vector2i& size, Format format, BitsPerPixel bitsPerPixel, u8* data) const;

		protected:
			virtual bool Load();
			virtual void Unload();
			static void GetOpenGLMode(Format format, BitsPerPixel bitsPerPixel, int& mode, int& byteFormat, int& colourFormat, int& pixelSize);

			GLuint m_glTextureId;
			GLuint m_glPixelBufferId;
			int m_glFormat;
			BitsPerPixel m_bitsPerPixel;
		};
	}
}
