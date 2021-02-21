/*!
	\file CStaticMesh.h
	\date 10.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IMesh.h"
#include "./../core/CBaseResource.h"
#include "./../graphics/IIndexBuffer.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;


	/*!
		\brief A factory function for creation objects of CStaticMesh's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] id An identifier of a resource
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMesh's implementation
	*/

	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CStaticMesh's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in] name A resource's name
		\param[in] params A parameters of created material
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMesh's implementation
	*/

	TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											const TMeshParameters& params, E_RESULT_CODE& result);


	/*!
		class CStaticMesh

		\brief The class describes a functionality of a static mesh
	*/

	class CStaticMesh: public IStaticMesh, public CBaseResource
	{
		public:
			friend TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);
			friend TDE2_API IStaticMesh* CreateStaticMesh(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														  const TMeshParameters& params, E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_RESOURCE_TYPE(CStaticMesh)
			TDE2_REGISTER_TYPE(CStaticMesh)

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a mesh

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										const TMeshParameters& params) override;
			
			TDE2_API E_RESULT_CODE PostLoad() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method adds a new point into the array of mesh's positions

				\param[in] pos A 3d point which represents a position of a mesh's vertex
			*/

			TDE2_API void AddPosition(const TVector4& pos) override;

			/*!
				\brief The method adds a new color for mesh's vertex

				\param[in] color A color of a vertex
			*/

			TDE2_API void AddColor(const TColor32F& color) override;

			/*!
				\brief The method adds a new point into the array of mesh's normals

				\param[in] normal A 3d vector which defines a normal
			*/

			TDE2_API void AddNormal(const TVector4& normal) override;

			/*!
				\brief The method adds a new point into the array of mesh's tangents

				\param[in] tangent A 3d vector which defines a tangent vector
			*/

			TDE2_API void AddTangent(const TVector4& tangent) override;

			/*!
				\brief The method adds a new texture coordinates for first texture channel

				\param[in] uv0 A 2d point which represents a texture coordinates
			*/

			TDE2_API void AddTexCoord0(const TVector2& uv0) override;

			/*!
				\brief The method adds an information about a face into the internal array

				\param[in] face An array with indices that form a face
			*/

			TDE2_API void AddFace(const U32 face[3]) override;

			TDE2_API const TPositionsArray& GetPositionsArray() const override;
			TDE2_API const TVertexColorArray& GetColorsArray() const override;
			TDE2_API const TNormalsArray& GetNormalsArray() const override;
			TDE2_API const TTangentsArray& GetTangentsArray() const override;
			TDE2_API const TTexcoordsArray& GetTexCoords0Array() const override;
			TDE2_API const TIndicesArray& GetIndices() const override;

			TDE2_API bool HasColors() const override;
			TDE2_API bool HasNormals() const override;
			TDE2_API bool HasTangents() const override;
			TDE2_API bool HasTexCoords0() const override;

			/*!
				\brief The method converts current data of the mesh's resource into
				array of vertex entries like the following vertex | vertex | ... | vertex

				\return An array that contains contiguous region of data
			*/

			TDE2_API std::vector<U8> ToArrayOfStructsDataLayout() const override;

			/*!
				\brief The method returns a pointer to IVertexBuffer which stores all vertex data of the mesh

				\return The method returns a pointer to IVertexBuffer which stores all vertex data of the mesh
			*/

			TDE2_API IVertexBuffer* GetSharedVertexBuffer() const override;

			/*!
				\brief The method returns a pointer to a vertex buffer that contains only positions of vertices

				\return The method returns a pointer to a vertex buffer that contains only positions of vertices
			*/

			TDE2_API IVertexBuffer* GetPositionOnlyVertexBuffer() const override;

			/*!
				\brief The method returns a pointer to IIndexBuffer which stores all index data of the mesh

				\return The method returns a pointer to IIndexBuffer which stores all index data of the mesh
			*/

			TDE2_API IIndexBuffer* GetSharedIndexBuffer() const override;

			/*!
				\brief The method returns a number of faces in the mesh

				\return The method returns a number of faces in the mesh
			*/

			TDE2_API U32 GetFacesCount() const override;

			/*!
				\brief The function creates a new static mesh resource which is a cube

				\param[in, out] pResourceManager A pointer to IResourceManager

				\return A pointer to static mesh of a cube
			*/

			TDE2_API static IStaticMesh* CreateCube(IResourceManager* pResourceManager);

			/*!
				\brief The function creates a new static mesh resource which is a 10x10 cells plane 
				
				\param[in, out] pResourceManager A pointer to IResourceManager

				\return The function returns a pointer to a new static mesh resource which is a 10x10 cells plane 
			*/

			TDE2_API static IStaticMesh* CreatePlane(IResourceManager* pResourceManager);

			/*!
				\brief The function creates a new spehere's mesh which is actually rounded cube to provide better uv mapping

				\param[in, out] pResourceManager A pointer to IResourceManager

				\return The function creates a new spehere's mesh which is actually rounded cube to provide better uv mapping
			*/

			TDE2_API static IStaticMesh* CreateSphere(IResourceManager* pResourceManager);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMesh)

			TDE2_API std::vector<U8> _getIndicesArray(const E_INDEX_FORMAT_TYPE& indexFormat) const;

			TDE2_API const IResourceLoader* _getResourceLoader() override;
		protected:
			IGraphicsObjectManager* mpGraphicsObjectManager;

			TPositionsArray         mPositions;

			TVertexColorArray       mVertexColors;

			TNormalsArray           mNormals;
			
			TTangentsArray          mTangents;

			TTexcoordsArray         mTexcoords0;

			TIndicesArray           mIndices;

			IVertexBuffer*          mpSharedVertexBuffer;
			IVertexBuffer*          mpPositionOnlyVertexBuffer;

			IIndexBuffer*           mpSharedIndexBuffer;
	};



	/*!
		\brief A factory function for creation objects of CStaticMeshLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation
		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
		\param[in, out] pFileSystem A pointer to IFileSystem's implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateStaticMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													 E_RESULT_CODE& result);


	/*!
		class CStaticMeshLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CStaticMeshLoader : public CBaseObject, public IMeshLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateStaticMeshLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshLoader)
		protected:
			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CStaticMeshFactory's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateStaticMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CStaticMeshFactory

		\brief The class is an abstract factory of CStaticMesh objects that
		is used by a resource manager
	*/

	class CStaticMeshFactory : public CBaseObject, public IMeshFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateStaticMeshFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshFactory)
		protected:
			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}