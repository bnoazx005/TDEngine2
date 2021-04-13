/*!
	\file ILocalizationPackage.h
	\date 20.12.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Utils.h"
#include "../../core/Serialization.h"
#include "../../core/IResourceLoader.h"
#include "../../core/IResourceFactory.h"
#include <unordered_map>
#include <string>


namespace TDEngine2
{
	class IResourceManager;


	/*!
		interface ILocalizationPackage

		\brief The interface describes functionality of a localization package's resource
	*/

	class ILocalizationPackage: public ISerializable
	{
		public:
			typedef std::unordered_map<std::string, std::string> TDictionary;
		public:
			TDE2_REGISTER_TYPE(ILocalizationPackage);

			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, const std::string& name) = 0;

			TDE2_API virtual const TDictionary& GetDictionaryData() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILocalizationPackage)
	};


	/*!
		interface ILocalizationPackageLoader

		\brief The interface describes a functionality of a material loader
	*/

	class ILocalizationPackageLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILocalizationPackageLoader)
	};


	/*!
		interface ILocalizationPackageFactory

		\brief The interface describes a functionality of a material factory
	*/

	class ILocalizationPackageFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILocalizationPackageFactory)
	};
}