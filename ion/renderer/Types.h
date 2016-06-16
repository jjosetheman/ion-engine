///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		types.h
// Date:		13th June 2016
// Authors:		Matt Phillips
// Description:	Renderer types
///////////////////////////////////////////////////

#pragma once

namespace ion
{
	namespace render
	{
#if defined WIN32
		typedef HWND WindowHandle;
		typedef HDC DeviceContext;
#else
		#error Unsupported platform
#endif
	}
}