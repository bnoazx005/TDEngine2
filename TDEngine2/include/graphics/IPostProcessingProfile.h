/*!
	\file IPostProcessingProfile.h
	\date 11.02.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"


namespace TDEngine2
{
	/*!
		struct TPostProcessingProfileParameters

		\brief The stucture contains fields for creation post-processing profiles objects
	*/

	typedef struct TPostProcessingProfileParameters : TBaseResourceParameters
	{
		TDE2_API TPostProcessingProfileParameters() = default;
	} TPostProcessingProfileParameters, *TPostProcessingProfileParametersPtr;


	/*!
		interface IPostProcessingProfile

		\brief The interface represents a functionality of a post-processing profile
	*/

	class IPostProcessingProfile
	{
		public:
			/*!
				\brief The method initializes an internal state of a post-processing profile. The method
				is used when we want just to load profile's data from some storage. In this
				case all the parameters will be executed automatically. To create a new blank
				object use overloaded version of Init()

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method initializes an internal state of a profile. The overloaded version of
				Init method creates a new profile based on given parameters

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TPostProcessingProfileParameters& params) = 0;

			/*!
				\brief The method writes the information about post-processing profile's content down onto disk

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Serialize(IFileSystem* pFileSystem, const std::string& filename) = 0;

			/*!
				\brief The method restores state of the post-processing profile based on information from a given file

				\param[in, out] pFileSystem A pointer to IFileSystem implementation
				\param[in] filename A name of a file into which the data will be written

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Deserialize(IFileSystem* pFileSystem, const std::string& filename) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPostProcessingProfile)
	};


	/*!
		interface IPostProcessingProfileLoader

		\brief The interface describes a functionality of a post-processing profile loader
	*/

	class IPostProcessingProfileLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPostProcessingProfileLoader)
	};


	/*!
		interface IPostProcessingProfileFactory

		\brief The interface describes a functionality of a post-processing profiles factory
	*/

	class IPostProcessingProfileFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a post-processing profiles factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPostProcessingProfileFactory)
	};
}
