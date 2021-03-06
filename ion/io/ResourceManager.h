///////////////////////////////////////////////////
// (c) 2016 Matt Phillips, Big Evil Corporation
// http://www.bigevilcorporation.co.uk
// mattphillips@mail.com
// @big_evil_corp
//
// Licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.html
//
// File:		ResourceManager.h
// Date:		22nd December 2013
// Authors:		Matt Phillips
// Description:	Resource management
///////////////////////////////////////////////////

#pragma once

#include "core/Types.h"
#include "core/thread/Thread.h"
#include "core/thread/Semaphore.h"
#include "core/containers/Queue.h"

#include <map>
#include <string>

namespace ion
{
	namespace io
	{
		class Resource;
		template <class T> class ResourceHandle;

		class ResourceManager
		{
		public:
			ResourceManager();
			~ResourceManager();

			//Set/get resource directory
			template <class T> void SetResourceDirectory(const std::string& directory, const std::string& extension);
			template <class T> std::string GetResourceDirectory() const;
			template <class T> std::string GetResourceExtension() const;

			//Get resource handle
			template <class T> ResourceHandle<T> GetResource(const std::string& filename);

			//Manually add/remove resource
			template <class T> ResourceHandle<T> AddResource(const std::string& filename, T& resourceObject);
			void RemoveResource(const std::string& filename);

			//Get number of resources in thread queue
			u32 GetNumResourcesWaiting() const;

		protected:

			//Add load/unload requests to thread queue
			void RequestLoad(Resource& resource);
			void RequestUnload(Resource& resource);

			class WorkerThread : public thread::Thread
			{
			public:

				struct Job
				{
					enum JobType { Load, Unload, Shutdown };

					Job() {}
					Job(JobType jobType, Resource& resource)
					{
						m_jobType = jobType;
						m_resource = &resource;
					}

					JobType m_jobType;
					Resource* m_resource;
				};

				WorkerThread();
				~WorkerThread();

				void PushJob(Job& job);
				u32 GetNumJobsInQueue() const;

				void Terminate();

			protected:
				virtual void Entry();

			private:
				static const int s_jobQueueSize = 1024;
				Queue<Job, s_jobQueueSize> m_jobQueue;
				thread::Semaphore m_jobQueueSemaphore;
				u32 m_numJobsInQueue;
				bool m_threadRunning;
			};


			//Directories
			struct DirectoryEntry
			{
				std::string directory;
				std::string extension;
			};

			std::map<std::string, DirectoryEntry> m_directoryMap;

			//Resources
			std::map<std::string, Resource*> m_resourceMap;

			//Worker thread
			WorkerThread* m_workerThread;

			friend class Resource;
		};
	}
}

////////////////////////////////////////////////////////////////
// Template definitions
////////////////////////////////////////////////////////////////

#include "io/Resource.h"
#include "io/ResourceHandle.h"

namespace ion
{
	namespace io
	{
		template <class T> void ResourceManager::SetResourceDirectory(const std::string& directory, const std::string& extension)
		{
			std::string typeName = typeid(T).name();
			std::map<std::string, DirectoryEntry>::iterator it = m_directoryMap.find(typeName);

			DirectoryEntry entry;
			entry.directory = directory;
			entry.extension = extension;

			if(it == m_directoryMap.end())
			{
				std::pair<std::map<std::string, DirectoryEntry>::iterator, bool> result = m_directoryMap.insert(std::pair<std::string, DirectoryEntry>(typeName, entry));
				it = result.first;
			}

			it->second = entry;
		}

		template <class T> std::string ResourceManager::GetResourceDirectory() const
		{
			std::string directory = "";

			std::string typeName = typeid(T).name();
			std::map<std::string, DirectoryEntry>::const_iterator it = m_directoryMap.find(typeName);

			if(it != m_directoryMap.end())
			{
				directory = it->second.directory;
			}

			return directory;
		}

		template <class T> std::string ResourceManager::GetResourceExtension() const
		{
			std::string extension = "";

			std::string typeName = typeid(T).name();
			std::map<std::string, DirectoryEntry>::const_iterator it = m_directoryMap.find(typeName);

			if (it != m_directoryMap.end())
			{
				extension = it->second.extension;
			}

			return extension;
		}

		template <class T> ResourceHandle<T> ResourceManager::GetResource(const std::string& filename)
		{
			std::map<std::string, Resource*>::iterator it = m_resourceMap.find(filename);

			if(it == m_resourceMap.end())
			{
				std::pair<std::map<std::string, Resource*>::iterator, bool> result = m_resourceMap.insert(std::pair<std::string, Resource*>(filename, new ResourceT<T>(*this, filename)));
				it = result.first;
			}

			return ResourceHandle<T>((ResourceT<T>*)it->second);
		}

		template <class T> ResourceHandle<T> ResourceManager::AddResource(const std::string& filename, T& resourceObject)
		{
			std::map<std::string, Resource*>::iterator it = m_resourceMap.find(filename);

			if(it == m_resourceMap.end())
			{
				std::pair<std::map<std::string, Resource*>::iterator, bool> result = m_resourceMap.insert(std::pair<std::string, Resource*>(filename, new ResourceT<T>(*this, filename, &resourceObject)));
				it = result.first;
			}
			else
			{
				ion::debug::Error("ResourceManager::AddResource() - Resource already exists");
			}

			return ResourceHandle<T>((ResourceT<T>*)it->second);
		}
	}
}