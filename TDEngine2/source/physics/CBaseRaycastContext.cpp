#include "../../include/physics/CBaseRaycastContext.h"
#include "../../include/ecs/CPhysics2DSystem.h"
#include "../../include/ecs/CPhysics3DSystem.h"
#include "../../include/core/memory/CPoolAllocator.h"


namespace TDEngine2
{
	CBaseRaycastContext::CBaseRaycastContext() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseRaycastContext::Init(CPhysics2DSystem* p2DPhysicsSystem, CPhysics3DSystem* p3DPhysicsSystem)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!p2DPhysicsSystem || !p3DPhysicsSystem)
		{
			return RC_INVALID_ARGS;
		}

		E_RESULT_CODE result = RC_OK;

		mpAllocator = CreatePoolAllocator(mRaycastResultTypeSize, __alignof(TRaycastResult), mMaxRaycastsPerFrame * mRaycastResultTypeSize, result);

		if (RC_OK != result)
		{
			return result;
		}

		mp2DPhysicsSystem = p2DPhysicsSystem;
		mp3DPhysicsSystem = p3DPhysicsSystem;

		mIsInitialized = true;

		return RC_OK;
	}

	TRaycastResult* CBaseRaycastContext::Raycast2DClosest(const TVector3& origin, const TVector3& direction, F32 maxDistance)
	{
		TRaycastResult* pResult = nullptr;

		mp2DPhysicsSystem->RaycastClosest({ origin.x, origin.y }, { direction.x, direction.y }, maxDistance, [this, &pResult](const TRaycastResult& hitResult)
		{
			pResult = static_cast<TRaycastResult*>(mpAllocator->Allocate(sizeof(mRaycastResultTypeSize), __alignof(TRaycastResult)));
			*pResult = hitResult;
		});

		return pResult;
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
		E_RESULT_CODE result = mpAllocator->Clear();
		TDE2_ASSERT(result == RC_OK);
	}


	TDE2_API IRaycastContext* CreateBaseRaycastContext(CPhysics2DSystem* p2DPhysicsSystem, CPhysics3DSystem* p3DPhysicsSystem, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IRaycastContext, CBaseRaycastContext, result, p2DPhysicsSystem, p3DPhysicsSystem);
	}
}