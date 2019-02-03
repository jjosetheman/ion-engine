///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		Semaphore.h
// Date:		8th January 2014
// Authors:		Matt Phillips
// Description:	Threading and synchronisation
///////////////////////////////////////////////////

#pragma once

#include "core/Platform.h"
#include "core/Types.h"

namespace ion
{
	namespace thread
	{
		class Semaphore
		{
		public:
			Semaphore(int maxSignalCount);
			~Semaphore();

			void Signal();
			void Wait();

		private:
			#if defined ION_PLATFORM_WINDOWS
			HANDLE m_semaphoreHndl;
            #elif defined ION_PLATFORM_LINUX
            sem_t m_semaphoreHndl;
            #elif defined ION_PLATFORM_MACOSX
            dispatch_semaphore_t m_semaphoreHndl;
			#endif
		};
	}
}
