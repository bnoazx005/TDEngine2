/*!
	\file CPrefabsManifest.h
	\date 14.05.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IPrefabsManifest.h"
#include "../core/CBaseResource.h"
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CPrefabsManifest's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CFont's implementation
	*/

	TDE2_API IPrefabsManifest* CreatePrefabsManifest(IResourceManager* pResourceManager, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CPrefabsManifest

		\brief The class implements basic collection of prefabs identifiers with related filesystem paths
	*/

	class CPrefabsManifest: public CBaseResource, public virtual IPrefabsManifest
	{
		public:
			friend 	TDE2_API IPrefabsManifest* CreatePrefabsManifest(IResourceManager*, const std::string&, E_RESULT_CODE&);
		public:
			typedef std::unordered_map<std::string, std::string> TPrefabsToPathHashTable;
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CPrefabsManifest)
			TDE2_REGISTER_TYPE(CPrefabsManifest)

			/*!
				\brief The method initializes an internal state of a font

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

			TDE2_API const std::string& GetPathToPrefabById(const std::string& prefabId) const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabsManifest)

			TDE2_API const TPtr<IResourceLoader> _getResourceLoader() override;
		private:
			static constexpr U16    mVersionTag = 0x1;

			TPrefabsToPathHashTable mPrefabsMappingsTable;
	};


	/*!
		\brief A factory function for creation objects of CPrefabsManifestLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPrefabsManifestLoader's implementation
	*/

	TDE2_API IResourceLoader* CreatePrefabsManifestLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);


	/*!
		class CPrefabsManifestLoader

		\brief The class implements a functionality of a prefabs manifest loader
	*/

	class CPrefabsManifestLoader : public CBaseObject, public IPrefabsManifestLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreatePrefabsManifestLoader(IResourceManager* pResourceManager, IFileSystem* pFileSystem, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabsManifestLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;
	};


	/*!
		\brief A factory function for creation objects of CPrefabsManifestFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CPrefabsManifestFactory's implementation
	*/

	TDE2_API IResourceFactory* CreatePrefabsManifestFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);


	/*!
		class CPrefabsManifestFactory

		\brief The class is an abstract factory of CPrefabsManifest objects that
		is used by a resource manager
	*/

	class CPrefabsManifestFactory : public CBaseObject, public IPrefabsManifestFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreatePrefabsManifestFactory(IResourceManager* pResourceManager, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager) override;

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CPrefabsManifestFactory)
		protected:
			IResourceManager* mpResourceManager;
	};
}