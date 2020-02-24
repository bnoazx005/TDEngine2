#include "./../../include/physics/CBaseRaycastContext.h"
#include "./../../include/ecs/CPhysics2DSystem.h"
#include "./../../include/ecs/CPhysics3DSystem.h"
#include "./../../include/core/memory/CPoolAllocator.h"
#include "./../../include/core/memory/IMemoryManager.h"


namespace TDEngine2
{
	CBaseRaycastContext::CBaseRaycastContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseRaycastContext::Init(IMemoryManager* pMemoryManager, CPhysics2DSystem* p2DPhysicsSystem, CPhysics3DSystem* p3DPhysicsSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pMemoryManager || !p2DPhysicsSystem || !p3DPhysicsSystem)
		{
			return RC_INVALID_ARGS;
		}

		mpAllocator = pMemoryManager->CreateAllocator<CPoolAllocator>(TPoolAllocatorParams 
																	  { 
																			mMaxRaycastsPerFrame * mRaycastResultTypeSize,
																			mRaycastResultTypeSize,
																			__alignof(TRaycastResult) 
																	  }, 
																	  "raycasts_allocator");

		mp2DPhysicsSystem = p2DPhysicsSystem;
		mp3DPhysicsSystem = p3DPhysicsSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseRaycastContext::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;
		
		delete this;

		return RC_OK;
	}

	TRaycastResult* CBaseRaycastContext::Raycast2DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance)
	{
		TDE2_UNIMPLEMENTED();
		return nullptr;
	}

	TRaycastResult* CBaseRaycastContext::Raycast3DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance)
	{
		TRaycastResult* pResult = nullptr;

		mp3DPhysicsSystem->RaycastClosest(origin, direction, maxDistance, [this, &pResult](const TRaycastResult& hitResult)
		{
			pResult = static_cast<TRaycastResult*>(mpAllocator->Allocate(sizeof(mRaycastResultTypeSize), __alignof(TRaycastResult)));
			*pResult = hitResult;
		});

		return pResult;
	}

	bool CBaseRaycastContext::Raycast3DAll(const TVector3& origin, const TVector3& direction, F32 maxDistance, std::vector<TRaycastResult>& result)
	{
		return mp3DPhysicsSystem->RaycastAll(origin, direction, maxDistance, result);
	}

	void CBaseRaycastContext::Reset()
	{
		TDE2_ASSERT(mpAllocator->Clear() == RC_OK);
	}


	TDE2_API IRaycastContext* CreateBaseRaycastContext(IMemoryManager* pMemoryManager, CPhysics2DSystem* p2DPhysicsSystem, CPhysics3DSystem* p3DPhysicsSystem, E_RESULT_CODE& result)
	{
		CBaseRaycastContext* pRaycastContextInstance = new (std::nothrow) CBaseRaycastContext();

		if (!pRaycastContextInstance)
		{
			result = RC_OUT_OF_MEMORY;

			return nullptr;
		}

		result = pRaycastContextInstance->Init(pMemoryManager, p2DPhysicsSystem, p3DPhysicsSystem);

		if (result != RC_OK)
		{
			delete pRaycastContextInstance;

			pRaycastContextInstance = nullptr;
		}

		return dynamic_cast<IRaycastContext*>(pRaycastContextInstance);
	}
}