///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		Archive.cpp
// Date:		22nd November 2012
// Authors:		Matt Phillips
// Description:	Serialisable archive
///////////////////////////////////////////////////

#include "io/Archive.h"
#include "io/Stream.h"
#include "core/cryptography/Hash.h"

namespace ion
{
	namespace io
	{
		Archive::Archive(Stream& stream, Direction direction, ResourceManager* resourceManager, u32 version)
			: m_stream(stream)
		{
			m_direction = direction;
			m_version = version;
			m_resourceManager = resourceManager;
		}

		void Archive::Serialise(void* data, u64 size)
		{
			if(GetDirection() == eIn)
			{
				m_stream.Read(data, size);
			}
			else
			{
				if(m_blockStack.size() > 0)
				{
					//Not root block, write to temp data
					m_blockStack.back().data.Write(data, size);
				}
				else
				{
					//Root block, write to file
					m_stream.Write(data, size);
				}
			}
		}

		Archive::Direction Archive::GetDirection() const
		{
			return m_direction;
		}

		u32 Archive::GetVersion() const
		{
			return m_version;
		}

		ResourceManager* Archive::GetResourceManager() const
		{
			return m_resourceManager;
		}

		bool Archive::PushBlock(const Tag& tag)
		{
			//Create block
			Block block;

			if(m_blockStack.size() > 0)
			{
				//Set parent
				block.parent = &m_blockStack.back();
			}

			if(m_direction == eIn)
			{
				//Record block position
				block.startPos = m_stream.GetPosition();

				//Read header at current position
				Serialise(block.header);

				if(block.header.tag != tag)
				{
					//No match, record search start pos
					u64 searchStartPos = block.startPos;

					do
					{
						//If end of parent or end of file
						if(block.header.size == 0 || (block.parent && (block.startPos + block.header.size) >= (block.parent->startPos + block.parent->header.size)))
						{
							if(block.parent)
							{
								//End of parent, seek back to first child
								m_stream.Seek(block.parent->startPos + sizeof(Block::Header), eSeekModeStart);
							}
							else
							{
								//End of file, seek back to start
								m_stream.Seek(0, eSeekModeStart);
							}
						}
						else
						{
							//Seek to start of next block
							m_stream.Seek(block.startPos + block.header.size, eSeekModeStart);
						}

						//Record next block position
						block.startPos = m_stream.GetPosition();

						//Serialise next block header
						Serialise(block.header);

					} while(block.header.tag != tag && block.startPos != searchStartPos);

					if(block.header.tag != tag)
					{
						//Block not found, return to original starting position
						m_stream.Seek(searchStartPos, eSeekModeStart);
					}
				}
			}
			else
			{
				block.startPos = m_stream.GetPosition();
				block.header.tag = tag;
			}

			if(block.header.tag == tag)
			{
				//Block found (or being written), push to stack
				m_blockStack.push_back(block);

				//Success
				return true;
			}
			else
			{
				//Could not find block
				return false;
			}
		}

		void Archive::PopBlock()
		{
			debug::Assert(m_blockStack.size() > 0, "Archive::PopBlock() - No block to pop");

			//Get top block
			Block block = m_blockStack.back();

			//Pop block
			m_blockStack.pop_back();

			if(m_direction == eIn)
			{
				//Seek to block end
				m_stream.Seek(block.startPos + block.header.size, eSeekModeStart);
			}
			else
			{
				//Set block size
				block.header.size = sizeof(Block::Header) + block.data.GetSize();

				//Write current block
				Serialise(block.header);
				Serialise(block.data);
			}
		}

		void Archive::Block::Header::Serialise(Archive& archive)
		{
			archive.Serialise(tag.m_hash);
			archive.Serialise(size);
		}

		Archive::Tag::Tag()
		{
			m_hash = 0;
		}

		Archive::Tag::Tag(const char* string)
		{
			m_hash = Hash(string);
		}

		Archive::Tag::Tag(u32 hash)
		{
			m_hash = hash;
		}

		Archive::Tag::Tag(const Tag& rhs)
		{
			m_hash = rhs.m_hash;
		}

		Archive::Tag& Archive::Tag::operator = (u32 hash)
		{
			m_hash = hash;
			return *this;
		}

		bool Archive::Tag::operator == (const Tag& rhs) const
		{
			return m_hash == rhs.m_hash;
		}

		bool Archive::Tag::operator != (const Tag& rhs) const
		{
			return m_hash != rhs.m_hash;
		}
	}
}