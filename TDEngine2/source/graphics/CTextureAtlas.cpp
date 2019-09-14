#include "./../../include/graphics/CTextureAtlas.h"
#include "./../../include/core/IGraphicsContext.h"
#include "./../../include/core/IResourceManager.h"
#include "./../../include/core/IFileSystem.h"
#include "./../../include/utils/Utils.h"
#include "./../../include/graphics/CBaseTexture2D.h"
#include <cassert>
#include <algorithm>
#include <stack>
#include <cmath>


namespace TDEngine2
{
	CTextureAtlas::CTextureAtlas() :
		CBaseResource(), mpTextureResource(nullptr)
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
		if (!(mpTextureResource = pResourceManager->Create<CBaseTexture2D>(name + "_Tex", params))->IsValid())
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CTextureAtlas::Load()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		const IResourceLoader* pResourceLoader = mpResourceManager->GetResourceLoader<CTextureAtlas>();

		if (!pResourceLoader)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = pResourceLoader->LoadResource(this);

		if (result != RC_OK)
		{
			mState = RST_PENDING;

			return result;
		}

		mState = RST_LOADED;

		return result;
	}

	E_RESULT_CODE CTextureAtlas::Unload()
	{
		return Reset();
	}

	E_RESULT_CODE CTextureAtlas::Reset()
	{
		TDE2_UNIMPLEMENTED();
		return RC_NOT_IMPLEMENTED_YET;
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
		TRectI32 textureRect { 0, 0, width, height };

		mPendingData.push_back({ name, textureRect, true, false, { { pData, format } } });

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
		
		ITexture2D* pAtlasInternalTexture = dynamic_cast<ITexture2D*>(mpTextureResource->Get(RAT_BLOCKING));

		/// \note while there is enough space within the atlas pack next entry
		TAtlasAreaEntry root { {0, 0, pAtlasInternalTexture->GetWidth(), pAtlasInternalTexture->GetHeight()} };
		
		std::stack<TAtlasAreaEntry*> areasToCheck;		

		TAtlasAreaEntry* pCurrSubdivisionEntry = nullptr;
		
		auto dataIter = mPendingData.cbegin();

		TRectI32 firstRect, secondRect, thirdRect;

		bool hasInsertionFailed = false;

		auto calcMetric = [](const TRectI32& textureSizes, const TRectI32& areaSizes) -> F32
		{
			F32 dx = fabs(areaSizes.width - textureSizes.width);
			F32 dy = fabs(areaSizes.height - textureSizes.height);

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

					float dx = pCurrSubdivisionEntry->mBounds.width - dataIter->mRect.width;
					float dy = pCurrSubdivisionEntry->mBounds.height - dataIter->mRect.height;

					bool isVerticalSlice = (dx > dy) || fabs(dx - dy) < 1e-3f;

					/// \note divide the area into sub areas based on filled space
					std::tie(firstRect, secondRect) = SplitRectWithLine(pCurrSubdivisionEntry->mBounds,
																		isVerticalSlice ? TVector2(dataIter->mRect.width, 0.0f) : TVector2(0.0f, dataIter->mRect.height),
																		isVerticalSlice);
					
					std::tie(thirdRect, firstRect) = SplitRectWithLine(firstRect,
																	   !isVerticalSlice ? TVector2(dataIter->mRect.width, 0.0f) : TVector2(0.0f, dataIter->mRect.height),
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

		while (!areasToCheck.empty())
		{
			pCurrSubdivisionEntry = areasToCheck.top();
			areasToCheck.pop();

			if (!pCurrSubdivisionEntry || pCurrSubdivisionEntry->mTextureEntryId == (std::numeric_limits<U32>::max)())
			{
				continue;
			}

			pCurrTextureEntry = &mPendingData[pCurrSubdivisionEntry->mTextureEntryId];

			if (!pCurrTextureEntry->mIsRawData)
			{
				/// \note for now we support only raw textures
				/// \todo implement support of CBaseTexture2D based textures
				TDE2_UNIMPLEMENTED();
			}

			assert(pAtlasInternalTexture->WriteData({ pCurrSubdivisionEntry->mBounds.x, 
													  pCurrSubdivisionEntry->mBounds.y, 
													  pCurrTextureEntry->mRect.width,
													  pCurrTextureEntry->mRect.height }, pCurrTextureEntry->mData.mRawTexture.mpData) == RC_OK);

			areasToCheck.push(pCurrSubdivisionEntry->mpLeft.get());
			areasToCheck.push(pCurrSubdivisionEntry->mpRight.get());
		}		

		return hasInsertionFailed ? RC_FAIL : RC_OK;
	}
	
	void CTextureAtlas::Bind(U32 slot)
	{
		TDE2_UNIMPLEMENTED();
	}

	U32 CTextureAtlas::GetWidth() const
	{
		TDE2_UNIMPLEMENTED();
		return 0x0;
	}

	U32 CTextureAtlas::GetHeight() const
	{
		TDE2_UNIMPLEMENTED();
		return 0x0;
	}

	ITexture2D* CTextureAtlas::GetTexture() const
	{
		return dynamic_cast<ITexture2D*>(mpTextureResource->Get(RAT_BLOCKING));
	}


	TDE2_API ITextureAtlas* CreateTextureAtlas(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
												const TTexture2DParameters& params, E_RESULT_CODE& result)
	{
		CTextureAtlas* pTextureAtlasInstance = new (std::nothrow) CTextureAtlas();

		if (!pTextureAtlasInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pTextureAtlasInstance->Init(pResourceManager, pGraphicsContext, name, params);

		if (result != RC_OK)
		{
			delete pTextureAtlasInstance;

			pTextureAtlasInstance = nullptr;
		}

		return pTextureAtlasInstance;
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
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		TDE2_UNIMPLEMENTED();
		E_RESULT_CODE result = RC_OK;
/*
		TResult<TFileEntryId> materialFileId = mpFileSystem->Open<CBinaryFileReader>(pResource->GetName());

		if (materialFileId.HasError())
		{
			return materialFileId.GetError();
		}

		IBinaryFileReader* pMaterialFile = dynamic_cast<IBinaryFileReader*>(mpFileSystem->Get<CBinaryFileReader>(materialFileId.Get()));

		/// try to read the file's header
		TTextureAtlasFileHeader header = _readMaterialFileHeader(pMaterialFile).Get();

		if ((result = pMaterialFile->Close()) != RC_OK)
		{
			return result;
		}*/

		return RC_OK;
	}

	U32 CTextureAtlasLoader::GetResourceTypeId() const
	{
		return CTextureAtlas::GetTypeId();
	}


	TDE2_API IResourceLoader* CreateTextureAtlasLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem, E_RESULT_CODE& result)
	{
		CTextureAtlasLoader* pMaterialLoaderInstance = new (std::nothrow) CTextureAtlasLoader();

		if (!pMaterialLoaderInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialLoaderInstance->Init(pResourceManager, pGraphicsContext, pFileSystem);

		if (result != RC_OK)
		{
			delete pMaterialLoaderInstance;

			pMaterialLoaderInstance = nullptr;
		}

		return pMaterialLoaderInstance;
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

		TDE2_UNIMPLEMENTED();

		return nullptr;
	}

	U32 CTextureAtlasFactory::GetResourceTypeId() const
	{
		return CTextureAtlas::GetTypeId();
	}


	TDE2_API IResourceFactory* CreateTextureAtlasFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		CTextureAtlasFactory* pMaterialFactoryInstance = new (std::nothrow) CTextureAtlasFactory();

		if (!pMaterialFactoryInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pMaterialFactoryInstance->Init(pResourceManager, pGraphicsContext);

		if (result != RC_OK)
		{
			delete pMaterialFactoryInstance;

			pMaterialFactoryInstance = nullptr;
		}

		return pMaterialFactoryInstance;
	}
}