/*!
	\file CSkinnedMesh.h
	\date 05.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseMesh.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CSkinnedMesh's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkinnedMesh's implementation
	*/

	TDE2_API ISkinnedMesh* CreateSkinnedMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		class CSkinnedMesh

		\brief The class describes a functionality of an animable skinned mesh
	*/

	class CSkinnedMesh: public ISkinnedMesh, public CBaseMesh
	{
		public:
			friend TDE2_API ISkinnedMesh* CreateSkinnedMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CSkinnedMesh)
			TDE2_REGISTER_TYPE(CSkinnedMesh)

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			TDE2_API E_RESULT_CODE Accept(IBinaryMeshFileReader* pReader) override;

			TDE2_API void AddVertexJointWeights(const TJointsWeightsArray& weights) override;
			TDE2_API void AddVertexJointIndices(const TJointsIndicesArray& indices) override;

			TDE2_API const std::vector<TJointsWeightsArray>& GetJointWeightsArray() const override;
			TDE2_API const std::vector<TJointsIndicesArray>& GetJointIndicesArray() const override;

			TDE2_API bool HasJointWeights() const override;
			TDE2_API bool HasJointIndices() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMesh)

			TDE2_API const IResourceLoader* _getResourceLoader() override;

			TDE2_API bool _hasJointWeightsInternal() const;
			TDE2_API bool _hasJointIndicesInternal() const;

			TDE2_API std::vector<U8> _toArrayOfStructsDataLayoutInternal() const override;
		protected:
			std::vector<TJointsWeightsArray> mJointsWeights;
			std::vector<TJointsIndicesArray> mJointsIndices;
	};



	/*!
		\brief A factory function for creation objects of CSkinnedMeshLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkinnedMeshLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateSkinnedMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													 E_RESULT_CODE& result);


	/*!
		class CSkinnedMeshLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CSkinnedMeshLoader : public CBaseObject, public IMeshLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateSkinnedMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMeshLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CSkinnedMeshFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSkinnedMeshFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateSkinnedMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CSkinnedMeshFactory

		\brief The class is an abstract factory of CSkinnedMesh objects that
		is used by a resource manager
	*/

	class CSkinnedMeshFactory : public CBaseObject, public IMeshFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateSkinnedMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSkinnedMeshFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}