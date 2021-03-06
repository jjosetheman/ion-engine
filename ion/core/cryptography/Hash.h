///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		Hash.h
// Date:		15th December 2011
// Authors:		Matt Phillips
// Description:	Various hashing algorithms
///////////////////////////////////////////////////

#pragma once

#include "core/Types.h"

namespace ion
{
	u32 Hash(const char* string);
	u32 Hash(const u8* data, int size);
	u64 Hash64(const u8* data, int size);
}