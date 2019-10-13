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
#include <string>
#include <vector>


namespace TDEngine2
{
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
			typedef std::vector<TVector4> TPositionsArray;
			typedef std::vector<TVector4> TNormalsArray;
			typedef std::vector<TVector4> TTangentsArray;
			typedef std::vector<TVector2> TTexcoordsArray;
			typedef std::vector<U32>      TIndicesArray;
		public:
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

			/*!
				\brief The method adds a new point into the array of mesh's positions

				\param[in] pos A 3d point which represents a position of a mesh's vertex
			*/

			TDE2_API virtual void AddPosition(const TVector4& pos) = 0;

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

			TDE2_API virtual void AddFace(F32 face[3]) = 0;

			TDE2_API virtual const TPositionsArray& GetPositionsArray() const = 0;
			TDE2_API virtual const TNormalsArray& GetNormalsArray() const = 0;
			TDE2_API virtual const TTangentsArray& GetTangentsArray() const = 0;
			TDE2_API virtual const TTexcoordsArray& GetTexCoords0Array() const = 0;
			TDE2_API virtual const TIndicesArray& GetIndices() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMesh)
	};


	/*!
		interface IMeshLoader

		\brief The interface describes a functionality of a mesh loader
	*/

	class IMeshLoader : public IResourceLoader
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
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMeshLoader)
	};


	/*!
		interface IStaticMeshFactory

		\brief The interface represents a functionality of a factory of IMesh objects
	*/

	class IMeshFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMeshFactory)
	};
}