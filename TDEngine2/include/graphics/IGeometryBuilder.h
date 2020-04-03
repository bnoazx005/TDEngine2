/*!
	\file IGeometryBuilder.h
	\date 15.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../core/IBaseObject.h"
#include "../utils/Config.h"
#include "../utils/Types.h"
#include "../math/TVector3.h"
#include "../math/TVector4.h"
#include <vector>


namespace TDEngine2
{
	/*!
		interface IGeometryBuilder

		\brief The interface represents a functionality of a builder of standard geometry meshes such as boxes, cylinders, spheres, etc
	*/

	class IGeometryBuilder : public virtual IBaseObject
	{
		public:
			typedef struct TGeometryData
			{
				typedef struct TVertex
				{
					TVector4 mPosition;
					TVector3 mUV;
				} TVertex, *TVertexPtr;

				std::vector<TVertex> mVertices;
				
				std::vector<U16>     mIndices;
			} TGeometryData, *TGeometryDataPtr;
		public:
			/*!
				\brief The method initializes an internal state of a builder
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;

			//TDE2_API virtual TGeometryData CreateBoxGeometry() = 0;
			

			/*!
				\brief The method generates geometry for a cylinder which is aligned along specified axis

				\param[in] position A position of a cylinder's center
				\param[in] axis An axis along which the cylinder is aligned to
				\param[in] raidus A radius of the cylinder
				\param[in] height A height of the cylinder
				\param[in] segmentsCount A number of line segments that forms cylinder's base
			*/

			TDE2_API virtual TGeometryData CreateCylinderGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount) = 0;

			/*!
				\brief The method generates geometry for a cone which lies at plane with normal equals to given axis and centered along poisition point

				\param[in] position A position of a cone's center
				\param[in] axis An axis along which the cone is aligned to
				\param[in] raidus A radius of the cone's base
				\param[in] height A height of the cone
				\param[in] segmentsCount A number of line segments that forms cone's base
			*/

			TDE2_API virtual TGeometryData CreateConeGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount) = 0;

			/*!
				\brief The mehtod generates geometry for a plane with given parameters

				\param[in] position A position of plane's center
				\param[in] normal Plane's normal
				\param[in] width Plane's width
				\param[in] height Plane's height
				\param[in] segmentsPerSide The number defines degree of plane's subdivision

				\return The method returns an object which contains geometry data of a plane
			*/

			TDE2_API virtual TGeometryData CreatePlaneGeometry(const TVector3& position, const TVector3& normal, F32 width, F32 height, U16 segmentsPerSide) = 0;

			//TDE2_API virtual TGeometryData CreateSphereGeometry() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IGeometryBuilder)
	};
}