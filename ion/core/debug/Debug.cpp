///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		Debug.cpp
// Date:		25th July 2011
// Authors:		Matt Phillips
// Description:	Debug output, log
///////////////////////////////////////////////////

#include "Debug.h"
#include "ion/core/Types.h"

#include <iostream>

#if defined ION_PLATFORM_DREAMCAST
#include <kos.h>
#include <malloc.h>
extern unsigned long end;
extern unsigned long start;
#define _end end
#define _start start
#endif

namespace ion
{
	namespace debug
	{
		void Log(const char* message)
		{
			std::cout << message << "\n";
		}

		void Flush()
		{
			std::cout << std::flush;
		}

		void Error(const char* message)
		{
			Log(message);
			Flush();
			Break();
		}

		void Assert(bool condition, const char* message)
		{
			if(!condition)
			{
				Error(message);
			}
		}

		void Break()
		{
			#if defined ION_PLATFORM_WINDOWS
			__debugbreak();
			#endif
		}

		void PrintMemoryUsage()
		{
#if defined ION_PLATFORM_WINDOWS
			printf("ion::debug::PrintMemoryUsage() - TODO\n");
#elif defined ION_PLATFORM_DREAMCAST
			malloc_stats();
			pvr_mem_stats();

			u32 systemRam = 0x8d000000 - 0x8c000000;
			u32 elfOffset = 0x8c000000;
			u32 stackSize = (int)&_end - (int)&_start + ((int)&_start - elfOffset);

			struct mallinfo mi = mallinfo();
			u32 systemRamFree = systemRam - (mi.usmblks + stackSize);

			printf("Total system RAM: %i (%ikb)\n", systemRam, systemRam / 1024);
			printf("Allocated system RAM: %i (%ikb)\n", mi.usmblks, mi.usmblks / 1024);
			printf("Allocated stack size: %i (%ikb)\n", stackSize, stackSize / 1024);
			printf("Free system RAM: %i (%ikb)\n", systemRamFree, systemRamFree / 1024);
#endif
		}

		LogStream& LogStream::operator << (const char* text)
		{
			m_stream << text;
			return *this;
		}

		LogStream& LogStream::operator << (const std::string& text)
		{
			m_stream << text;
			return *this;
		}

		LogStream& LogStream::operator << (u8 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (s8 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (u16 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (s16 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (u32 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (s32 number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (float number)
		{
			m_stream << (int)number;
			return *this;
		}

		LogStream& LogStream::operator << (LogTokenEnd token)
		{
			Log(m_stream.str().c_str());
			m_stream.clear();
			return *this;
		}
	}
}