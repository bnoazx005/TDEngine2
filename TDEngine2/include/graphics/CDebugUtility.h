/*!
	\file CDebugUtility.h
	\date 17.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IDebugUtility.h"
#include "../core/CBaseObject.h"
#include "../math/TVector4.h"
#include "../math/TMatrix4.h"
#include <vector>
#include <unordered_map>
#include <mutex>


namespace TDEngine2
{
	class CFramePacketsStorage;
	class IVertexDeclaration;
	class IGeometryBuilder;


	enum class TBufferHandleId : U32;


	/*!
		\brief A factory function for creation objects of IDebugUtility's type.

		\param[in, out] pResourceManager A pointer to implementation of IResourceManager interface
		\param[in, out] pRenderer A pointer to implementation of IRenderer interface
		\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IDebugUtility's implementation
	*/

	TDE2_API IDebugUtility* CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer, 
											   IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CDebugUtility

		\brief The class implements debug helper to
		draw information on a screen
	*/

	class CDebugUtility : public IDebugUtility, public CBaseObject
	{
		public:
			friend TDE2_API IDebugUtility* CreateDebugUtility(IResourceManager* pResourceManager, IRenderer* pRenderer, 
															  IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
		public:
			struct TLineVertex
			{
				TVector4  mPos;
				TColor32F mColor;
			};

			struct TTextVertex
			{
				TVector4 mPosUV;
			};

			struct TGizmoData
			{
				E_GIZMO_TYPE mType;
				TMatrix4     mTransform;
				F32          mScale;
			};

			struct TGizmoTypeInfo
			{
				U32 mIndicesCount;
				U32 mStartVertex;
				U32 mStartIndex;
			};

			typedef std::unordered_map<E_GIZMO_TYPE, TGizmoTypeInfo> TGizmoTypesMap;
		public:
			/*!
				\brief The method initializes an initial state of the object

				\param[in, out] pResourceManager A pointer to implementation of IResourceManager interface
				\param[in, out] pRenderer A pointer to implementation of IRenderer interface
				\param[in, out] pGraphicsObjectManager A pointer to implementation of IGraphicsObjectManager interface

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager) override;

			/*!
				\brief The method prepares internal state of the helper before it will be actually rendered
			*/

			TDE2_API void PreRender() override;

			/*!
				\brief The method resets current state of the helper
			*/

			TDE2_API void PostRender() override;

			/*!
				\brief The method adds a request to draw a line with a given parameters to rendering queue

				\param[in] start A start position of a line's segment
				\param[in] end An end position of a line's segment
				\param[in] color A line's color
			*/

			TDE2_API void DrawLine(const TVector3& start, const TVector3& end, const TColor32F& color) override;

			/*!
				\brief The method draws a text onto a screen. Note that method should be used for debug purposes
				only, use normal text rendering techniques for production

				\param[in] start A screen-space position of a text
				\param[in] scale A sizes of a text's mesh
				\param[in] str A UTF-8 encoded string with text data
				\param[in] color A text's color
			*/

			TDE2_API void DrawText(const TVector2& screenPos, F32 scale, const std::string& str, const TColor32F& color) override;

			/*!
				\brief The method draws an axis-aligned cross with specified parameters

				\param[in] position A position at which cross will be drawn
				\param[in] size A size of a cross
				\param[in] color A color of a cross
			*/

			TDE2_API void DrawCross(const TVector3& position, F32 size, const TColor32F& color) override;
			
			/*!
				\brief The method draws a rectange on top of the screen. All coordinates of the rectangle
				should be given in world space

				\param[in] rect A rectangle should be drawn
				\param[in] color A color of a cross of the rectangle
			*/

			TDE2_API void DrawRect(const TRectF32& rect, const TColor32F& color) override;

			/*!
				\brief The method draws an axis aligned bounding box

				\param[in] aabb An axis aligned bounding box that will be drawn
				\param[in] color A color of drawn AABB
			*/

			TDE2_API void DrawAABB(const TAABB& aabb, const TColor32F& color) override;

			/*!
				\brief The method draws a circle with specified number of segments

				\param[in] position A position of circle's center
				\param[in] planeNormal A normal of the plane in which the circle lies
				\param[in] radius Circle's radius
				\param[in] color A color of the circle
				\param[in] segmentsCount A number of circle's segments
			*/

			TDE2_API void DrawCircle(const TVector3& position, const TVector3& planeNormal, F32 radius, const TColor32F& color, U16 segmentsCount = 16) override;

			/*!
				\brief the method draws a sphere with specified parameters

				\param[in] position A position of sphere's center
				\param[in] radius Sphere's radius
				\param[in] color A color of the sphere
				\param[in] triangulationFactor A number of triangles per each sphere's quadrant
			*/

			TDE2_API void DrawSphere(const TVector3& position, F32 radius, const TColor32F& color, U16 triangulationFactor = 1) override;
			
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDebugUtility)

			TDE2_API std::vector<U16> _buildTextIndexBuffer(U32 textLength) const;

			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			IGraphicsObjectManager*  mpGraphicsObjectManager;

			IResourceManager*        mpResourceManager;

			CFramePacketsStorage*    mpFramePacketsStorage = nullptr;

			IVertexDeclaration*      mpLinesVertDeclaration;

			TBufferHandleId          mLinesVertexBufferHandle;

			std::vector<TLineVertex> mLinesDataBuffer;

			TResourceId              mSystemFontHandle;

			IVertexDeclaration*      mpTextVertDeclaration;

			TBufferHandleId          mTextVertexBufferHandle;
			TBufferHandleId          mTextIndexBufferHandle;

			std::vector<TTextVertex> mTextDataBuffer;

			std::vector<TLineVertex> mCrossesDataBuffer;

			TBufferHandleId          mCrossesVertexBufferHandle;

			const static U32         mMaxLinesVerticesCount;

			const static std::string mDefaultDebugMaterialName;

			const static std::string mTextMaterialName;

			IGeometryBuilder*        mpGeometryBuilder;

			mutable std::mutex       mMutex;
	};
}