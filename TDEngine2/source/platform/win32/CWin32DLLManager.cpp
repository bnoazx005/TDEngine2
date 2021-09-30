#include "./../../../include/platform/win32/CWin32DLLManager.h"
#include "./../../../include/utils/CFileLogger.h"


#if defined (TDE2_USE_WIN32PLATFORM)

namespace TDEngine2
{
	CWin32DLLManager::CWin32DLLManager() :
		CBaseObject(), mNextFreeHandler(TDynamicLibraryHandler(0))
	{
	}

	E_RESULT_CODE CWin32DLLManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		LOG_MESSAGE("[Win32 DLL Manager] The Win32 DLL manager was successfully initialized");

		return RC_OK;
	}
	
	E_RESULT_CODE CWin32DLLManager::_onFreeInternal()
	{
		E_RESULT_CODE result = RC_OK;

		if ((result = UnloadAll()) != RC_OK)
		{
			return result;
		}

		LOG_MESSAGE("[Win32 DLL Manager] The Win32 DLL manager was successfully destroyed");

		return RC_OK;
	}

	TDynamicLibraryHandler CWin32DLLManager::Load(const std::string& filename, E_RESULT_CODE& result)
	{
		if (filename.empty())
		{
			result = RC_INVALID_ARGS;

			return TDynamicLibraryHandler::Invalid;
		}
		
		TDynLibHandlersMap::const_iterator duplicateIter = mHandlersTable.find(filename);

		if (duplicateIter != mHandlersTable.cend())
		{
			return (*duplicateIter).second;	// just return a handler to the already loaded library
		}

		std::string filenameWithExt(filename + ".dll");

		TDynamicLibrary dynamicLibrary = ::LoadLibrary(filenameWithExt.c_str());

		if (!dynamicLibrary)
		{
			result = RC_FILE_NOT_FOUND;

			return TDynamicLibraryHandler::Invalid;
		}

		TDynamicLibraryHandler loadedLibraryHandler = TDynamicLibraryHandler(mLoadedLibraries.size());

		if (mNextFreeHandler >= loadedLibraryHandler) // there is no free slot, so extend the existing array
		{
			mLoadedLibraries.push_back(dynamicLibrary);
		}
		else
		{
			mLoadedLibraries[static_cast<U32>(mNextFreeHandler)] = dynamicLibrary;

			loadedLibraryHandler = mNextFreeHandler;

			// seek for next free space within the array
			if (mFreeHandlersList.empty())
			{
				mNextFreeHandler = loadedLibraryHandler;
			}
			else
			{
				mNextFreeHandler = mFreeHandlersList.front();

				mFreeHandlersList.pop_front();
			}
		}

		LOG_MESSAGE("[Win32 DLL Manager] A new dll file was successfully loaded (" + filenameWithExt + ")");

		return loadedLibraryHandler;
	}

	E_RESULT_CODE CWin32DLLManager::Unload(const std::string& filename)
	{
		if (filename.empty())
		{
			return RC_FAIL;
		}

		TDynLibHandlersMap::const_iterator targetLibraryIter = mHandlersTable.find(filename);

		if (targetLibraryIter != mHandlersTable.cend()) // library isn't loaded
		{
			return RC_FAIL;
		}

		TDynamicLibraryHandler libraryHandler = (*targetLibraryIter).second;
		
		if (!FreeLibrary(mLoadedLibraries[static_cast<U32>(libraryHandler)]))
		{
			return RC_FAIL;
		}

		mLoadedLibraries[static_cast<U32>(libraryHandler)] = nullptr;

		mHandlersTable.erase(targetLibraryIter);

		// add current handler into the list of free handlers
		mFreeHandlersList.push_back(libraryHandler);

		LOG_MESSAGE("[Win32 DLL Manager] The existing dll file was unloaded (" + filename + ")");

		return RC_OK;
	}

	E_RESULT_CODE CWin32DLLManager::Unload(TDynamicLibraryHandler& libraryHandler)
	{
		U32 libHandlerValue = static_cast<U32>(libraryHandler);

		if (libraryHandler == TDynamicLibraryHandler::Invalid ||
			libHandlerValue >= mLoadedLibraries.size() ||
			!mLoadedLibraries[libHandlerValue])
		{
			return RC_FAIL;
		}
		
		TDynamicLibrary loadedLibrary = mLoadedLibraries[libHandlerValue];

		TDynLibHandlersMap::const_iterator libraryHandlerIter = mHandlersTable.cend();
		
		for (TDynLibHandlersMap::const_iterator iter = mHandlersTable.cbegin(); iter != mHandlersTable.cend(); ++iter)
		{
			if ((*iter).second == libraryHandler)
			{
				libraryHandlerIter = iter;

				break;
			}
		}
		
		if (libraryHandlerIter == mHandlersTable.cend())
		{
			return RC_FAIL;
		}
		
		if (!FreeLibrary(loadedLibrary))
		{
			return RC_FAIL;
		}

		mHandlersTable.erase(libraryHandlerIter);

		mLoadedLibraries[libHandlerValue] = nullptr;

		// add current handler into the list of free handlers
		mFreeHandlersList.push_back(libraryHandler);
		
		return RC_OK;
	}

	E_RESULT_CODE CWin32DLLManager::UnloadAll()
	{
		E_RESULT_CODE result = RC_OK;

		std::string currFilename;

		for (TDynLibHandlersMap::iterator iter = mHandlersTable.begin(); iter != mHandlersTable.end();)
		{
			currFilename = (*iter).first;

			iter++;

			if ((result = Unload(currFilename)) != RC_OK)
			{
				return result;
			}
		}

		return RC_OK;
	}
	
	void* CWin32DLLManager::GetSymbol(const TDynamicLibraryHandler& libraryHandler, const std::string& symbol)
	{
		U32 libHandlerValue = static_cast<U32>(libraryHandler);

		if (libraryHandler == TDynamicLibraryHandler::Invalid ||
			libHandlerValue >= mLoadedLibraries.size())
		{
			return nullptr;
		}

		return GetProcAddress(mLoadedLibraries[libHandlerValue], symbol.c_str());
	}


	TDE2_API IDLLManager* CreateWin32DLLManager(E_RESULT_CODE& result)
	{
		CWin32DLLManager* pDLLManagerInstance = new (std::nothrow) CWin32DLLManager();

		if (!pDLLManagerInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pDLLManagerInstance->Init();

		if (result != RC_OK)
		{
			delete pDLLManagerInstance;

			pDLLManagerInstance = nullptr;
		}

		return dynamic_cast<IDLLManager*>(pDLLManagerInstance);
	}
}

#endif