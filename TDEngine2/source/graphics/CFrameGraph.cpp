#include "../../include/graphics/CFrameGraph.h"
#include <stack>


namespace TDEngine2
{
	/*!
		\brief CFrameGraphBuilder's definition
	*/

	CFrameGraphBuilder::CFrameGraphBuilder(CFrameGraph* pFrameGraph, IFrameGraphPass* pPass) :
		mpFrameGraph(pFrameGraph), mpPass(pPass)
	{
	}

	TFrameGraphResourceHandle CFrameGraphBuilder::Read(TFrameGraphResourceHandle handle)
	{
		return mpPass->ContainsResourceRead(handle) ? handle : mpPass->AddRead(handle);
	}

	TFrameGraphResourceHandle CFrameGraphBuilder::Write(TFrameGraphResourceHandle handle)
	{
		if (mpPass->ContainsResourceCreation(handle))
		{
			return mpPass->ContainsResourceWrite(handle) ? handle : mpPass->AddWrite(handle);
		}

		Read(handle);
		return mpPass->AddWrite(handle); // \todo Update version of handle for the resource
	}

	void CFrameGraphBuilder::MarkAsPersistent()
	{
		mpPass->SetPersistent(true);
	}


	/*!
		\brief CFrameGraphResource's definition
	*/

	void CFrameGraphResource::Acquire()
	{
		mpResourceHolder->Acquire();
	}

	void CFrameGraphResource::Release()
	{
		mpResourceHolder->Release();
	}

	void CFrameGraphResource::BeforeReadOp()
	{
		mpResourceHolder->BeforeReadOp();
	}

	void CFrameGraphResource::BeforeWriteOp()
	{
		mpResourceHolder->BeforeWriteOp();
	}

	void CFrameGraphResource::SetProducerPass(IFrameGraphPass* pPass)
	{
		mpProducerPass = pPass;
	}

	void CFrameGraphResource::SetLastUserPass(IFrameGraphPass* pPass)
	{
		mpLastUserPass = pPass;
	}

	IFrameGraphPass* CFrameGraphResource::GetProducerPass() const
	{
		return mpProducerPass;
	}

	IFrameGraphPass* CFrameGraphResource::GetLastUserPass() const
	{
		return mpLastUserPass;
	}

	void CFrameGraphResource::SetRefCount(U32 value)
	{
		mRefCount = value;
	}

	U32 CFrameGraphResource::GetRefCount() const
	{
		return mRefCount;
	}

	bool CFrameGraphResource::IsTransient() const
	{
		return mpCreator != nullptr;
	}


	/*!
		\brief CFrameGraph's definition
	*/


	CFrameGraph::CFrameGraph() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CFrameGraph::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	void CFrameGraph::Reset()
	{
		mpActivePasses.clear();
		mResources.clear();
	}

	E_RESULT_CODE CFrameGraph::Compile()
	{
		for (auto&& pCurrPass : mpActivePasses)
		{
			pCurrPass->SetRefCount(static_cast<U32>(pCurrPass->GetWrites().size()));

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
				resource.SetRefCount(resource.GetRefCount() + 1);
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
				resource.SetProducerPass(pCurrPass.get());
			}
		}

		// \note Cull using flood-fill algorithm
		{
			std::stack<USIZE> unreferencedResourcesHandles;

			for (USIZE i = 0; i < mResources.size(); i++)
			{
				if (mResources[i].GetRefCount() > 0)
				{
					continue;
				}

				unreferencedResourcesHandles.push(i);
			}

			while (!unreferencedResourcesHandles.empty())
			{
				CFrameGraphResource& currResource = mResources[unreferencedResourcesHandles.top()];
				unreferencedResourcesHandles.pop();

				IFrameGraphPass* pProducerPass = currResource.GetProducerPass();
				if (!pProducerPass)
				{
					continue;
				}

				pProducerPass->SetRefCount(pProducerPass->GetRefCount() - 1);
				if (pProducerPass->IsActive())
				{
					continue;
				}

				for (const TFrameGraphResourceHandle currResourceHandle : pProducerPass->GetReads())
				{
					auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
					resource.SetRefCount(resource.GetRefCount() - 1);

					unreferencedResourcesHandles.push(static_cast<USIZE>(currResourceHandle));
				}
			}
		}

		// compute lifetime of resources
		for (auto&& pCurrPass : mpActivePasses)
		{
			if (!pCurrPass->IsActive())
			{
				continue;
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetCreations())
			{
				auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
				resource.SetProducerPass(pCurrPass.get());
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
				resource.SetLastUserPass(pCurrPass.get());
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				auto& resource = mResources[static_cast<USIZE>(currResourceHandle)];
				resource.SetLastUserPass(pCurrPass.get());
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CFrameGraph::Execute()
	{
		for (auto&& pCurrPass : mpActivePasses)
		{
			if (!pCurrPass->IsActive())
				continue;

			// \note de-virtualize resources that is used by the pass
			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetCreations())
			{
				mResources[static_cast<USIZE>(currResourceHandle)].Acquire();
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				mResources[static_cast<USIZE>(currResourceHandle)].BeforeReadOp();
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				mResources[static_cast<USIZE>(currResourceHandle)].BeforeWriteOp();
			}
			
			// \note invoke execute
			pCurrPass->Execute();

			// \note destroy resources
			for (auto& currResource: mResources)
			{
				if (currResource.GetLastUserPass() == pCurrPass.get() && currResource.IsTransient())
				{
					currResource.Release();
				}
			}
		}

		return RC_OK;
	}

	CFrameGraph::TResourcesRegistry& CFrameGraph::GetResources(const CPassKey<CFrameGraphBuilder>& passkey)
	{
		return mResources;
	}


	TDE2_API CFrameGraph* CreateFrameGraph(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CFrameGraph, CFrameGraph, result);
	}
}