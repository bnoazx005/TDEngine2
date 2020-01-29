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