/*!
	\file CDebugUtility.h
	\date 17.09.2019
	\authors Kasimov Ildar
*/

#pragma once


#include "IDebugUtility.h"
#include "./../core/CBaseObject.h"
#include "./../math/TVector4.h"
#include <vector>


namespace TDEngine2
{
	class CRenderQueue;
	class IVertexDeclaration;
	class IVertexBuffer;
	class IIndexBuffer;
	class IResourceHandler;


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
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

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
				\param[in] str A UTF-8 encoded string with text data
				\param[in] color A text's color
			*/

			TDE2_API void DrawText(const TVector2& screenPos, const CU8String& str, const TColor32F& color) override;

			/*!
				\brief The method draws an axis-aligned cross with specified parameters

				\param[in] position A position at which cross will be drawn
				\param[in] size A size of a cross
				\param[in] color A color of a cross
			*/

			TDE2_API void DrawCross(const TVector3& position, F32 size, const TColor32F& color) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CDebugUtility)

			TDE2_API std::vector<U16> _buildTextIndexBuffer(U32 textLength) const;
		protected:
			IGraphicsObjectManager*  mpGraphicsObjectManager;

			IResourceManager*        mpResourceManager;

			CRenderQueue*            mpRenderQueue;

			IVertexDeclaration*      mpLinesVertDeclaration;

			IVertexBuffer*           mpLinesVertexBuffer;

			std::vector<TLineVertex> mLinesDataBuffer;

			IResourceHandler*        mpSystemFont;

			IVertexDeclaration*      mpTextVertDeclaration;

			IVertexBuffer*           mpTextVertexBuffer;

			IIndexBuffer*            mpTextIndexBuffer;

			std::vector<TTextVertex> mTextDataBuffer;

			std::vector<TLineVertex> mCrossesDataBuffer;

			IVertexBuffer*           mpCrossesVertexBuffer;

			const static std::string mDefaultDebugMaterialName;

			const static std::string mTextMaterialName;
	};
}