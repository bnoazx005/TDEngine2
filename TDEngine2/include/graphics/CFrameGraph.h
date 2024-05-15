/*!
	\file CFrameGraph.h
	\date 06.03.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/CBaseObject.h"
#include <string>
#include <vector>
#include <memory>


namespace TDEngine2
{
	class CFrameGraph;
	class CFrameGraphBuilder;
	class CFrameGraphResource;
	class ITextFileWriter;
	class IGraphicsObjectManager;
	class IGraphicsContext;


	TDE2_DECLARE_HANDLE_TYPE(TFrameGraphResourceHandle);


	struct TFramePassExecutionContext
	{
		IGraphicsContext* mpGraphicsContext = nullptr;
		CFrameGraph*      mpOwnerGraph = nullptr;
	};


	/*!
		interface IFrameGraphPass
	*/

	class IFrameGraphPass
	{
		public:
			typedef std::vector<TFrameGraphResourceHandle> TResourcesArray;
		public:
			TDE2_API virtual ~IFrameGraphPass() = default;

			TDE2_API virtual E_RESULT_CODE Setup(CFrameGraphBuilder& builder) = 0;
			TDE2_API virtual E_RESULT_CODE Execute(const TFramePassExecutionContext& executionContext) = 0;

			TDE2_API virtual bool ContainsResourceCreation(TFrameGraphResourceHandle handle) const = 0;
			TDE2_API virtual bool ContainsResourceRead(TFrameGraphResourceHandle handle) const = 0;
			TDE2_API virtual bool ContainsResourceWrite(TFrameGraphResourceHandle handle) const = 0;

			TDE2_API virtual TFrameGraphResourceHandle AddCreation(TFrameGraphResourceHandle handle) = 0;
			TDE2_API virtual TFrameGraphResourceHandle AddRead(TFrameGraphResourceHandle handle) = 0;
			TDE2_API virtual TFrameGraphResourceHandle AddWrite(TFrameGraphResourceHandle handle) = 0;

			TDE2_API virtual void SetPersistent(bool state) = 0;

			TDE2_API virtual void SetRefCount(U32 value) = 0;
			TDE2_API virtual U32 GetRefCount() const = 0;

			TDE2_API virtual const TResourcesArray& GetWrites() const = 0;
			TDE2_API virtual const TResourcesArray& GetReads() const = 0;
			TDE2_API virtual const TResourcesArray& GetCreations() const = 0;

			TDE2_API virtual bool IsActive() const = 0;
			TDE2_API virtual bool IsPersistent() const = 0;

			TDE2_API virtual const std::string& GetName() const = 0;
	};


	/*!
		class CFrameGraphPass
	*/

	template <typename TPassData, typename TSetupCallback, typename TExecuteCallback>
	class CFrameGraphPass : public IFrameGraphPass
	{
		public:
			CFrameGraphPass(const std::string& id, TSetupCallback&& setupCallback, TExecuteCallback&& executeCallback) :
				mName(id), mSetupCallback(std::forward<TSetupCallback>(setupCallback)), mExecuteCallback(std::forward<TExecuteCallback>(executeCallback))
			{
			}

			~CFrameGraphPass() override = default;

			E_RESULT_CODE Setup(CFrameGraphBuilder& builder) override
			{
				mSetupCallback(builder, mData);
				return RC_OK;
			}

			E_RESULT_CODE Execute(const TFramePassExecutionContext& executionContext) override
			{
				mExecuteCallback(mData, executionContext);
				return RC_OK;
			}

			void SetPersistent(bool state) override
			{
				mIsPersistent = state;
			}

			void SetRefCount(U32 value) { mRefCount = value; }
			U32 GetRefCount() const { return mRefCount; }

			bool ContainsResourceCreation(TFrameGraphResourceHandle handle) const override { return std::find(mCreates.cbegin(), mCreates.cend(), handle) != mCreates.cend(); }
			bool ContainsResourceRead(TFrameGraphResourceHandle handle) const override { return std::find(mReads.cbegin(), mReads.cend(), handle) != mReads.cend(); }
			bool ContainsResourceWrite(TFrameGraphResourceHandle handle) const override { return std::find(mWrites.cbegin(), mWrites.cend(), handle) != mWrites.cend(); }

			TFrameGraphResourceHandle AddCreation(TFrameGraphResourceHandle handle) override
			{
				mCreates.emplace_back(handle);
				return handle;
			}

			TFrameGraphResourceHandle AddRead(TFrameGraphResourceHandle handle) override
			{
				mReads.emplace_back(handle);
				return handle;
			}

			TFrameGraphResourceHandle AddWrite(TFrameGraphResourceHandle handle) override
			{
				mWrites.emplace_back(handle);
				return handle;
			}

			void SetName(const std::string& id) { mName = id; }
			const std::string& GetName() const { return mName; }

			bool IsActive() const override { return mRefCount > 0 || mIsPersistent; }
			bool IsPersistent() const override { return mIsPersistent;  }

			const TResourcesArray& GetWrites() const override { return mWrites; }
			const TResourcesArray& GetReads() const override { return mReads; }
			const TResourcesArray& GetCreations() const override { return mCreates; }

			const TPassData& GetData() const { return mData; }
		private:
			std::string      mName;

			TSetupCallback   mSetupCallback = nullptr;
			TExecuteCallback mExecuteCallback = nullptr;

			TPassData        mData{};

			std::vector<TFrameGraphResourceHandle> mCreates;
			std::vector<TFrameGraphResourceHandle> mReads;
			std::vector<TFrameGraphResourceHandle> mWrites;

			U32 mRefCount = 0;

			bool mIsPersistent = false;
	};


	struct TFrameGraphResourceNode
	{
		TDE2_API TFrameGraphResourceNode(const std::string& name, TFrameGraphResourceHandle id, TFrameGraphResourceHandle resourceHandle, U32 version);

		TDE2_API inline std::string ToString() const { return mName + " ver: " + std::to_string(mVersion); }

		const std::string mName;
		const TFrameGraphResourceHandle mId = TFrameGraphResourceHandle::Invalid;
		const TFrameGraphResourceHandle mResourceHandle = TFrameGraphResourceHandle::Invalid;
		const U32 mVersion = 0;
		U32 mRefCount = 0;

		IFrameGraphPass* mpProducerPass = nullptr;
		IFrameGraphPass* mpLastUserPass = nullptr;
	};


	class CFrameGraphBuilder
	{
		public:
			TDE2_API CFrameGraphBuilder(CFrameGraph* pFrameGraph, IFrameGraphPass* pPass);

			template <typename TResourceType>
			TFrameGraphResourceHandle Create(const std::string& id, const typename TResourceType::TDesc& desc);

			TDE2_API TFrameGraphResourceHandle Read(TFrameGraphResourceHandle handle);
			TDE2_API TFrameGraphResourceHandle Write(TFrameGraphResourceHandle handle);

			TDE2_API void MarkAsPersistent();
		private:
			CFrameGraph*     mpFrameGraph;
			IFrameGraphPass* mpPass;
	};


	/*!
		class CFrameGraphResource
		\brief
	*/

	class CFrameGraphResource
	{
		private:
			struct IResourceHolder
			{
				virtual ~IResourceHolder() = default;

				TDE2_API virtual E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager) = 0;
				TDE2_API virtual E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager) = 0;

				TDE2_API virtual void BeforeReadOp() = 0;
				TDE2_API virtual void BeforeWriteOp() = 0;
			};

			template <typename TResource>
			struct TResourceHolder : public IResourceHolder
			{
				TResourceHolder(const typename TResource::TDesc& desc, TResource&& resource) : mDesc(desc), mResource(std::forward<TResource>(resource)) {}

				E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager) override
				{
					return mResource.Acquire(pGraphicsObjectManager, mDesc);
				}

				E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager) override
				{
					return mResource.Release(pGraphicsObjectManager);
				}

				void BeforeReadOp() override
				{
					mResource.BeforeReadOp();
				}

				void BeforeWriteOp() override
				{
					mResource.BeforeWriteOp();
				}

				typename TResource::TDesc mDesc;
				TResource mResource;
			};
		public:
			template <typename TResourceType>
			CFrameGraphResource(const std::string& name, TFrameGraphResourceHandle handle, const typename TResourceType::TDesc& desc, TResourceType&& resource,
				IFrameGraphPass* pPass = nullptr, U32 version = mInitialVersion) :
				mName(name), mId(handle), mVersion(version), mpCreator(pPass), 
				mpResourceHolder(std::make_unique<TResourceHolder<TResourceType>>(desc, std::forward<TResourceType>(resource)))
			{
			}

			TDE2_API E_RESULT_CODE Acquire(IGraphicsObjectManager* pGraphicsObjectManager);
			TDE2_API E_RESULT_CODE Release(IGraphicsObjectManager* pGraphicsObjectManager);

			TDE2_API void BeforeReadOp();
			TDE2_API void BeforeWriteOp();

			TDE2_API void SetProducerPass(IFrameGraphPass* pPass);
			TDE2_API void SetLastUserPass(IFrameGraphPass* pPass);

			TDE2_API IFrameGraphPass* GetProducerPass() const;
			TDE2_API IFrameGraphPass* GetLastUserPass() const;

			TDE2_API void SetRefCount(U32 value);
			TDE2_API U32 GetRefCount() const;

			TDE2_API void SetVersion(U32 value);
			TDE2_API U32 GetVersion() const;

			template <typename T> T& Get() { return dynamic_cast<TResourceHolder<T>*>(mpResourceHolder.get())->mResource; }

			TDE2_API bool IsTransient() const;

			TDE2_API const std::string& GetName() const;
			TDE2_API TFrameGraphResourceHandle GetHandle() const;

		public:
			TDE2_STATIC_CONSTEXPR U32 mInitialVersion = 1;
		private:
			std::string mName;

			TFrameGraphResourceHandle mId = TFrameGraphResourceHandle::Invalid;
			U32 mVersion = mInitialVersion;
			U32 mRefCount = 0;

			IFrameGraphPass* mpCreator = nullptr;
			IFrameGraphPass* mpProducerPass = nullptr;
			IFrameGraphPass* mpLastUserPass = nullptr;

			std::unique_ptr<IResourceHolder> mpResourceHolder;
	};


	TDE2_API CFrameGraph* CreateFrameGraph(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		\brief The class represents a graph structure that contains render passes and graphics resources that form a frame
	*/

	class CFrameGraph: public CBaseObject
	{
		public:
			friend TDE2_API CFrameGraph* CreateFrameGraph(IGraphicsContext*, E_RESULT_CODE&);
		public:
			typedef std::vector<CFrameGraphResource> TResourcesRegistry;
		public:
			TDE2_API E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext);

			template <typename TPassData, typename TSetupCallback, typename TExecuteCallback>
			const TPassData& AddPass(const std::string& name, TSetupCallback&& setupCallback, TExecuteCallback&& executeCallback)
			{
				mpActivePasses.emplace_back(std::make_unique<CFrameGraphPass<TPassData, TSetupCallback, TExecuteCallback>>(
					name, 
					std::forward<TSetupCallback>(setupCallback), 
					std::forward<TExecuteCallback>(executeCallback)));

				auto pCurrPass = dynamic_cast<CFrameGraphPass<TPassData, TSetupCallback, TExecuteCallback>*>(mpActivePasses.back().get());

				CFrameGraphBuilder builder(this, pCurrPass);
				pCurrPass->Setup(builder);

				return pCurrPass->GetData();
			}

			template <typename TResourceType>
			TFrameGraphResourceHandle ImportResource(const std::string& name, const typename TResourceType::TDesc& desc, TResourceType&& resource)
			{
				const TFrameGraphResourceHandle resourceHandle = static_cast<TFrameGraphResourceHandle>(mResources.size());
				const TFrameGraphResourceHandle resourceNodeHandle = static_cast<TFrameGraphResourceHandle>(mResourcesGraph.size());

				mResources.emplace_back(CFrameGraphResource{ name, resourceHandle, desc, std::forward<TResourceType>(resource), nullptr });
				mResourcesGraph.emplace_back(TFrameGraphResourceNode{ name, resourceNodeHandle, resourceHandle, CFrameGraphResource::mInitialVersion });

				return resourceNodeHandle;
			}

			TDE2_API void Reset();

			TDE2_API E_RESULT_CODE Compile();
			TDE2_API E_RESULT_CODE Execute();

#if TDE2_EDITORS_ENABLED
			TDE2_API E_RESULT_CODE Dump(ITextFileWriter* pFileWriter);
#endif

			template <typename TResourceType>
			TResourceType& GetResource(TFrameGraphResourceHandle handle)
			{
				return _getResource(handle).Get<TResourceType>();
			}

			TDE2_API TResourcesRegistry& GetResources(const CPassKey<CFrameGraphBuilder>& passkey);
			TDE2_API std::vector<TFrameGraphResourceNode>& GetResourcesGraph(const CPassKey<CFrameGraphBuilder>& passkey);

		private:
			CFrameGraphResource& _getResource(TFrameGraphResourceHandle handle)
			{
				TDE2_ASSERT(static_cast<USIZE>(handle) < mResourcesGraph.size());
				TDE2_ASSERT(static_cast<USIZE>(mResourcesGraph[static_cast<USIZE>(handle)].mResourceHandle) < mResources.size());

				return mResources[static_cast<USIZE>(mResourcesGraph[static_cast<USIZE>(handle)].mResourceHandle)];
			}

		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFrameGraph)
		private:
			std::vector<std::unique_ptr<IFrameGraphPass>> mpActivePasses;
			TResourcesRegistry                            mResources;
			std::vector<TFrameGraphResourceNode>          mResourcesGraph;
			
			IGraphicsContext*                             mpGraphicsContext = nullptr;
			IGraphicsObjectManager*                       mpGraphicsObjectManager = nullptr;

			TFramePassExecutionContext                    mExecutionContext;
	};


	TDE2_DECLARE_SCOPED_PTR_INLINED(CFrameGraph)


	template <typename TResourceType>
	TFrameGraphResourceHandle CFrameGraphBuilder::Create(const std::string& id, const typename TResourceType::TDesc& desc)
	{
		auto& resources = mpFrameGraph->GetResources({});
		auto& resourcesGraph = mpFrameGraph->GetResourcesGraph({});

		const TFrameGraphResourceHandle handle = static_cast<TFrameGraphResourceHandle>(resources.size());
		const TFrameGraphResourceHandle resourceNodeHandle = static_cast<TFrameGraphResourceHandle>(resourcesGraph.size());

		resources.emplace_back(CFrameGraphResource{ id, handle, desc, TResourceType{}, mpPass });
		resourcesGraph.emplace_back(TFrameGraphResourceNode{ id, resourceNodeHandle, handle, CFrameGraphResource::mInitialVersion });

		mpPass->AddCreation(resourceNodeHandle);

		return resourceNodeHandle;
	}
}