/*!
	\file CBaseMaterial.h
	\date 18.12.2018
	\author Kasimov Ildar
*/

#pragma once


#include "IMaterial.h"
#include "./../core/CBaseResource.h"


namespace TDEngine2
{
	class IShader;


	/*!
		\brief A factory function for creation objects of CBaseMaterial's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterial's implementation
	*/

	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   TResourceId id, E_RESULT_CODE& result);


	/*!
		class CBaseMaterial

		\brief The class represents a default implementation of a material
		which is used within the engine
	*/

	class CBaseMaterial: public CBaseResource, public IMaterial
	{
		public:
			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name

				\param[in] id An identifier of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, TResourceId id) override;

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMaterial)
		protected:
			IShader*               mpShaders[MaxNumOfShaderStages];

			TUserUniformBufferData mpUserUniformsData[MaxNumberOfUserConstantBuffers];


	};


	/*!
		\brief A factory function for creation objects of CBaseMaterialLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterialLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													   E_RESULT_CODE& result);


	/*!
		class CBaseMaterialLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CBaseMaterialLoader : public IMaterialLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMaterialLoader)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};
}
