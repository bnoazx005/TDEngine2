/*!
	\file IMesh.h
	\date 10.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../math/TVector2.h"
#include "./../math/TVector4.h"
#include "./../core/IResource.h"
#include "./../core/IResourceFactory.h"
#include "./../core/IResourceLoader.h"
#include "./../utils/Color.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	class IVertexBuffer;
	class IIndexBuffer;


	/*!
		struct TMesh2DParameters

		\brief The stucture contains fields for creation IMesh and its derived classes
	*/

	typedef struct TMeshParameters : TBaseResourceParameters
	{
	} TMeshParameters, *TMeshParametersPtr;


	/*!
		interface IMesh

		\brief The interface describes a functionality of a common mesh
	*/

	class IMesh
	{
		public:
			typedef std::vector<TVector4>  TPositionsArray;
			typedef std::vector<TColor32F> TVertexColorArray;
			typedef std::vector<TVector4>  TNormalsArray;
			typedef std::vector<TVector4>  TTangentsArray;
			typedef std::vector<TVector2>  TTexcoordsArray;
			typedef std::vector<U32>       TIndicesArray;
		public:

			/*!
				\brief The method adds a new point into the array of mesh's positions

				\param[in] pos A 3d point which represents a position of a mesh's vertex
			*/

			TDE2_API virtual void AddPosition(const TVector4& pos) = 0;

			/*!
				\brief The method adds a new color for mesh's vertex

				\param[in] color A color of a vertex
			*/

			TDE2_API virtual void AddColor(const TColor32F& color) = 0;

			/*!
				\brief The method adds a new point into the array of mesh's normals

				\param[in] normal A 3d vector which defines a normal
			*/

			TDE2_API virtual void AddNormal(const TVector4& normal) = 0;

			/*!
				\brief The method adds a new point into the array of mesh's tangents

				\param[in] tangent A 3d vector which defines a tangent vector
			*/

			TDE2_API virtual void AddTangent(const TVector4& tangent) = 0;

			/*!
				\brief The method adds a new texture coordinates for first texture channel

				\param[in] uv0 A 2d point which represents a texture coordinates
			*/

			TDE2_API virtual void AddTexCoord0(const TVector2& uv0) = 0;

			/*!
				\brief The method adds an information about a face into the internal array

				\param[in] face An array with indices that form a face
			*/

			TDE2_API virtual void AddFace(const U32 face[3]) = 0;

			TDE2_API virtual const TPositionsArray& GetPositionsArray() const = 0;
			TDE2_API virtual const TVertexColorArray& GetColorsArray() const = 0;
			TDE2_API virtual const TNormalsArray& GetNormalsArray() const = 0;
			TDE2_API virtual const TTangentsArray& GetTangentsArray() const = 0;
			TDE2_API virtual const TTexcoordsArray& GetTexCoords0Array() const = 0;
			TDE2_API virtual const TIndicesArray& GetIndices() const = 0;

			TDE2_API virtual bool HasColors() const = 0;
			TDE2_API virtual bool HasNormals() const = 0;
			TDE2_API virtual bool HasTangents() const = 0;
			TDE2_API virtual bool HasTexCoords0() const = 0;

			/*!
				\brief The method converts current data of the mesh's resource into
				array of vertex entries like the following vertex | vertex | ... | vertex

				\return An array that contains contiguous region of data
			*/

			TDE2_API virtual std::vector<U8> ToArrayOfStructsDataLayout() const = 0;

			/*!
				\brief The method returns a number of faces in the mesh

				\return The method returns a number of faces in the mesh
			*/

			TDE2_API virtual U32 GetFacesCount() const = 0;

			/*!
				\brief The method returns a pointer to a vertex buffer that contains only positions of vertices

				\return The method returns a pointer to a vertex buffer that contains only positions of vertices
			*/

			TDE2_API virtual IVertexBuffer* GetPositionOnlyVertexBuffer() const = 0;

			/*!
				\brief The method process internal state of the mesh after it was actually loaded
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/
			
			TDE2_API virtual E_RESULT_CODE PostLoad() = 0;

			/*!
				\brief The method returns a pointer to IVertexBuffer which stores all vertex data of the mesh
				
				\return The method returns a pointer to IVertexBuffer which stores all vertex data of the mesh
			*/

			TDE2_API virtual IVertexBuffer* GetSharedVertexBuffer() const = 0;
			
			/*!
				\brief The method returns a pointer to IIndexBuffer which stores all index data of the mesh

				\return The method returns a pointer to IIndexBuffer which stores all index data of the mesh
			*/

			TDE2_API virtual IIndexBuffer* GetSharedIndexBuffer() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMesh)
	};


	/*!
		interface IStaticMesh

		\brief The interface describes a functionality of static meshes
	*/

	class IStaticMesh : public IMesh
	{
		public:
			TDE2_REGISTER_TYPE(IStaticMesh);

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a mesh

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TMeshParameters& params) = 0;

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMesh)
	};


	/*!
		interface ISkinnedMesh

		\brief The interface describes a functionality of skinned animable meshes
	*/

	class ISkinnedMesh : public IMesh
	{
		public:
			typedef std::vector<F32> TJointsWeightsArray;
			typedef std::vector<U16> TJointsIndicesArray;
		public:
			TDE2_REGISTER_TYPE(ISkinnedMesh);

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method initializes an internal state of a mesh object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a mesh

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TMeshParameters& params) = 0;

			TDE2_API virtual void AddVertexJointWeights(const TJointsWeightsArray& weights) = 0;
			TDE2_API virtual void AddVertexJointIndices(const TJointsIndicesArray& weights) = 0;

			TDE2_API virtual const TPositionsArray& GetJointWeightsArray() const = 0;
			TDE2_API virtual const TPositionsArray& GetJointIndicesArray() const = 0;

			TDE2_API virtual bool HasJointWeights() const = 0;
			TDE2_API virtual bool HasJointIndices() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ISkinnedMesh)
	};


	class IMeshLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class IMeshFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}