/*!
	\file CBasePostProcessingProfile.h
	\date 12.02.2020
	\author Kasimov Ildar
*/

#pragma once


#include "IPostProcessingProfile.h"
#include "./../core/CBaseResource.h"


namespace TDEngine2
{
	class IResourceHandler;


	/*!
		\brief A factory function for creation objects of CBasePostProcessingProfile's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePostProcessingProfile's implementation
	*/

	TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CBasePostProcessingProfile's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params A parameters of created material

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePostProcessingProfile's implementation
	*/

	TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																	 const TPostProcessingProfileParameters& params, E_RESULT_CODE& result);


	/*!
		class CBasePostProcessingProfile

		\brief The class represents a default implementation of a post-processing profile which contains
		parameters for different camera effects
	*/

	class CBasePostProcessingProfile : public CBaseResource, public IPostProcessingProfile
	{
		public:
			friend 	TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
																					 E_RESULT_CODE& result);

			friend TDE2_API IPostProcessingProfile* CreateBasePostProcessingProfile(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
																					const TPostProcessingProfileParameters& params, E_RESULT_CODE& result);

		public:
			TDE2_REGISTER_RESOURCE_TYPE(CBasePostProcessingProfile)
			TDE2_REGISTER_TYPE(CBasePostProcessingProfile)

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

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method initializes an internal state of a profile. The overloaded version of
				Init method creates a new profile based on given parameters

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] params Additional parameters of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TPostProcessingProfileParameters& params) override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			TDE2_API const TPostProcessingProfileParameters::TBloomParameters& GetBloomParameters() const override;
			TDE2_API const TPostProcessingProfileParameters::TColorGradingParameters& GetColorGradingParameters() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePostProcessingProfile)

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			IGraphicsContext* mpGraphicsContext;
			
			TPostProcessingProfileParameters::TBloomParameters mBloomParameters;

			TPostProcessingProfileParameters::TColorGradingParameters mColorGradingParameters;
	};


	/*!
		\brief A factory function for creation objects of CBasePostProcessingProfileLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePostProcessingProfileLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBasePostProcessingProfileLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																	E_RESULT_CODE& result);


	/*!
		class CBasePostProcessingProfileLoader

		\brief The class implements a functionality of a loader of post-processing profile resources
	*/

	class CBasePostProcessingProfileLoader : public CBaseObject, public IPostProcessingProfileLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBasePostProcessingProfileLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																				   E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API TypeId GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePostProcessingProfileLoader)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CBasePostProcessingProfileFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBasePostProcessingProfileFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateBasePostProcessingProfileFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CBasePostProcessingProfileFactory

		\brief The class is an abstract factory of CBasePostProcessingProfile objects that
		is used by a resource manager
	*/

	class CBasePostProcessingProfileFactory : public CBaseObject, public IPostProcessingProfileFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateBasePostProcessingProfileFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBasePostProcessingProfileFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
