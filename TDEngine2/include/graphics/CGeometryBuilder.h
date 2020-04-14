/*!
	\file CGeometryBuilder.h
	\date 15.03.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "IGeometryBuilder.h"
#include "../core/CBaseObject.h"


namespace TDEngine2
{	
	/*!
		\brief A factory function for creation objects of CGeometryBuilder's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CGeometryBuilder's implementation
	*/

	TDE2_API IGeometryBuilder* CreateGeometryBuilder(E_RESULT_CODE& result);

	/*!
		class CGeometryBuilder

		\brief The class implements a functionality of a builder of standard geometry meshes such as boxes, cylinders, spheres, etc
	*/

	class CGeometryBuilder : public CBaseObject, public IGeometryBuilder
	{
		public:
			friend TDE2_API IGeometryBuilder* CreateGeometryBuilder(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a builder
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method generates geometry for a cube which is axis aligned

				\param[in] position A position of a cube's center
				\param[in] size A size of each side of the cube

				\return  The method generates geometry for a cube which is axis aligned
			*/

			TDE2_API TGeometryData CreateCubeGeometry(const TVector3& position, F32 size) override;

			/*!
				\brief The method generates geometry for a cylinder which is aligned along specified axis

				\param[in] position A position of a cylinder's center
				\param[in] axis An axis along which the cylinder is aligned to
				\param[in] raidus A radius of the cylinder
				\param[in] height A height of the cylinder
				\param[in] segmentsCount A number of line segments that forms cylinder's base
			*/

			TDE2_API TGeometryData CreateCylinderGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount) override;

			/*!
				\brief The method generates geometry for a cone which lies at plane with normal equals to given axis and centered along poisition point

				\param[in] position A position of a cone's center
				\param[in] axis An axis along which the cone is aligned to
				\param[in] raidus A radius of the cone's baseCGeometryBuilder::
				\param[in] segmentsCount A number of line segments that forms cone's base
			*/

			TDE2_API TGeometryData CreateConeGeometry(const TVector3& position, const TVector3& axis, F32 radius, F32 height, U16 segmentsCount) override;

			/*!
				\brief The mehtod generates geometry for a plane with given parameters

				\param[in] position A position of plane's center
				\param[in] normal Plane's normal
				\param[in] width Plane's width
				\param[in] height Plane's height
				\param[in] segmentsPerSide The number defines degree of plane's subdivision

				\return The method returns an object which contains geometry data of a plane
			*/

			TDE2_API TGeometryData CreatePlaneGeometry(const TVector3& position, const TVector3& normal, F32 width, F32 height, U16 segmentsPerSide) override;

			/*!
				\brief The method generates translation gizmo for one of following types TRANSLATE_X, TRANSLATE_Y and TRANSLATE_Z

				\param[in] type A type of a translation gizmo

				\return The method returns an object which contains geometry data of a translation gizmo
			*/

			TDE2_API TGeometryData CreateTranslateGizmo(E_GIZMO_TYPE type) override;

			/*!
				\brief The method generates rotation Gizmo for one of following types ROTATION_X, ROTATION_Y and ROTATION_Z

				\param[in] type A type of a rotation gizmo

				\return The method returns an object which contains geometry data of a rotation gizmo
			*/

			TDE2_API TGeometryData CreateRotationGizmo(E_GIZMO_TYPE type) override;

			/*!
				\brief The method generates scaling gizmo for one of following types SCALE_X, SCALE_Y and SCALE_Z

				\param[in] type A type of a scaling gizmo

				\return The method returns an object which contains geometry data of a scaling gizmo
			*/

			TDE2_API TGeometryData CreateScaleGizmo(E_GIZMO_TYPE type) override;

			/*!
				\brief The mehtod generates geometry for a solid arc

				\param[in] position A position of arc's center
				\param[in] normal Arc's normal
				\param[in] raidus Arc's radius
				\param[in] fromAngle Angle in radians
				\param[in] toAngle Angle in radians
				\param[in] isLooped The flag tells whether geometry of arc should be closed (forms a circle) or not
				\param[in] width Arc's width
				\param[in] segmentsCount The parameter defines level of details of the arg

				\return The method returns an object which contains geometry data of a plane
			*/

			TDE2_API TGeometryData CreateSolidArcGeometry(const TVector3& position, const TVector3& normal, F32 radius,
														  F32 fromAngle, F32 toAngle, bool isLooped, F32 width, U16 segmentsCount) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CGeometryBuilder)
	};
}