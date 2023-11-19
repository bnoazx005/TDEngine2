/*!
	\file COGLTexture2D.h
	\date 20.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <core/CBaseObject.h>
#include <graphics/CBaseTexture2D.h>
#include <GL/glew.h>


namespace TDEngine2
{
	TDE2_API ITextureImpl* CreateOGLTextureImpl(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params, E_RESULT_CODE& result);


	/*!
		class COGLTextureImpl
	*/

	class COGLTextureImpl : public virtual ITextureImpl, public CBaseObject
	{
		public:
			friend TDE2_API ITextureImpl* CreateOGLTextureImpl(IGraphicsContext*, const TInitTextureImplParams&, E_RESULT_CODE&);
		public:
			/*!
				\brief The method initializes an initial state of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext, const TInitTextureImplParams& params) override;

			TDE2_API E_RESULT_CODE Resize(U32 width, U32 height, U32 depth = 1) override;

			TDE2_API GLuint GetTextureHandle();

			TDE2_API std::vector<U8> ReadBytes(U32 index) override;

			TDE2_API const TInitTextureParams& GetParams() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLTextureImpl)

			TDE2_API E_RESULT_CODE _onInitInternal();
			TDE2_API E_RESULT_CODE _onFreeInternal() override;
		protected:
			TInitTextureImplParams     mInitParams;

			GLuint mTextureHandle = 0;
	};
}