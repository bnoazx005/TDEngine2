#include "../../../include/graphics/effects/ParticleEmitters.h"
#include "../../../include/graphics/effects/TParticle.h"
#include "../../../include/graphics/effects/IParticleEffect.h"
#include "../../../include/ecs/CTransform.h"
#include "../../../include/math/TVector2.h"
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

		--mRefCounter;

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

	TColor32F CBaseParticlesEmitter::GetColorData(const TParticleColorParameter& colorData, F32 time)
	{
		switch (colorData.mType)
		{
			case E_PARTICLE_COLOR_PARAMETER_TYPE::SINGLE_COLOR:
				return colorData.mFirstColor;

			case E_PARTICLE_COLOR_PARAMETER_TYPE::TWEEN_RANDOM:
				return LerpColors(colorData.mFirstColor, colorData.mSecondColor, CRandomUtils::GetRandF32Value({ 0.0f, 1.0f }));

			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_LERP:
				if (colorData.mGradientColor)
				{
					return colorData.mGradientColor->Sample(time);
				}

				TDE2_ASSERT(colorData.mGradientColor);
				return TColorUtils::mWhite;

			case E_PARTICLE_COLOR_PARAMETER_TYPE::GRADIENT_RANDOM:
				if (colorData.mGradientColor)
				{
					return colorData.mGradientColor->Sample(CRandomUtils::GetRandF32Value({ 0.0f, 1.0f }));
				}

				TDE2_ASSERT(colorData.mGradientColor);
				return TColorUtils::mWhite;
		}

		TDE2_UNREACHABLE();

		return TColorUtils::mWhite;
	}

	TVector3 CBaseParticlesEmitter::GetVelocityData(const TParticleVelocityParameter& velocityData, F32 time)
	{
		switch (velocityData.mType)
		{
			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CONSTANTS:
				if (Length(velocityData.mVelocityConst) < 1e-3f)
				{
					return velocityData.mVelocityConst;
				}

				return Normalize(velocityData.mVelocityConst) * velocityData.mSpeedFactorConst;
			
			case E_PARTICLE_VELOCITY_PARAMETER_TYPE::CURVES:
			{
				const TVector3 velocity { velocityData.mXCurve->Sample(time), velocityData.mYCurve->Sample(time), velocityData.mZCurve->Sample(time) };
				if (Length(velocity) < 1e-3f)
				{
					return velocity;
				}

				return Normalize(velocity) * velocityData.mSpeedFactorCurve->Sample(time);
			}

			default:
				TDE2_UNREACHABLE();
				break;
		}

		return ZeroVector3;
	}

	TColor32F CBaseParticlesEmitter::_getInitColor() const
	{
		if (!mpOwnerEffect)
		{
			TDE2_ASSERT(false);
			return TColorUtils::mWhite;
		}

		return GetColorData(mpOwnerEffect->GetInitialColor(), 0.0f);
	}

	TVector3 CBaseParticlesEmitter::_getInitVelocity() const
	{
		if (!mpOwnerEffect)
		{
			TDE2_ASSERT(false);
			return ZeroVector3;
		}

		//TDE2_ASSERT(Length(mpOwnerEffect->GetInitialMoveDirection()) < 1e-3f);

		return Normalize(mpOwnerEffect->GetInitialMoveDirection()) * mpOwnerEffect->GetInitialSpeedFactor();
	}

	
	static const std::unordered_map<TypeId, std::function<CBaseParticlesEmitter*(IParticleEffect*, E_RESULT_CODE&)>> FactoriesTable
	{
		{ CBoxParticlesEmitter::GetTypeId(), std::bind(&CreateBoxParticlesEmitter, std::placeholders::_1, std::placeholders::_2) },
		{ CSphereParticlesEmitter::GetTypeId(), std::bind(&CreateSphereParticlesEmitter, std::placeholders::_1, std::placeholders::_2) },
		{ CConeParticlesEmitter::GetTypeId(), std::bind(&CreateConeParticlesEmitter, std::placeholders::_1, std::placeholders::_2) },
	};


	TResult<CBaseParticlesEmitter*> CBaseParticlesEmitter::Deserialize(IArchiveReader* pReader)
	{		
		const TypeId typeId = static_cast<TypeId>(pReader->GetUInt32("type_id"));

		auto it = FactoriesTable.find(typeId);
		if (it == FactoriesTable.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		E_RESULT_CODE result = RC_OK;
		CBaseParticlesEmitter* pEmitterPtr = (it->second)(nullptr, result);

		if (RC_OK != result)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		if (RC_OK != (result = pEmitterPtr->Load(pReader)))
		{
			return Wrench::TErrValue<E_RESULT_CODE>(result);
		}

		return Wrench::TOkValue<CBaseParticlesEmitter*>(pEmitterPtr);
	}

	const std::vector<std::tuple<std::string, TypeId>>& CBaseParticlesEmitter::GetEmittersTypes()
	{
		static const std::vector<std::tuple<std::string, TypeId>> emittersTypesIdentifiers
		{
			{ "Box", CBoxParticlesEmitter::GetTypeId() },
			{ "Sphere", CSphereParticlesEmitter::GetTypeId() },
			{ "Cone", CConeParticlesEmitter::GetTypeId() }
		};

		return emittersTypesIdentifiers;
	}


	CScopedPtr<CBaseParticlesEmitter> CreateEmitterByTypeId(IParticleEffect* pOwnerEffect, TypeId id)
	{
		auto it = FactoriesTable.find(id);
		if (it == FactoriesTable.cend())
		{
			return CScopedPtr<CBaseParticlesEmitter> { nullptr };
		}

		E_RESULT_CODE result = RC_OK;
		CBaseParticlesEmitter* pEmitterPtr = (it->second)(pOwnerEffect, result);

		if (RC_OK != result)
		{
			return CScopedPtr<CBaseParticlesEmitter> { nullptr };
		}

		return CScopedPtr<CBaseParticlesEmitter>{ pEmitterPtr };
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

		pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId()));
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
		particleInfo.mColor = _getInitColor();
		particleInfo.mSize = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetInitialSize());
		particleInfo.mPosition = pTransform->GetPosition() + RandVector3(mBoxOrigin - 0.5f * mBoxSizes, mBoxOrigin + 0.5f * mBoxSizes); // \todo Fix this with proper computation of transformed position
		particleInfo.mVelocity = _getInitVelocity();
		particleInfo.mRotation = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetInitialRotation());

		if (mIs2DEmitter)
		{
			particleInfo.mPosition.z = pTransform->GetPosition().z; // \todo Fix this with proper computation of transformed position
		}

		return RC_OK;
	}

	E_RESULT_CODE CBoxParticlesEmitter::SetSizes(const TVector3& sizes)
	{
		if (Length(sizes) < 1e-3f)
		{
			return RC_INVALID_ARGS;
		}

		mBoxSizes = sizes;

		return RC_OK;
	}

	E_RESULT_CODE CBoxParticlesEmitter::SetOrigin(const TVector3& origin)
	{
		mBoxOrigin = origin;
		return RC_OK;
	}

	const TVector3& CBoxParticlesEmitter::GetSizes() const
	{
		return mBoxSizes;
	}

	const TVector3& CBoxParticlesEmitter::GetOrigin() const
	{
		return mBoxOrigin;
	}


	TDE2_API CBaseParticlesEmitter* CreateBoxParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CBaseParticlesEmitter, CBoxParticlesEmitter, result, pOwnerEffect);
	}


	/*!
		\brief CSphereParticlesEmitter's definition
	*/


	struct TSphereParticlesEmitterArchiveKeys
	{
		static const std::string mRadiusKeyId;
		static const std::string mOriginKeyId;
	};

	const std::string TSphereParticlesEmitterArchiveKeys::mRadiusKeyId = "radius";
	const std::string TSphereParticlesEmitterArchiveKeys::mOriginKeyId = "origin";


	CSphereParticlesEmitter::CSphereParticlesEmitter() :
		CBaseParticlesEmitter()
	{
	}

	E_RESULT_CODE CSphereParticlesEmitter::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mRadius = pReader->GetFloat(TSphereParticlesEmitterArchiveKeys::mRadiusKeyId);

		pReader->EndGroup();

		pReader->BeginGroup(TSphereParticlesEmitterArchiveKeys::mOriginKeyId);

		auto boxOriginResult = LoadVector3(pReader);
		if (boxOriginResult.HasError())
		{
			return boxOriginResult.GetError();
		}

		mOrigin = boxOriginResult.Get();

		pReader->EndGroup();

		mIs2DEmitter = pReader->GetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId);

		return RC_OK;
	}

	E_RESULT_CODE CSphereParticlesEmitter::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->SetFloat(TSphereParticlesEmitterArchiveKeys::mRadiusKeyId, mRadius);

		pWriter->BeginGroup(TSphereParticlesEmitterArchiveKeys::mOriginKeyId);
		SaveVector3(pWriter, mOrigin);
		pWriter->EndGroup();

		pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId()));
		pWriter->SetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId, mIs2DEmitter);

		return RC_OK;
	}

	E_RESULT_CODE CSphereParticlesEmitter::EmitParticle(const CTransform* pTransform, TParticle& particleInfo)
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
		particleInfo.mColor = _getInitColor();
		particleInfo.mSize = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetInitialSize());
		particleInfo.mPosition = pTransform->GetPosition() + mOrigin + Normalize(RandVector3(TVector3 { 0.001f }, TVector3 { 1.0f })) * mRadius; // \todo Fix this with proper computation of transformed position
		particleInfo.mVelocity = _getInitVelocity();

		if (mIs2DEmitter)
		{
			particleInfo.mPosition.z = pTransform->GetPosition().z; // \todo Fix this with proper computation of transformed position
		}

		return RC_OK;
	}

	E_RESULT_CODE CSphereParticlesEmitter::SetOrigin(const TVector3& origin)
	{
		mOrigin = origin;
		return RC_OK;
	}

	E_RESULT_CODE CSphereParticlesEmitter::SetRadius(F32 radius)
	{
		if (radius < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRadius = radius;
		return RC_OK;
	}

	const TVector3& CSphereParticlesEmitter::GetOrigin() const
	{
		return mOrigin;
	}

	F32 CSphereParticlesEmitter::GetRadius() const
	{
		return mRadius;
	}


	TDE2_API CBaseParticlesEmitter* CreateSphereParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CBaseParticlesEmitter, CSphereParticlesEmitter, result, pOwnerEffect);
	}


	/*!
		\brief CConeParticlesEmitter's definition
	*/


	struct TConeParticlesEmitterArchiveKeys
	{
		static const std::string mRadiusKeyId;
		static const std::string mHeightKeyId;
	};

	const std::string TConeParticlesEmitterArchiveKeys::mRadiusKeyId = "radius";
	const std::string TConeParticlesEmitterArchiveKeys::mHeightKeyId = "height";


	CConeParticlesEmitter::CConeParticlesEmitter() :
		CBaseParticlesEmitter()
	{
	}

	E_RESULT_CODE CConeParticlesEmitter::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_INVALID_ARGS;
		}

		mRadius = pReader->GetFloat(TConeParticlesEmitterArchiveKeys::mRadiusKeyId);
		mHeight = pReader->GetFloat(TConeParticlesEmitterArchiveKeys::mHeightKeyId);

		mIs2DEmitter = pReader->GetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId);

		return RC_OK;
	}

	E_RESULT_CODE CConeParticlesEmitter::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_INVALID_ARGS;
		}

		pWriter->SetFloat(TConeParticlesEmitterArchiveKeys::mRadiusKeyId, mRadius);
		pWriter->SetFloat(TConeParticlesEmitterArchiveKeys::mHeightKeyId, mHeight);

		pWriter->SetUInt32("type_id", static_cast<U32>(GetTypeId()));
		pWriter->SetBool(TBaseParticlesEmitterArchiveKeys::mIs2DModeKeyId, mIs2DEmitter);

		return RC_OK;
	}

	E_RESULT_CODE CConeParticlesEmitter::EmitParticle(const CTransform* pTransform, TParticle& particleInfo)
	{
		if (!mpOwnerEffect)
		{
			return RC_FAIL;
		}

		if (!pTransform)
		{
			return RC_INVALID_ARGS;
		}

		// \todo Clean up this listing
		TVector2 pos = TVector2(CRandomUtils::GetRandF32Value({ 0.0f, 1.0f }), CRandomUtils::GetRandF32Value({ 0.0f, 1.0f })); // \todo Replace with helper functions
		const F32 h = CRandomUtils::GetRandF32Value({ 0.0f, 1.0f });

		F32 r = h * mRadius * 0.5f / mHeight;
		pos = Normalize(pos) * r;

		particleInfo.mAge = 0.0f;
		particleInfo.mLifeTime = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetLifetime());
		particleInfo.mColor = _getInitColor();
		particleInfo.mSize = CRandomUtils::GetRandF32Value(mpOwnerEffect->GetInitialSize());
		particleInfo.mPosition = pTransform->GetPosition() + TVector3(pos.x, h, pos.y); // \todo Fix this with proper computation of transformed position
		particleInfo.mVelocity = ZeroVector3;

		if (mIs2DEmitter)
		{
			particleInfo.mPosition.z = pTransform->GetPosition().z; // \todo Fix this with proper computation of transformed position
		}

		return RC_OK;
	}

	E_RESULT_CODE CConeParticlesEmitter::SetHeight(F32 height)
	{
		if (height < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mHeight = height;

		return RC_OK;
	}

	E_RESULT_CODE CConeParticlesEmitter::SetRadius(F32 radius)
	{
		if (radius < 0.0f)
		{
			return RC_INVALID_ARGS;
		}

		mRadius = radius;

		return RC_OK;
	}

	F32 CConeParticlesEmitter::GetHeight() const
	{
		return mHeight;
	}

	F32 CConeParticlesEmitter::GetRadius() const
	{
		return mRadius;
	}


	TDE2_API CBaseParticlesEmitter* CreateConeParticlesEmitter(IParticleEffect* pOwnerEffect, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CBaseParticlesEmitter, CConeParticlesEmitter, result, pOwnerEffect);
	}
}