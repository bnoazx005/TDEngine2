#include "../../include/graphics/CTextureAtlas.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IResourceManager.h"
#include "../../include/core/CBaseFileSystem.h"
#include "../../include/utils/Utils.h"
#include "../../include/graphics/CBaseTexture2D.h"
#include "../../include/graphics/IAtlasSubTexture.h"
#include "../../include/core/IFile.h"
#include "../../include/platform/CYAMLFile.h"
#include "stringUtils.hpp"
#include "deferOperation.hpp"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../../deps/stb/stb_rect_pack.h"
#include <algorithm>
#include <stack>
#include <cmath>


namespace TDEngine2
{
	CTextureAtlas::TTextureAtlasEntry::TTextureAtlasEntry(const std::string& name, const TRectI32& rect, const CTextureAtlas::TTextureAtlasEntry::TRawTextureData& texture):
		mName(name),
		mRect(rect),
		mIsRawData(true)
	{
		mData = texture;
	}
	
	CTextureAtlas::TTextureAtlasEntry::TTextureAtlasEntry(const std::string& name, const TRectI32& rect, TPtr<ITexture2D> pTexture) :
		mName(name),
		mRect(rect),
		mIsRawData(false)
	{
		mData = pTexture;
	}


	CTextureAtlas::CTextureAtlas() :
		CBaseResource(), mTextureResourceHandle(TResourceId::Invalid)
	{
	}

	E_RESULT_CODE CTextureAtlas::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
									  const TTexture2DParameters& params)
	{
		E_RESULT_CODE result = _init(pResourceManager, name);

		if (result != RC_OK)
		{
			return result;
		}

		if (!pGraphicsContext || !params.mHeight || !params.mWidth)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;
		
		/// \note create a texture using the resource manager
		if (TResourceId::Invalid == (mTextureResourceHandle = pResourceManager->Create<ITexture2D>(name + "_Tex", params)))
		{
			return RC_FAIL;
		}

		if (auto pTexture = pResourceManager->GetResource<ITexture2D>(mTextureResourceHandle))
		{
			pTexture->SetFilterType(E_TEXTURE_FILTER_TYPE::FT_BILINEAR);

			pTexture->SetUWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pTexture->SetVWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
			pTexture->SetWWrapMode(E_ADDRESS_MODE_TYPE::AMT_CLAMP);
		}

		mWidth  = params.mWidth;
		mHeight = params.mHeight;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Reset()
	{
		mAtlasEntities.clear();
		mpAtlasRectsContext = nullptr;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::AddRawTexture(const std::string& name, U32 width, U32 height, E_FORMAT_TYPE format, const U8* pData)
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (name.empty() || (format == FT_UNKNOWN) || !pData)
		{
			return RC_INVALID_ARGS;
		}

		/// \note check whether the texture's data is already within the atlas or not
		if (mAtlasEntities.find(name) != mAtlasEntities.cend())
		{
			return RC_FAIL;
		}

		/// \note add a new entry
		TRectI32 textureRect { 0, 0, static_cast<I32>(width), static_cast<I32>(height) };

		return _addNewEntityToAtlas(TTextureAtlasEntry{ name, textureRect, { pData, format } });
	}

	E_RESULT_CODE CTextureAtlas::AddTexture(TResourceId textureHandle)
	{
		if (TResourceId::Invalid == textureHandle)
		{
			return RC_INVALID_ARGS;
		}

		auto pTextureResource = mpResourceManager->GetResource<ITexture2D>(textureHandle);
		if (!pTextureResource)
		{
			return RC_FAIL;
		}

		const std::string textureName = dynamic_cast<IResource*>(pTextureResource.Get())->GetName();

		/// \note check whether the texture's data is already within the atlas or not
		if (mAtlasEntities.find(textureName) != mAtlasEntities.cend())
		{
			return RC_FAIL;
		}

		/// \note add a new entry
		TRectI32 textureRect{ 0, 0, static_cast<I32>(pTextureResource->GetWidth()), static_cast<I32>(pTextureResource->GetHeight()) };

		return _addNewEntityToAtlas(TTextureAtlasEntry{ textureName, textureRect, pTextureResource });
	}

	E_RESULT_CODE CTextureAtlas::RemoveTexture(const std::string& name)
	{
		if (name.empty())
		{
			return RC_INVALID_ARGS;
		}

		auto it = mAtlasEntities.find(name);
		
		if (it != mAtlasEntities.cend())
		{
			mAtlasEntities.erase(it);
			return RC_OK;
		}

		return RC_FAIL;
	}

	E_RESULT_CODE CTextureAtlas::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		auto pAtlasInternalTexture = mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle);

		/// \note save texture atlas into an image file
		/// \todo for now we save all atlases as png files, but it should be replaced with general solution
	//	 ->Get<IImageFileWriter>(pFileSystem->Open<IImageFileWriter>(mName + "_Tex.png", true).Get())->Write(pAtlasInternalTexture);
			
		PANIC_ON_FAILURE(pWriter->SetString("name", mName));

		E_RESULT_CODE result = RC_OK;

		/// \note write texture's information
		{
			if ((result = pWriter->BeginGroup("texture_resource")) != RC_OK)
			{
				return result;
			}

			PANIC_ON_FAILURE(pWriter->SetUInt32("width", pAtlasInternalTexture->GetWidth()));
			PANIC_ON_FAILURE(pWriter->SetUInt32("height", pAtlasInternalTexture->GetHeight()));
			PANIC_ON_FAILURE(pWriter->SetUInt32("channels_count", CFormatUtils::GetNumOfChannelsOfFormat(pAtlasInternalTexture->GetFormat())));

			if ((result = pWriter->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pWriter->BeginGroup("textures_list", true)) != RC_OK)
		{
			return result;
		}

		TRectI32 currBounds;

		for (auto currTextureEntity : mAtlasEntities)
		{
			currBounds = currTextureEntity.second;

			if ((result = pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr())) != RC_OK)
			{
				return result;
			}

			PANIC_ON_FAILURE(pWriter->SetString("name", currTextureEntity.first));

			{
				if ((result = pWriter->BeginGroup("bounds")) != RC_OK)
				{
					return result;
				}

				PANIC_ON_FAILURE(pWriter->SetInt32("x", currBounds.x));
				PANIC_ON_FAILURE(pWriter->SetInt32("y", currBounds.y));
				PANIC_ON_FAILURE(pWriter->SetInt32("width", currBounds.width));
				PANIC_ON_FAILURE(pWriter->SetInt32("height",currBounds.height));
				
				if ((result = pWriter->EndGroup()) != RC_OK)
				{
					return result;
				}
			}

			if ((result = pWriter->EndGroup()) != RC_OK)
			{
				return result;
			}
		}

		if ((result = pWriter->EndGroup()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}


	E_RESULT_CODE CTextureAtlas::Serialize(IFileSystem* pFileSystem, ITextureAtlas* pTextureAtlas, const std::string& filename)
	{
		if (!pFileSystem || !pTextureAtlas || filename.empty())
		{
			return RC_INVALID_ARGS;
		}

		ITexture2D* pAtlasInternalTexture = pTextureAtlas->GetTexture();

		IResource* pAtlasResource = dynamic_cast<IResource*>(pTextureAtlas);
		if (!pAtlasResource)
		{
			return RC_FAIL;
		}

		std::string atlasName = pAtlasResource->GetName();
		atlasName = atlasName.substr(0, atlasName.find_last_of('.'));

		/// \note save texture atlas into an image file
		/// \todo for now we save all atlases as png files, but it should be replaced with general solution
		pFileSystem->Get<IImageFileWriter>(pFileSystem->Open<IImageFileWriter>(atlasName + "_Tex.png", true).Get())->Write(pAtlasInternalTexture);

		/// \note try to create YAML file with the given name
		if (auto archiveHandle = pFileSystem->Open<IYAMLFileWriter>(filename, true))
		{
			if (auto pArchiveFile = pFileSystem->Get<IYAMLFileWriter>(archiveHandle.Get()))
			{
				defer([pArchiveFile] { pArchiveFile->Close(); });
				return pTextureAtlas->Save(pArchiveFile);
			}
		}

		return RC_FAIL;
	}


	E_RESULT_CODE CTextureAtlas::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		/// \note load texture based on read parameters
		mName = pReader->GetString("name");

		/// \todo for now we save all atlases as png files, but it should be replaced with general solution
		mTextureResourceHandle = mpResourceManager->Load<ITexture2D>(mName + "_Tex.png", E_RESOURCE_LOADING_POLICY::SYNCED);

		/// \todo ansynchronously update sizes of the atlas when the texture has been loaded
		_updateAtlasSizes(mpResourceManager->GetResource(mTextureResourceHandle).Get());

		if ((result = pReader->BeginGroup("textures_list")) != RC_OK)
		{
			return result;
		}

		std::string currEntryName;

		TRectI32 currBounds;

		while (pReader->HasNextItem())
		{
			if ((result = pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr())) != RC_OK)
			{
				return result;
			}

			currEntryName = pReader->GetString("name");

			// \note read bounds information
			{
				if ((result = pReader->BeginGroup("bounds")) != RC_OK)
				{
					return result;
				}
			
				currBounds.x      = pReader->GetInt32("x");
				currBounds.y      = pReader->GetInt32("y");
				currBounds.width  = pReader->GetInt32("width");
				currBounds.height = pReader->GetInt32("height");

				if ((result = pReader->EndGroup()) != RC_OK)
				{
					return result;
				}
			}

			if ((result = pReader->EndGroup()) != RC_OK)
			{
				return result;
			}

			mAtlasEntities.insert({ currEntryName, currBounds });

			_createSubTexture(currEntryName, currBounds);
		}

		if ((result = pReader->EndGroup()) != RC_OK)
		{
			return result;
		}

		return RC_OK;
	}

	ITexture2D* CTextureAtlas::GetTexture() const
	{
		return mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle).Get();
	}

	TResult<TRectI32> CTextureAtlas::GetTextureRect(const std::string& textureName) const
	{
		if (mAtlasEntities.find(textureName) == mAtlasEntities.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		return Wrench::TOkValue<TRectI32>(mAtlasEntities.at(textureName));
	}

	TResult<TRectF32> CTextureAtlas::GetNormalizedTextureRect(const std::string& textureName) const
	{
		if (mAtlasEntities.find(textureName) == mAtlasEntities.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		TRectI32 nonNormalizedRect = mAtlasEntities.at(textureName);

		F32 invWidth  = 1.0f / mWidth;
		F32 invHeight = 1.0f / mHeight;

		return Wrench::TOkValue<TRectF32>({ nonNormalizedRect.x * invWidth,
									nonNormalizedRect.y * invHeight, 
									nonNormalizedRect.width * invWidth, 
									nonNormalizedRect.height * invHeight });
	}

	std::vector<std::string> CTextureAtlas::GetTexturesIdentifiersList() const
	{
		std::vector<std::string> output;
		
		for (auto&& currAtlasEntity : mAtlasEntities)
		{
			output.push_back(currAtlasEntity.first);
		}
		
		return std::move(output);
	}

	void CTextureAtlas::_updateAtlasSizes(IResource* pTexture)
	{
		ITexture2D* pTexture2D = dynamic_cast<ITexture2D*>(pTexture);

		mWidth  = pTexture2D->GetWidth();
		mHeight = pTexture2D->GetHeight();
	}

	TResult<TResourceId> CTextureAtlas::_createSubTexture(const std::string& id, const TRectI32& rect)
	{
		TAtlasSubTextureParameters subTextureParams;
		subTextureParams.mTextureAtlasId = mId;
		subTextureParams.mTextureRectInfo = rect;

		TResourceId subTextureHandle = mpResourceManager->Create<IAtlasSubTexture>(id, subTextureParams);
		if (TResourceId::Invalid == subTextureHandle)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		mpSubTextures.push_back(mpResourceManager->GetResource<IAtlasSubTexture>(subTextureHandle).Get());

		return Wrench::TOkValue<TResourceId>(subTextureHandle);
	}

	const TPtr<IResourceLoader> CTextureAtlas::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ITextureAtlas>();
	}

	stbrp_context& CTextureAtlas::_getAtlasEntitiesContext()
	{
		/// \note Recreate the context also in the case when the size of the atlas was changed
		if (!mpAtlasRectsContext || (mpAtlasRectsContext && (mpAtlasRectsContext->width != mWidth || mpAtlasRectsContext->height != mHeight)))
		{
			mpAtlasRectsContext = std::make_unique<stbrp_context>();

			mAtlasNodes.resize(mWidth + 1); 

			stbrp_init_target(mpAtlasRectsContext.get(), mWidth, mHeight, &mAtlasNodes.front(), static_cast<I32>(mAtlasNodes.size()));
		}

		TDE2_ASSERT(mpAtlasRectsContext);
		return *mpAtlasRectsContext.get();
	}

	E_RESULT_CODE CTextureAtlas::_addNewEntityToAtlas(TTextureAtlasEntry&& entity)
	{
		stbrp_rect rect;
		rect.w  = entity.mRect.width;
		rect.h  = entity.mRect.height;
		
		auto& context = _getAtlasEntitiesContext();

		/// \note Update rects packing with a new entity
		if (!stbrp_pack_rects(&context, &rect, 1))
		{
			return RC_FAIL;
		}

		TDE2_ASSERT(rect.was_packed);

		TRectI32 textureRect;
		textureRect.x = rect.x;
		textureRect.y = rect.y;
		textureRect.width = rect.w;
		textureRect.height = rect.h;

		mAtlasEntities[entity.mName] = textureRect;

		E_RESULT_CODE result = RC_OK;

		/// \note Write data into the 
		if (auto pAtlasInternalTexture = mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle))
		{
			if (!entity.mIsRawData)
			{
				auto&& textureData = entity.mData.As<TPtr<ITexture2D>>()->GetInternalData();
				result = pAtlasInternalTexture->WriteData(textureRect, &textureData.front());
			}
			else
			{
				result = pAtlasInternalTexture->WriteData(textureRect, entity.mData.As<TTextureAtlasEntry::TRawTextureData>().mpData);
			}
		}

		TDE2_ASSERT(result == RC_OK);

		return result;
	}


	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureAtlas, CTextureAtlas, result, pResourceManager, pGraphicsContext, name, params);
	}


	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
											   E_RESULT_CODE& result)
	{
		return CREATE_IMPL(ITextureAtlas, CTextureAtlas, result, pResourceManager, pGraphicsContext, name);
	}


	/*!
		\brief CTextureAtlasLoader's definition
	*/

	CTextureAtlasLoader::CTextureAtlasLoader() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTextureAtlasLoader::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pResourceManager || !pGraphicsContext || !pFileSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpFileSystem = pFileSystem;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlasLoader::LoadResource(IResource* pResource) const
	{
		if (!pResource)
		{
			return RC_INVALID_ARGS;
		}

		if (TResult<TFileEntryId> atlasFileId = mpFileSystem->Open<IYAMLFileReader>(pResource->GetName()))
		{
			return dynamic_cast<ITextureAtlas*>(pResource)->Load(mpFileSystem->Get<IYAMLFileReader>(atlasFileId.Get()));
		}

		return RC_FILE_NOT_FOUND;
	}

	TypeId CTextureAtlasLoader::GetResourceTypeId() const
	{
		return ITextureAtlas::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceLoader, CTextureAtlasLoader, result, pResourceManager, pGraphicsContext, pFileSystem);
	}


	/*!
		\brief CTextureAtlasFactory's definition
	*/

	CTextureAtlasFactory::CTextureAtlasFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CTextureAtlasFactory::Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pGraphicsContext || !pResourceManager)
		{
			return RC_INVALID_ARGS;
		}

		mpResourceManager = pResourceManager;

		mpGraphicsContext = pGraphicsContext;

		mIsInitialized = true;

		return RC_OK;
	}

	IResource* CTextureAtlasFactory::Create(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateTextureAtlas(mpResourceManager, mpGraphicsContext, name, 
														   dynamic_cast<const TTexture2DParameters&>(params), 
														   result));
	}

	IResource* CTextureAtlasFactory::CreateDefault(const std::string& name, const TBaseResourceParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return dynamic_cast<IResource*>(CreateTextureAtlas(mpResourceManager, mpGraphicsContext, name, result));
	}

	TypeId CTextureAtlasFactory::GetResourceTypeId() const
	{
		return ITextureAtlas::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IResourceFactory, CTextureAtlasFactory, result, pResourceManager, pGraphicsContext);
	}
}