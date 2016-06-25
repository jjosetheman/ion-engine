/*
	Symmetry Engine 2.0
	Matt Phillips (c) 2010

	File.cpp

	File IO
*/

#include "io/File.h"
#include "core/debug/Debug.h"
#include <algorithm>

namespace ion
{
	namespace io
	{
		bool FileExists(const std::string& filename)
		{
			return false;
		}

		s64 GetFileSize(const std::string& filename)
		{
			return false;
		}

		File::File()
		{
			m_openMode = eOpenRead;
			m_currentPosition = 0;
			m_size = 0;
			m_open = false;
		}

		File::File(const std::string& filename, File::OpenMode openMode)
		{
			m_openMode = openMode;
			m_currentPosition = 0;
			m_size = 0;
			m_open = false;

			Open(filename, openMode);
		}

		File::~File()
		{
			if(m_open)
				Close();
		}

		bool File::Open(const std::string& filename, File::OpenMode openMode)
		{
			std::ios::openmode mode = std::ios::binary;

			if(openMode == eOpenRead)
				mode |= std::ios::in;
			else if(openMode == eOpenWrite)
				mode |= std::ios::out;

			m_stream.open(filename.c_str(), mode);
			m_open = m_stream.is_open();
			m_openMode = openMode;
			m_filename = filename;

			if(m_open)
			{
				//Use buffer
				m_stream.rdbuf()->pubsetbuf((char*)m_buffer, s_bufferSize);

				//Get size (seek to end, get pos, seek back)
				m_stream.seekg(0, std::ios::end);
				m_size = (s64)m_stream.tellg();
				m_stream.seekg(0, std::ios::beg);
			}

			return m_open;
		}

		void File::Close()
		{
			if(m_open)
			{
				m_stream.close();
				m_open = false;
			}
		}

		s64 File::Seek(s64 position, SeekMode origin)
		{
			if(m_open)
			{
				std::ios_base::seek_dir direction = std::ios_base::cur;

				if(origin == eSeekModeStart)
				{
					direction = std::ios_base::beg;
					position = std::min(position, m_size - 1);
					m_currentPosition = position;
				}
				else if(origin == eSeekModeCurrent)
				{
					position = std::min(position, m_size - m_currentPosition - 1);
					m_currentPosition += position;
				}
			
				m_stream.seekg(position, direction);

#if !defined _RELEASE
				s64 currPos = (s64)m_stream.tellg();
				debug::Assert(m_currentPosition == currPos, "File::Seek() - Seek failed");
#endif
			}
		
			return m_currentPosition;
		}

		s64 File::Read(void* data, s64 size)
		{
			s64 bytesRead = 0;

			if(m_open)
			{
				size = std::min(size, m_size - m_currentPosition);
				m_stream.read((char*)data, size);
				m_currentPosition += size;
				bytesRead = size;

#if !defined _RELEASE
				s64 currPos = (s64)m_stream.tellg();
				debug::Assert(m_currentPosition == currPos, "File::Read() - Read failed");
#endif
			}

			return bytesRead;
		}

		s64 File::Write(const void* Data, s64 Size)
		{
			s64 bytesWritten = 0;

			if(m_open)
			{
				s64 startPosition = (s64)m_stream.tellp();
				m_stream.write((const char*)Data, Size);
				m_currentPosition = (s64)m_stream.tellp();
				bytesWritten = m_currentPosition - startPosition;
				m_size += bytesWritten;
			}

			return bytesWritten;
		}

		void File::Flush()
		{
			m_stream.flush();
		}

		s64 File::GetSize() const
		{
			return m_size;
		}

		s64 File::GetPosition() const
		{
			return m_currentPosition;
		}

		File::OpenMode File::GetOpenMode() const
		{
			return m_openMode;
		}

		bool File::IsOpen() const
		{
			return m_open;
		}
	}
} //Namespace