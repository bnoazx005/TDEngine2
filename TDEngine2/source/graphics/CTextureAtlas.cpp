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
#include <cassert>
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
		mData.mRawTexture = texture;
	}
	
	CTextureAtlas::TTextureAtlasEntry::TTextureAtlasEntry(const std::string& name, const TRectI32& rect, ITexture2D* pTexture) :
		mName(name),
		mRect(rect),
		mIsRawData(false)
	{
		mData.mpTexture = pTexture;
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

		if (!pGraphicsContext)
		{
			return RC_INVALID_ARGS;
		}

		mpGraphicsContext = pGraphicsContext;
		
		/// \note create a texture using the resource manager
		if (TResourceId::Invalid == (mTextureResourceHandle = pResourceManager->Create<ITexture2D>(name + "_Tex", params)))
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Reset()
	{
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
		if (std::find_if(mPendingData.cbegin(), mPendingData.cend(), [&name](const TTextureAtlasEntry& entry)
		{
			return entry.mName == name;
		}) != mPendingData.cend())
		{
			return RC_FAIL;
		}

		/// \note add a new entry
		TRectI32 textureRect { 0, 0, static_cast<I32>(width), static_cast<I32>(height) };

		TTextureAtlasEntry rootEntry { name, textureRect, { pData, format } };

		mPendingData.push_back(rootEntry);

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::AddTexture(TResourceId textureHandle)
	{
		if (TResourceId::Invalid == textureHandle)
		{
			return RC_INVALID_ARGS;
		}

		ITexture2D* pTextureResource = mpResourceManager->GetResource<ITexture2D>(textureHandle);
		if (!pTextureResource)
		{
			return RC_FAIL;
		}

		const std::string textureName = dynamic_cast<IResource*>(pTextureResource)->GetName();

		/// \note check whether the texture's data is already within the atlas or not
		if (std::find_if(mPendingData.cbegin(), mPendingData.cend(), [&textureName](const TTextureAtlasEntry& entry) { return entry.mName == textureName; }) != mPendingData.cend() ||
		    mAtlasEntities.find(textureName) != mAtlasEntities.cend())
		{
			return RC_FAIL;
		}

		/// \note add a new entry
		TRectI32 textureRect{ 0, 0, static_cast<I32>(pTextureResource->GetWidth()), static_cast<I32>(pTextureResource->GetHeight()) };

		TTextureAtlasEntry rootEntry{ textureName, textureRect, pTextureResource };

		mPendingData.push_back(rootEntry);

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Bake()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		/// \note sort all entries by their sizes
		std::sort(mPendingData.begin(), mPendingData.end(), [](const TTextureAtlasEntry& left, const TTextureAtlasEntry& right)
		{
			TRectI32 leftRect  = left.mRect;
			TRectI32 rightRect = right.mRect;

			return (leftRect.width > rightRect.width) && (leftRect.height > rightRect.height);
		});
		
		ITexture2D* pAtlasInternalTexture = mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle);

		/// \note while there is enough space within the atlas pack next entry
		TAtlasAreaEntry root;
		root.mBounds = {0, 0, static_cast<I32>(pAtlasInternalTexture->GetWidth()), static_cast<I32>(pAtlasInternalTexture->GetHeight())};
		
		std::stack<TAtlasAreaEntry*> areasToCheck;		

		TAtlasAreaEntry* pCurrSubdivisionEntry = nullptr;
		
		auto dataIter = mPendingData.cbegin();

		TRectI32 firstRect, secondRect, thirdRect;

		bool hasInsertionFailed = false;

		auto calcMetric = [](const TRectI32& textureSizes, const TRectI32& areaSizes) -> F32
		{
			F32 dx = fabs(static_cast<F32>(areaSizes.width - textureSizes.width));
			F32 dy = fabs(static_cast<F32>(areaSizes.height - textureSizes.height));

			return sqrt(dx * dx + dy * dy);
		};

		while (dataIter != mPendingData.cend())
		{
			areasToCheck.push(&root);

			while (!areasToCheck.empty())
			{
				hasInsertionFailed = false;

				pCurrSubdivisionEntry = areasToCheck.top();
				areasToCheck.pop();

				/// \note traverse down to leaves, 'cause the rect is already filled up
				if (pCurrSubdivisionEntry->mTextureEntryId < (std::numeric_limits<U32>::max)())
				{
					auto pLeftChild  = pCurrSubdivisionEntry->mpLeft.get();
					auto pRightChild = pCurrSubdivisionEntry->mpRight.get();

					/// \note choose the branch based on minimal error between its sizes and sizes of an inserted texture
					F32 leftChildMetric  = calcMetric(dataIter->mRect, pLeftChild->mBounds);
					F32 rightChildMetric = calcMetric(dataIter->mRect, pRightChild->mBounds);

					areasToCheck.push(leftChildMetric < rightChildMetric ? pRightChild : pLeftChild);
					areasToCheck.push(leftChildMetric < rightChildMetric ? pLeftChild : pRightChild);

					continue;
				}

				/// \note if current texture fits into the area, fill it up
				if (pCurrSubdivisionEntry->mBounds.width >= dataIter->mRect.width &&
					pCurrSubdivisionEntry->mBounds.height >= dataIter->mRect.height)
				{
					pCurrSubdivisionEntry->mTextureEntryId = std::distance(mPendingData.cbegin(), dataIter);

					F32 dx = static_cast<F32>(pCurrSubdivisionEntry->mBounds.width - dataIter->mRect.width);
					F32 dy = static_cast<F32>(pCurrSubdivisionEntry->mBounds.height - dataIter->mRect.height);

					bool isVerticalSlice = (dx > dy) || fabs(dx - dy) < 1e-3f;

					/// \note divide the area into sub areas based on filled space
					std::tie(firstRect, secondRect) = SplitRectWithLine(pCurrSubdivisionEntry->mBounds,
																		isVerticalSlice ? TVector2(static_cast<F32>(dataIter->mRect.width), 0.0f) : TVector2(0.0f, static_cast<F32>(dataIter->mRect.height)),
																		isVerticalSlice);
					
					std::tie(thirdRect, firstRect) = SplitRectWithLine(firstRect,
																	   !isVerticalSlice ? TVector2(static_cast<F32>(dataIter->mRect.width), 0.0f) : TVector2(0.0f, static_cast<F32>(dataIter->mRect.height)),
																	   !isVerticalSlice);

					pCurrSubdivisionEntry->mpLeft  = std::make_unique<TAtlasAreaEntry>();
					pCurrSubdivisionEntry->mpLeft->mBounds = firstRect;

					pCurrSubdivisionEntry->mpRight = std::make_unique<TAtlasAreaEntry>();
					pCurrSubdivisionEntry->mpRight->mBounds = secondRect;

					++dataIter;

					break;
				}

				hasInsertionFailed = true;
			}

			/// \note there is no enough free space in texture atlas anymore
			if (areasToCheck.empty() && hasInsertionFailed)
			{
				break;
			}

			/// \note clean up the stack
			while (!areasToCheck.empty())
			{
				areasToCheck.pop();
			}
		}

		/// \note write down all data into atlas's texture
		areasToCheck.push(&root);

		TTextureAtlasEntry* pCurrTextureEntry = nullptr;

		TRectI32 textureRect;

		while (!areasToCheck.empty())
		{
			pCurrSubdivisionEntry = areasToCheck.top();
			areasToCheck.pop();

			if (!pCurrSubdivisionEntry || pCurrSubdivisionEntry->mTextureEntryId == (std::numeric_limits<U32>::max)())
			{
				continue;
			}

			pCurrTextureEntry = &mPendingData[pCurrSubdivisionEntry->mTextureEntryId];

			textureRect = { pCurrSubdivisionEntry->mBounds.x,
							pCurrSubdivisionEntry->mBounds.y,
							pCurrTextureEntry->mRect.width,
							pCurrTextureEntry->mRect.height };

			mAtlasEntities[pCurrTextureEntry->mName] = textureRect;

			if (!pCurrTextureEntry->mIsRawData)
			{
				auto&& textureData = pCurrTextureEntry->mData.mpTexture->GetInternalData();

				E_RESULT_CODE result = pAtlasInternalTexture->WriteData(textureRect, &textureData.front());
				TDE2_ASSERT(result == RC_OK);
			}
			else
			{
				E_RESULT_CODE result = pAtlasInternalTexture->WriteData(textureRect, pCurrTextureEntry->mData.mRawTexture.mpData);
				TDE2_ASSERT(result == RC_OK);
			}

			areasToCheck.push(pCurrSubdivisionEntry->mpLeft.get());
			areasToCheck.push(pCurrSubdivisionEntry->mpRight.get());
		}		

		return hasInsertionFailed ? RC_FAIL : RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		ITexture2D* pAtlasInternalTexture = mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle);

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
				CDeferOperation _([pArchiveFile] { pArchiveFile->Close(); });
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
		mTextureResourceHandle = mpResourceManager->Load<ITexture2D>(mName + "_Tex.png");

		/// \todo ansynchronously update sizes of the atlas when the texture has been loaded
		_updateAtlasSizes(mpResourceManager->GetResource(mTextureResourceHandle));

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
		return mpResourceManager->GetResource<ITexture2D>(mTextureResourceHandle);
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

		mpSubTextures.push_back(mpResourceManager->GetResource<IAtlasSubTexture>(subTextureHandle));

		return Wrench::TOkValue<TResourceId>(subTextureHandle);
	}

	const IResourceLoader* CTextureAtlas::_getResourceLoader()
	{
		return mpResourceManager->GetResourceLoader<ITextureAtlas>();
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
		mIsInitialized(false)
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

	E_RESULT_CODE CTextureAtlasLoader::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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
		mIsInitialized(false)
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

	E_RESULT_CODE CTextureAtlasFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

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