/*!
	\file CLocalizationPackage.h
	\date 20.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "ILocalizationPackage.h"
#include "../CBaseResource.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CLocalizationPackage's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLocalizationPackage's implementation
	*/

	TDE2_API ILocalizationPackage* CreateLocalizationPackage(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);


	class CLocalizationPackage : public ILocalizationPackage, public CBaseResource
	{
		public:
			friend TDE2_API ILocalizationPackage* CreateLocalizationPackage(IResourceManager*, const std::string&, E_RESULT_CODE&);

		public:
			TDE2_REGISTER_RESOURCE_TYPE(CLocalizationPackage)
			TDE2_REGISTER_TYPE(CLocalizationPackage)

			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) override;

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

			TDE2_API const TDictionary& GetDictionaryData() const override;
			
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLocalizationPackage)

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			std::unordered_map<std::string, std::string> mDictionary;
	};


	/*!
		\brief A factory function for creation objects of CLocalizationPackageLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLocalizationPackageLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateLocalizationPackageLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem,	E_RESULT_CODE& result);


	/*!
		class CLocalizationPackageLoader

		\brief The class implements a functionality of a loader of localization packages
	*/

	class CLocalizationPackageLoader : public CBaseObject, public ILocalizationPackageLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateLocalizationPackageLoader(IResourceManager*, IFileSystem*, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLocalizationPackageLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CLocalizationPackageFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CLocalizationPackageFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateLocalizationPackageFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CLocalizationPackageFactory

		\brief The class is an abstract factory of CLocalizationPackage objects that is used by a resource manager
	*/

	class CLocalizationPackageFactory : public CBaseObject, public ILocalizationPackageFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateLocalizationPackageFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CLocalizationPackageFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}