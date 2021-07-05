/*!
	\file CBaseMesh.h
	\date 05.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "IMesh.h"
#include "../core/CBaseResource.h"
#include "../graphics/IIndexBuffer.h"


namespace TDEngine2
{
	class IGraphicsObjectManager;

	/*!
		class CBaseMesh

		\brief The class gathers common logic of all types of meshes
	*/

	class CBaseMesh: public virtual IMesh, public CBaseResource
	{
		public:
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
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMesh)

			TDE2_API std::vector<U8> _toArrayOfStructsDataLayoutInternal() const;
			TDE2_API std::vector<U8> _getIndicesArray(const E_INDEX_FORMAT_TYPE& indexFormat) const;

			TDE2_API bool _hasColorsInternal() const;
			TDE2_API bool _hasNormalsInternal() const;
			TDE2_API bool _hasTangentsInternal() const;
			TDE2_API bool _hasTexCoords0Internal() const;

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
}