/*!
	\file IMaterial.h
	\date 18.12.2018
	\author Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IResourceLoader.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;


	/*!
		interface IMaterial

		\brief The interface describes a functionality of a material
	*/

	class IMaterial
	{
		public:
			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterial)
	};


	/*!
		interface IMaterialLoader

		\brief The interface describes a functionality of a material loader
	*/

	class IMaterialLoader : public IResourceLoader
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
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterialLoader)
	};
}
