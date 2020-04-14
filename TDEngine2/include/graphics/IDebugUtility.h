/*!
	\file IDebugUtility.h
	\date 17.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Config.h"
#include "./../utils/Types.h"
#include "./../core/IBaseObject.h"
#include "./../math/TVector2.h"
#include "./../math/TVector3.h"
#include "./../math/TRect.h"
#include "./../utils/Color.h"
#include "./../utils/CU8String.h"


namespace TDEngine2
{
	class IResourceManager;
	class IRenderer;
	class IGraphicsObjectManager;
	struct TAABB;
	struct TMatrix4;


	/*!
		enum class E_GIZMO_TYPE

		\brief The enumeration contains all types of gizmos that support the engine
	*/

	enum class E_GIZMO_TYPE: U8
	{
		TRANSLATION,
		TRANSLATION_X,
		TRANSLATION_Y,
		TRANSLATION_Z,
		TRANSLATION_XY,
		TRANSLATION_XZ,
		TRANSLATION_YZ,
		ROTATION,
		ROTATION_X,
		ROTATION_Y,
		ROTATION_Z,
		SCALING,
		SCALING_X,
		SCALING_Y,
		SCALING_Z,
		LAST
	};

	TDE2_API inline E_GIZMO_TYPE operator+ (E_GIZMO_TYPE type, U8 value) { return static_cast<E_GIZMO_TYPE>(static_cast<U8>(type) + value); }


	/*!
		interface IDebugUtility

		\brief The interfaces describes a functionality of a debug helper,
		which provides methods to draw gizmos, lines, text, to simplify
		debugging process
	*/

	class IDebugUtility: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of the object

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager interface
				\param[in, out] pRenderer A pointer to implementation of IRenderer interface
				\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager) = 0;

			/*!
				\brief The method adds a request to draw a line with a given parameters to rendering queue

				\param[in] start A start position of a line's segment
				\param[in] end An end position of a line's segment
				\param[in] color A line's color
			*/

			TDE2_API virtual void DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color) = 0;

			/*!
				\brief The method draws a text onto a screen. Note that method should be used for debug purposes
				only, use normal text rendering techniques for production

				\param[in] start A screen-space position of a text
				\param[in] scale A sizes of a text's mesh
				\param[in] str A UTF-8 encoded string with text data
				\param[in] color A text's color
			*/

			TDE2_API virtual void DrawText(const TVector2& screenPos, F32 scale, const CU8String& str, const TColor32F& color) = 0;

			/*!
				\brief The method draws an axis-aligned cross with specified parameters

				\param[in] position A position at which cross will be drawn
				\param[in] size A size of a cross
				\param[in] color A color of a cross
			*/

			TDE2_API virtual void DrawCross(const TVector3& position, F32 size, const TColor32F& color) = 0;

			/*!
				\brief The method draws a rectange on top of the screen. All coordinates of the rectangle
				should be given in world space

				\param[in] rect A rectangle should be drawn
				\param[in] color A color of a cross of the rectangle
			*/

			TDE2_API virtual void DrawRect(const TRectF32& rect, const TColor32F& color) = 0;

			/*!
				\brief The method draws an axis aligned bounding box

				\param[in] aabb An axis aligned bounding box that will be drawn
				\param[in] color A color of drawn AABB
			*/

			TDE2_API virtual void DrawAABB(const TAABB& aabb, const TColor32F& color) = 0;

			/*!
				\brief The method draws a circle with specified number of segments

				\param[in] position A position of circle's center
				\param[in] planeNormal A normal of the plane in which the circle lies
				\param[in] radius Circle's radius
				\param[in] color A color of the circle
				\param[in] segmentsCount A number of circle's segments
			*/

			TDE2_API virtual void DrawCircle(const TVector3& position, const TVector3& planeNormal, F32 radius, const TColor32F& color, U16 segmentsCount = 16) = 0;

			/*!
				\brief the method draws a sphere with specified parameters

				\param[in] position A position of sphere's center
				\param[in] radius Sphere's radius
				\param[in] color A color of the sphere
				\param[in] triangulationFactor A number of triangles per each sphere's quadrant
			*/

			TDE2_API virtual void DrawSphere(const TVector3& position, F32 radius, const TColor32F& color, U16 triangulationFactor = 1) = 0;

			/*!
				\brief The method draws a gizmo of one of predefined types for given transform matrix

				\param[in] type A type of a gizmo
				\param[in] transform A matrix which stores translation, rotation and scaling data
				\param[in] size A scale of a gizmo
			*/

			TDE2_API virtual void DrawTransformGizmo(E_GIZMO_TYPE type, const TMatrix4& transform, F32 size = 1.0f) = 0;

			/*!
				\brief The method prepares internal state of the helper before it will be actually rendered
			*/

			TDE2_API virtual void PreRender() = 0;

			/*!
				\brief The method resets current state of the helper
			*/

			TDE2_API virtual void PostRender() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDebugUtility)
	};
}