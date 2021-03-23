#include "../../../include/graphics/effects/ParticleEmitters.h"
#include "../../../include/graphics/effects/TParticle.h"
#include "../../../include/graphics/effects/IParticleEffect.h"
#include "../../../include/ecs/CTransform.h"
#include <unordered_map>
#include <functional>


namespace TDEngine2
{
	/*!
		\brief CBaseParticlesEmitter's definition
	*/

	struct TBaseParticlesEmitterArchiveKeys
	{
		static const std::string mIs2DModeKeyId;
	};

	const std::string TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId = "2d-mode";


	CBaseParticlesEmitter::CBaseParticlesEmitter():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseParticlesEmitter::Init(IParticleEffect* pOwnerEffect)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mpOwnerEffect = pOwnerEffect;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CBaseParticlesEmitter::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!mRefCounter)
		{
			delete this;
		}

		return RC_OK;
	}

	E_RESULT_CODE CBaseParticlesEmitter::SetOwnerEffect(IParticleEffect* pOwner)
	{
		if (!pOwner)
		{
			return RC_INVALID_ARGS;
		}

		mpOwnerEffect = pOwner;

		return RC_OK;
	}

	void CBaseParticlesEmitter::Set2DMode(bool value)
	{
		mIs2DEmitter = value;
	}

	bool CBaseParticlesEmitter::Is2DModeEnabled() const
	{
		return mIs2DEmitter;
	}


	TResult<CBaseParticlesEmitter*> CBaseParticlesEmitter::Load(IArchiveReader* pReader)
	{
		static const std::unordered_map<TypeId, std::function<CBaseParticlesEmitter*(IParticleEffect*, E_RESULT_CODE&)>> factoriesTable
		{
			{ CBoxParticlesEmitter::GetTypeId(), std::bind(&CreateBoxParticlesEmitter, std::placeholders::_1, std::placeholders::_2) },
		};

		const TypeId typeId = static_cast<TypeId>(pReader->GetUInt32("type_id"));

		auto it = factoriesTable.find(typeId);
		if (it == factoriesTable.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		E_RESULT_CODE result = RC_OK;
		CBaseParticlesEmitter* pEmitterPtr = (it->second)(nullptr, result);

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<CBaseParticlesEmitter*>(pEmitterPtr);
	}


	/*!
		\brief CBoxParticlesEmitter's definition
	*/


	struct TBoxParticlesEmitterArchiveKeys
	{
		static const std::string mSizesKeyId;
		static const std::string mOriginKeyId;
	};

	const std::string TBoxParticlesEmitterArchiveKeys::mSizesKeyId = "sizes";
	const std::string TBoxParticlesEmitterArchiveKeys::mOriginKeyId = "origin";


	CBoxParticlesEmitter::CBoxParticlesEmitter() :
		CBaseParticlesEmitter()
	{
	}

	E_RESULT_CODE CBoxParticlesEmitter::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		pReader->BeginGroup(TBoxParticlesEmitterArchiveKeys::mSizesKeyId);
		
		auto boxSizesResult = LoadVector3(pReader);
		if (boxSizesResult.HasError())
		{
			return boxSizesResult.GetError();
		}

		mBoxSizes = boxSizesResult.Get();

		pReader->EndGroup();

		pReader->BeginGroup(TBoxParticlesEmitterArchiveKeys::mOriginKeyId);

		auto boxOriginResult = LoadVector3(pReader);
		if (boxOriginResult.HasError())
		{
			return boxOriginResult.GetError();
		}

		mBoxOrigin = boxOriginResult.Get();

		pReader->EndGroup();

		mIs2DEmitter = pReader->GetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId);

		return RC_OK;
	}

	E_RESULT_CODE CBoxParticlesEmitter::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->BeginGroup(TBoxParticlesEmitterArchiveKeys::mSizesKeyId);
		SaveVector3(pWriter, mBoxSizes);
		pWriter->EndGroup();

		pWriter->BeginGroup(TBoxParticlesEmitterArchiveKeys::mOriginKeyId);
		SaveVector3(pWriter, mBoxOrigin);
		pWriter->EndGroup();

		pWriter->SetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId, mIs2DEmitter);

		return RC_OK;
	}

	E_RESULT_CODE CBoxParticlesEmitter::EmitParticle(const CTransform* pTransform, TParticle& particleInfo)
	{
		if (!mpOwnerEffect)
		{
			return RC_FAIL;
		}

		if (!pTransform)
		{
			return RC_INVALID_ARGS;
		}

		particleInfo.mAge = 0.0f;
		particleInfo.mLifeTime = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetLifetime());
		particleInfo.mColor = TColorUtils::mWhite;
		particleInfo.mSize = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetInitialSize());
		particleInfo.mPosition = pTransform->GetPosition() + RandVector3(mBoxOrigin - 0.5f * mBoxSizes, mBoxOrigin + 0.5f * mBoxSizes); // \todo Fix this with proper computation of transformed position
		particleInfo.mVelocity = ZeroVector3;

		if (mIs2DEmitter)
		{
			particleInfo.mPosition.z = pTransform->GetPosition().z; // \todo Fix this with proper computation of transformed position
		}

		return RC_OK;
	}


	TDE2_API CBaseParticlesEmitter* CreateBoxParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CBaseParticlesEmitter, CBoxParticlesEmitter, result, pOwnerEffect);
	}
}