/*!
	\file IAudioSource.h
	\date 06.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IResourceFactory.h"
#include "../core/IResourceLoader.h"


namespace TDEngine2
{
	class IAudioContext;


	/*!
		interface IAudioSource

		\brief The interface describes a functionality of an audio source. It could be represented as a single sound or a bank of sounds, etc
	*/

	class IAudioSource
	{
		public:
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAudioSource)
	};


	/*!
		interface IAudioSourceLoader

		\brief The interface describes a functionality of an audio source's loader
	*/

	class IAudioSourceLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation
				\param[in, out] pAudioContext A pointer to IAudioContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IAudioContext* pAudioContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAudioSourceLoader)
	};


	/*!
		interface IAudioSourceFactory

		\brief The interface describes a functionality of an audio source's factory
	*/

	class IAudioSourceFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pAudioContext A pointer to IAudioContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAudioSourceFactory)
	};
}