///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		FixedArray.h
// Date:		16th June 2016
// Authors:		Matt Phillips
// Description:	Wrapper for basic C array with bounds checking
///////////////////////////////////////////////////

#pragma once

#include "core/Types.h"

namespace ion
{
	template <typename T, int SIZE> class FixedArray
	{
	public:
		T& operator [] (int index) { return m_items[index]; }
		const T& operator [] (int index) const { return m_items[index]; }

	private:
		T m_items[SIZE];
	};
}