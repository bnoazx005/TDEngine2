#include "./../../../include/platform/unix/CUnixDLLManager.h"


#if defined (TDE2_USE_UNIXPLATFORM)

#include <dlfcn.h>


namespace TDEngine2
{
	CUnixDLLManager::CUnixDLLManager() :
		CBaseObject(), mNextFreeHandler(0)
	{
	}

	E_RESULT_CODE CUnixDLLManager::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CUnixDLLManager::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		if ((result = UnloadAll()) != RC_OK)
		{
			return result;
		}

		delete this;

		mIsInitialized = false;

		return RC_OK;
	}

	TDynamicLibraryHandler CUnixDLLManager::Load(const std::string& filename, E_RESULT_CODE& result)
	{
		if (filename.empty())
		{
			result = RC_INVALID_ARGS;

			return InvalidDynamicLibHandlerValue;
		}

		TDynLibHandlersMap::const_iterator duplicateIter = mHandlersTable.find(filename);

		if (duplicateIter != mHandlersTable.cend())
		{
			return (*duplicateIter).second;	// just return a handler to the already loaded library
		}

		std::string filenameWithExt(filename + ".so");

		TDynamicLibrary dynamicLibrary = dlopen(filenameWithExt.c_str(), RTLD_LAZY | RTLD_GLOBAL);

		if (!dynamicLibrary)
		{
			result = RC_FILE_NOT_FOUND;

			return InvalidDynamicLibHandlerValue;
		}

		TDynamicLibraryHandler loadedLibraryHandler = mLoadedLibraries.size();

		if (mNextFreeHandler >= loadedLibraryHandler) // there is no free slot, so extend the existing array
		{
			mLoadedLibraries.push_back(dynamicLibrary);
		}
		else
		{
			mLoadedLibraries[mNextFreeHandler] = dynamicLibrary;

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

		return loadedLibraryHandler;
	}

	E_RESULT_CODE CUnixDLLManager::Unload(const std::string& filename)
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

		if (dlclose(mLoadedLibraries[libraryHandler]) != 0)
		{
			return RC_FAIL;
		}

		mLoadedLibraries[libraryHandler] = nullptr;

		mHandlersTable.erase(targetLibraryIter);

		// add current handler into the list of free handlers
		mFreeHandlersList.push_back(libraryHandler);

		return RC_OK;
	}

	E_RESULT_CODE CUnixDLLManager::Unload(TDynamicLibraryHandler& libraryHandler)
	{
		if (libraryHandler == InvalidDynamicLibHandlerValue ||
			libraryHandler >= mLoadedLibraries.size() ||
			!mLoadedLibraries[libraryHandler])
		{
			return RC_FAIL;
		}

		TDynamicLibrary loadedLibrary = mLoadedLibraries[libraryHandler];

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

		if (dlclose(loadedLibrary) != 0)
		{
			return RC_FAIL;
		}

		mHandlersTable.erase(libraryHandlerIter);

		mLoadedLibraries[libraryHandler] = nullptr;

		// add current handler into the list of free handlers
		mFreeHandlersList.push_back(libraryHandler);

		return RC_OK;
	}

	E_RESULT_CODE CUnixDLLManager::UnloadAll()
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

	void* CUnixDLLManager::GetSymbol(const TDynamicLibraryHandler& libraryHandler, const std::string& symbol)
	{
		if (libraryHandler == InvalidDynamicLibHandlerValue ||
			libraryHandler >= mLoadedLibraries.size())
		{
			return nullptr;
		}

		return dlsym(mLoadedLibraries[libraryHandler], symbol.c_str());
	}


	TDE2_API IDLLManager* CreateUnixDLLManager(E_RESULT_CODE& result)
	{
		CUnixDLLManager* pDLLManagerInstance = new (std::nothrow) CUnixDLLManager();

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