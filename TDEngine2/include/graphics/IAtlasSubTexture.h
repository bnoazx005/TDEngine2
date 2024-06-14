/*!
	\brief IAtlasSubTexture.h
	\date 18.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Config.h"
#include "../core/IResource.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../math/TRect.h"
#include "ITexture.h"


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		struct TAtlasSubTextureParameters

		\brief The stucture contains fields for creation IAtlasSubTexture objects
	*/

	typedef struct TAtlasSubTextureParameters : TBaseResourceParameters
	{
		TDE2_API virtual ~TAtlasSubTextureParameters() = default;

		TResourceId mTextureAtlasId = TResourceId::Invalid;

		TRectI32 mTextureRectInfo{};
	} TAtlasSubTextureParameters, *TAtlasSubTextureParametersPtr;


	/*!
		interface IAtlasSubTexture

		\brief The interface describes a common functionality of a texture entity that refers to a region within a texture atlas
	*/

	class IAtlasSubTexture : public ITexture
	{
		public:
			TDE2_REGISTER_TYPE(IAtlasSubTexture);

			/*!
				\brief The method initializes an internal state of a texture. 

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation
				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation
				\param[in] name A resource's name
				\param[in] params Additional parameters of a texture

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, const TAtlasSubTextureParameters& params) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAtlasSubTexture)
	};


	class IAtlasSubTextureLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class IAtlasSubTextureFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};
}