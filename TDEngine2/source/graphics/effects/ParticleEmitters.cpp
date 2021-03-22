#include "../../../include/graphics/effects/ParticleEmitters.h"
#include "../../../include/graphics/effects/TParticle.h"
#include <unordered_map>
#include <functional>


namespace TDEngine2
{
	/*!
		\brief CBaseParticlesEmitter's definition
	*/

	CBaseParticlesEmitter::CBaseParticlesEmitter():
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseParticlesEmitter::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

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

	TResult<CBaseParticlesEmitter*> CBaseParticlesEmitter::Load(IArchiveReader* pReader)
	{
		static const std::unordered_map<TypeId, std::function<CBaseParticlesEmitter*(E_RESULT_CODE&)>> factoriesTable
		{
			{ CBoxParticlesEmitter::GetTypeId(), std::bind(&CreateBoxParticlesEmitter, std::placeholders::_1) },
		};

		const TypeId typeId = static_cast<TypeId>(pReader->GetUInt32("type_id"));

		auto it = factoriesTable.find(typeId);
		if (it == factoriesTable.cend())
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		E_RESULT_CODE result = RC_OK;
		CBaseParticlesEmitter* pEmitterPtr = (it->second)(result);

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

		return RC_OK;
	}

	E_RESULT_CODE CBoxParticlesEmitter::EmitParticle(const CTransform* pTransform, TParticle& particleInfo)
	{
		return RC_OK;
	}


	TDE2_API CBaseParticlesEmitter* CreateBoxParticlesEmitter(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(CBaseParticlesEmitter, CBoxParticlesEmitter, result);
	}
}