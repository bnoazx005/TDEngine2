/*!
	\file CFmodSoundClip.h
	\date 06.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <audio/IAudioSource.h>
#include <core/CBaseResource.h>


namespace FMOD
{
	class Sound;
}

namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CFMODAudioClip's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pAudioContext A pointer to IAudioContext's implementation
		\param[in] name A resource's name
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CD3D11Shader's implementation
	*/

	TDE2_API IAudioSource* CreateFMODAudioClip(IResourceManager* pResourceManager, IAudioContext* pAudioContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CFMODAudioClip

		\brief The class represents a single audio clip that's functionality is provided by FMOD library
	*/

	class CFMODAudioClip : public CBaseResource, public IAudioSource
	{
		public:
			friend TDE2_API IAudioSource* CreateFMODAudioClip(IResourceManager*, IAudioContext*, const std::string&, E_RESULT_CODE&);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CFMODAudioClip)
			TDE2_REGISTER_TYPE(CFMODAudioClip)

			/*!
				\brief The method initializes an internal state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pAudioContext A pointer to IAudioContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext, const std::string& name) override;

			/*!
				\brief The method should be called with a valid object. It requests a handle to FMOD sound object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ResetInternalSoundHandle(FMOD::Sound* pSoundHandle);

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method runs playback of the audio source
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Play() override;

			FMOD::Sound* GetInternalHandle() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFMODAudioClip)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		protected:
			IAudioContext* mpAudioContext;

			FMOD::Sound* mpSoundHandle;
			
	};


	/*!
		\brief A factory function for creation objects of CFMODSoundClipLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pAudioContext A pointer to IAudioContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFMODSoundClipLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateFMODAudioClipLoader(IResourceManager* pResourceManager, IAudioContext* pAudioContext, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CFMODAudioClipLoader

		\brief The class is a common implementation of a shader loaded
	*/

	class CFMODAudioClipLoader : public CBaseObject, public IAudioSourceLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateFMODAudioClipLoader(IResourceManager*, IAudioContext*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's 
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation
				\param[in, out] pAudioContext A pointer to IAudioContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem, IAudioContext* pAudioContext) override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\return The method returns an identifier of a resource's type, which the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFMODAudioClipLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IAudioContext*    mpAudioContext;
	};


	/*!
		\brief A factory function for creation objects of CFMODAudioClipFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pAudioContext A pointer to IAudioContext's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFMODAudioClipFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateFMODAudioClipFactory(IResourceManager* pResourceManager, IAudioContext* pAudioContext, E_RESULT_CODE& result);


	/*!
		class CFMODAudioClipFactory

		\brief The class is an abstract factory of CFMODAudioClip objects that is used by a resource manager
	*/

	class CFMODAudioClipFactory : public CBaseObject, public IAudioSourceFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateFMODAudioClipFactory(IResourceManager*, IAudioContext*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pAudioContext A pointer to IAudioContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IAudioContext* pAudioContext) override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFMODAudioClipFactory)
		protected:
			IResourceManager* mpResourceManager;

			IAudioContext* mpAudioContext;
	};
}