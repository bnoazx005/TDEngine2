/*!
	\file CStaticMesh.h
	\date 10.10.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IMesh.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CStaticMesh's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMesh's implementation
	*/

	TDE2_API IComponent* CreateStaticMesh(E_RESULT_CODE& result);


	/*!
		class CStaticMesh

		\brief The class describes a functionality of a static mesh
	*/

	class CStaticMesh: public IStaticMesh, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateStaticMesh(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_TYPE(CStaticMesh)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method adds a new point into the array of mesh's positions

				\param[in] pos A 3d point which represents a position of a mesh's vertex
			*/

			TDE2_API void AddPosition(const TVector4& pos) override;

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

			TDE2_API void AddFace(F32 face[3]) override;

			TDE2_API const TPositionsArray& GetPositionsArray() const override;
			TDE2_API const TNormalsArray& GetNormalsArray() const override;
			TDE2_API const TTangentsArray& GetTangentsArray() const override;
			TDE2_API const TTexcoordsArray& GetTexCoords0Array() const override;
			TDE2_API const TIndicesArray& GetIndices() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMesh)
		protected:
			TPositionsArray mPositions;

			TNormalsArray   mNormals;
			
			TTangentsArray  mTangents;

			TTexcoordsArray mTexcoords0;

			TIndicesArray   mIndices;
	};


	/*!
		struct TStaticMeshParameters

		\brief The structure contains parameters for creation of CStaticMesh
	*/

	typedef struct TStaticMeshParameters : public TBaseComponentParameters
	{
	} TStaticMeshParameters;


	/*!
		\brief A factory function for creation objects of CStaticMeshFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CStaticMeshFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateStaticMeshFactory(E_RESULT_CODE& result);


	/*!
		class CStaticMeshFactory

		\brief The class is factory facility to create a new objects of CStaticMesh type
	*/

	class CStaticMeshFactory : public IStaticMeshFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateStaticMeshFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CStaticMeshFactory)
	};
}