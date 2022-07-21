/*!
	\file IRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture2D.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture2D;


	/*!
		interface IRenderTarget

		\brief The interface describes a common functionality of a render target (frame buffer)
	*/

	class IRenderTarget : public ITexture
	{
		public:
			TDE2_REGISTER_TYPE(IRenderTarget);

			/*!
				\brief The method initializes an internal state of a render target

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
			
				\param[in] params Additional parameters of a render target

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, 
												const TTexture2DParameters& params) = 0;

			/*!
				\brief The method copies existing data of the render target into given texture object

				\param[in] pDestTexture A pointer to ITexture2D implementation, the object should be already initialized

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Blit(ITexture2D*& pDestTexture) = 0;

			TDE2_API virtual void UnbindFromShader() = 0;

			/*!
				\brief The method allows to resize internal resources that corresponds to the given render target
			*/

			TDE2_API virtual E_RESULT_CODE Resize(U32 width, U32 height) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderTarget)
	};


	/*!
		interface IRenderTargetFactory

		\brief The interface describes a functionality of a render target factory
	*/

	class IRenderTargetFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};


	/*!
		interface IDepthBufferTarget

		\brief The interface defines functionality of depth buffer which can be used as a target for rendering
	*/

	class IDepthBufferTarget : public virtual IRenderTarget
	{
		public:
			TDE2_REGISTER_TYPE(IDepthBufferTarget);
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDepthBufferTarget)
	};


	/*!
		interface IDepthBufferTargetFactory

		\brief The interface is a factory which can create an objects of IDepthBufferTarget type
	*/

	class IDepthBufferTargetFactory : public IRenderTargetFactory
	{
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDepthBufferTargetFactory)
	};
}
