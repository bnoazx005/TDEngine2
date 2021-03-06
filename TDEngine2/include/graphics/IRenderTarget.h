/*!
	\file IRenderTarget.h
	\date 09.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ITexture2D.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;


	/*!
		interface IRenderTarget

		\brief The interface describes a common functionality of a render target (frame buffer)
	*/

	class IRenderTarget : public ITexture
	{
		public:
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
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderTarget)
	};


	/*!
		interface IRenderTargetFactory

		\brief The interface describes a functionality of a render target factory
	*/

	class IRenderTargetFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IRenderTargetFactory)
	};
}
