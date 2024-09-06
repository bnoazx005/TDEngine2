#include "../../include/graphics/CFrameGraph.h"
#include "../../include/core/IGraphicsContext.h"
#include "../../include/core/IFile.h"
#include "../../include/editor/CPerfProfiler.h"
#include <stack>
#include "stringUtils.hpp"


namespace TDEngine2
{
	TFrameGraphResourceNode::TFrameGraphResourceNode(const std::string& name, TFrameGraphResourceHandle id, TFrameGraphResourceHandle resourceHandle, U32 version):
		mName(name), mId(id), mRefCount(0), mResourceHandle(resourceHandle), mVersion(version)
	{
	}


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

		// increment version of the resource on each write to it
		auto& resources = mpFrameGraph->GetResources({});
		auto& resourcesGraph = mpFrameGraph->GetResourcesGraph({});

		auto&& nodeIt = std::find_if(resourcesGraph.cbegin(), resourcesGraph.cend(), [handle](const TFrameGraphResourceNode& node) { return node.mId == handle; });
		auto&& resourceIt = std::find_if(resources.begin(), resources.end(), [&nodeIt](const CFrameGraphResource& resource) { return resource.GetHandle() == nodeIt->mResourceHandle; });

		TDE2_ASSERT(nodeIt != resourcesGraph.cend());
		TDE2_ASSERT(resourceIt != resources.end());

		resourceIt->SetVersion(resourceIt->GetVersion() + 1);

		const TFrameGraphResourceHandle resourceNodeHandle = static_cast<TFrameGraphResourceHandle>(resourcesGraph.size());
		resourcesGraph.emplace_back(TFrameGraphResourceNode{ nodeIt->mName, resourceNodeHandle, nodeIt->mResourceHandle, resourceIt->GetVersion() });

		return mpPass->AddWrite(resourceNodeHandle);
	}

	void CFrameGraphBuilder::MarkAsPersistent()
	{
		mpPass->SetPersistent(true);
	}


	/*!
		\brief CFrameGraphResource's definition
	*/

	E_RESULT_CODE CFrameGraphResource::Acquire(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		return mpResourceHolder->Acquire(pGraphicsObjectManager);
	}

	E_RESULT_CODE CFrameGraphResource::Release(IGraphicsObjectManager* pGraphicsObjectManager)
	{
		return mpResourceHolder->Release(pGraphicsObjectManager);
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

	void CFrameGraphResource::SetVersion(U32 value)
	{
		mVersion = value;
	}

	U32 CFrameGraphResource::GetVersion() const
	{
		return mVersion;
	}

	bool CFrameGraphResource::IsTransient() const
	{
		return mpCreator != nullptr;
	}

	const std::string& CFrameGraphResource::GetName() const
	{
		return mName;
	}

	TFrameGraphResourceHandle CFrameGraphResource::GetHandle() const
	{
		return mId;
	}


	/*!
		\brief CFrameGraph's definition
	*/


	CFrameGraph::CFrameGraph() :
		CBaseObject(), mpGraphicsObjectManager(nullptr)
	{
	}

	E_RESULT_CODE CFrameGraph::Init(IGraphicsContext* pGraphicsContext)
	{
		TDE2_PROFILER_SCOPE("CFrameGraph::Init");

		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpGraphicsContext = pGraphicsContext;
		mpGraphicsObjectManager = pGraphicsContext ? pGraphicsContext->GetGraphicsObjectManager() : nullptr;

		mExecutionContext = { mpGraphicsContext, this };

		mIsInitialized = true;

		return RC_OK;
	}

	void CFrameGraph::Reset()
	{
		mpActivePasses.clear();
		mResources.clear();
		mResourcesGraph.clear();
	}

	E_RESULT_CODE CFrameGraph::Compile()
	{
		TDE2_PROFILER_SCOPE("CFrameGraph::Compile");

		for (auto&& pCurrPass : mpActivePasses)
		{
			pCurrPass->SetRefCount(static_cast<U32>(pCurrPass->GetWrites().size()));

			for (const TFrameGraphResourceHandle currResourceNodeHandle : pCurrPass->GetReads())
			{
				auto& resourceNode = mResourcesGraph[static_cast<USIZE>(currResourceNodeHandle)];
				++resourceNode.mRefCount;
			}

			for (const TFrameGraphResourceHandle currResourceNodeHandle : pCurrPass->GetWrites())
			{
				auto& resourceNode = mResourcesGraph[static_cast<USIZE>(currResourceNodeHandle)];
				++resourceNode.mRefCount;
				resourceNode.mpProducerPass = pCurrPass.get();
			}
		}

		// \note Cull using flood-fill algorithm
		{
			std::stack<USIZE> unreferencedResourcesHandles;

			for (USIZE i = 0; i < mResourcesGraph.size(); i++)
			{
				if (mResourcesGraph[i].mRefCount)
				{
					continue;
				}

				unreferencedResourcesHandles.push(i);
			}

			while (!unreferencedResourcesHandles.empty())
			{
				TFrameGraphResourceNode& currResourceNode = mResourcesGraph[unreferencedResourcesHandles.top()];
				unreferencedResourcesHandles.pop();

				IFrameGraphPass* pProducerPass = currResourceNode.mpProducerPass;
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
					auto& resourceNode = mResourcesGraph[static_cast<USIZE>(currResourceHandle)];
					--resourceNode.mRefCount;

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
				auto& resource = _getResource(currResourceHandle);
				resource.SetProducerPass(pCurrPass.get());
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				auto& resource = _getResource(currResourceHandle);
				resource.SetLastUserPass(pCurrPass.get());
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				auto& resource = _getResource(currResourceHandle);
				resource.SetLastUserPass(pCurrPass.get());
			}
		}

		return RC_OK;
	}

	E_RESULT_CODE CFrameGraph::Execute()
	{
		TDE2_PROFILER_SCOPE("CFrameGraph::Execute");

		for (auto&& pCurrPass : mpActivePasses)
		{
			if (!pCurrPass->IsActive())
				continue;

			// \note de-virtualize resources that is used by the pass
			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetCreations())
			{
				_getResource(currResourceHandle).Acquire(mpGraphicsObjectManager);
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				_getResource(currResourceHandle).BeforeReadOp();
			}

			for (const TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				_getResource(currResourceHandle).BeforeWriteOp();
			}
			
			// \note invoke execute
			pCurrPass->Execute(mExecutionContext);

			// \note destroy resources
			for (auto& currResource: mResources)
			{
				if (currResource.GetLastUserPass() == pCurrPass.get() && currResource.IsTransient())
				{
					currResource.Release(mpGraphicsObjectManager);
				}
			}
		}

		return RC_OK;
	}

#if TDE2_EDITORS_ENABLED

	E_RESULT_CODE CFrameGraph::Dump(ITextFileWriter* pFileWriter)
	{
		if (!pFileWriter)
		{
			return RC_INVALID_FILE;
		}

		// \note Writes file in DOT format
		pFileWriter->WriteLine("digraph FrameGraph {");
		pFileWriter->WriteLine("rankdir = LR\nsplines=spline\nnode [shape=rectangle, fontname=\"helvetica\", fontsize=12]\n");

		// \note Write passes nodes
		for (auto&& pCurrPass : mpActivePasses)
		{
			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" [label=\"{0}\\nRefs:{1}\", style=\"rounded, filled\", fillcolor=darkorange]", pCurrPass->GetName(), pCurrPass->GetRefCount()));
		}

		pFileWriter->WriteLine(Wrench::StringUtils::GetEmptyStr());

		// \note Write resources nodes
		for (auto&& currResource : mResourcesGraph)
		{
			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" [label=\"{0}\\nRefs:{1}\\nHandle:{2}\", style=filled, fillcolor={3}]", 
				currResource.ToString(),
				currResource.mRefCount, 
				static_cast<U32>(currResource.mResourceHandle), 
				_getResource(currResource.mResourceHandle).IsTransient() ? "skyblue" : "steelblue"));
		}

		pFileWriter->WriteLine(Wrench::StringUtils::GetEmptyStr());

		// \note Create edges between passes and resources
		for (auto&& pCurrPass : mpActivePasses)
		{
			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" -> {", pCurrPass->GetName()));				

			for (TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetCreations())
			{
				pFileWriter->WriteLine("\"" + mResourcesGraph[static_cast<USIZE>(currResourceHandle)].ToString() + "\" ");
			}

			pFileWriter->WriteLine("} [color = seagreen]");

			pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" -> {", pCurrPass->GetName()));

			for (TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetWrites())
			{
				pFileWriter->WriteLine("\"" + mResourcesGraph[static_cast<USIZE>(currResourceHandle)].ToString() + "\" ");
			}

			pFileWriter->WriteLine("} [color = gold]");			
		}

		for (auto&& pCurrPass : mpActivePasses)
		{
			for (TFrameGraphResourceHandle currResourceHandle : pCurrPass->GetReads())
			{
				pFileWriter->WriteLine(Wrench::StringUtils::Format("\"{0}\" -> {", mResourcesGraph[static_cast<USIZE>(currResourceHandle)].ToString()));
				pFileWriter->WriteLine("\"" + pCurrPass->GetName() + "\" ");
				pFileWriter->WriteLine("} [color = firebrick]");
			}
		}

		pFileWriter->WriteLine("}");

		return RC_OK;
	}

#endif

	CFrameGraph::TResourcesRegistry& CFrameGraph::GetResources(const CPassKey<CFrameGraphBuilder>& passkey)
	{
		return mResources;
	}

	std::vector<TFrameGraphResourceNode>& CFrameGraph::GetResourcesGraph(const CPassKey<CFrameGraphBuilder>& passkey)
	{
		return mResourcesGraph;
	}


	TDE2_API CFrameGraph* CreateFrameGraph(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CFrameGraph, CFrameGraph, result, pGraphicsContext);
	}
}