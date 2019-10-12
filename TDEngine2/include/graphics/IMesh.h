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
#include "./../ecs/IComponentFactory.h"
#include <string>
#include <vector>


namespace TDEngine2
{
	/*!
		interface IMesh

		\brief The interface describes a functionality of a common mesh
	*/

	class IMesh : public virtual IBaseObject
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
		public:
			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

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

			TDE2_API virtual const TPositionsArray& GetPositionsArray() const = 0;
			TDE2_API virtual const TNormalsArray& GetNormalsArray() const = 0;
			TDE2_API virtual const TTangentsArray& GetTangentsArray() const = 0;
			TDE2_API virtual const TTexcoordsArray& GetTexCoords0Array() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IStaticMesh)
	};


	/*!
		interface IStaticMeshFactory

		\brief The interface represents a functionality of a factory of IStaticMesh objects
	*/

	class IStaticMeshFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
	};
}