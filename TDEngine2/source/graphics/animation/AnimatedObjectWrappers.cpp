#include "../../../include/graphics/animation/AnimatedObjectWrappers.h"
#include "../../../include/ecs/IWorld.h"
#include "../../../include/ecs/CEntity.h"
#include "../../../include/ecs/CTransform.h"
#include <result.hpp>
#include <stringUtils.hpp>


namespace TDEngine2
{
	/*!
		\brief CBaseAnimatedObjectWrapper's definition
	*/

	CBaseAnimatedObjectWrapper::CBaseAnimatedObjectWrapper() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CBaseAnimatedObjectWrapper::Free()
	{
		if (!mIsInitialized)
		{
			return RC_OK;
		}

		--mRefCounter;

		if (!mRefCounter)
		{
			mIsInitialized = false;
			delete this;
		}

		return RC_OK;
	}


	/*!
		\brief CAnimatedEntityTransformObject's definition
	*/

	CAnimatedEntityTransformWrapper::CAnimatedEntityTransformWrapper() :
		CBaseAnimatedObjectWrapper()
	{
	}

	E_RESULT_CODE CAnimatedEntityTransformWrapper::Init(IWorld* pWorld, TEntityId entityId)
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		if (!pWorld || entityId == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		mpWorld = pWorld;
		mEntityId = entityId;

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CAnimatedEntityTransformWrapper::_setInternal(const std::string& propertyName, const void* pValue, size_t valueSize)
	{
		if (!pValue || !valueSize || propertyName.empty())
		{
			return RC_INVALID_ARGS;
		}

		auto pEntity = mpWorld->FindEntity(mEntityId);
		if (!pEntity)
		{
			return RC_FAIL;
		}

		auto pTransform = pEntity->GetComponent<CTransform>();
		if (!pTransform)
		{
			return RC_FAIL;
		}

		static const std::unordered_map<std::string, std::function<void()>> propertiesMap
		{
			{ "position", [pTransform, pValue] { pTransform->SetPosition(*static_cast<const TVector3*>(pValue)); } },
			{ "rotation", [pTransform, pValue] { pTransform->SetRotation(*static_cast<const TQuaternion*>(pValue)); } },
			{ "scale", [pTransform, pValue] { pTransform->SetScale(*static_cast<const TVector3*>(pValue)); } },
		};

		auto it = propertiesMap.find(propertyName);
		if (it == propertiesMap.cend())
		{
			return RC_FAIL;
		}

		(it->second)();

		return RC_OK;
	}
	
	const void* CAnimatedEntityTransformWrapper::_getInternal(const std::string& propertyName) const
	{
		if (propertyName.empty())
		{
			Wrench::Panic("[CAnimatedEntityTransformWrapper] Property cannot be empty string");
			return nullptr;
		}

		auto pEntity = mpWorld->FindEntity(mEntityId);
		if (!pEntity)
		{
			Wrench::Panic(Wrench::StringUtils::Format("[CAnimatedEntityTransformWrapper] There is not such entity, entity {0}", static_cast<U32>(mEntityId)));
			return nullptr;
		}

		auto pTransform = pEntity->GetComponent<CTransform>();
		
		if (propertyName == "position")	{ return static_cast<const void*>(&pTransform->GetPosition()); }
		if (propertyName == "rotation") { return static_cast<const void*>(&pTransform->GetRotation()); }
		if (propertyName == "scale") { return static_cast<const void*>(&pTransform->GetScale()); }

		Wrench::Panic(Wrench::StringUtils::Format("[CAnimatedEntityTransformWrapper] There is not such property, property {0}", propertyName));
		return nullptr;
	}


	TDE2_API IAnimatedObjectWrapper* CreateAnimatedEntityTransformWrapper(IWorld* pWorld, TEntityId entityId, E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IAnimatedObjectWrapper, CAnimatedEntityTransformWrapper, result, pWorld, entityId);
	}
}