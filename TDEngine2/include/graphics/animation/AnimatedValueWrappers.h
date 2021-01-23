/*!
	\file AnimatedValueWrappers.h
	\date 21.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../math/TVector3.h"
#include "../../math/TQuaternion.h"
#include "../../ecs/IWorld.h"
#include "../../ecs/CEntity.h"
#include "../../ecs/CTransform.h"


namespace TDEngine2
{
	/*!
		\brief The wrapper is simple for built-in types
	*/

	template <typename T>
	struct TAnimatedValueWrapper
	{
		explicit TAnimatedValueWrapper(T&& value) :
			mValue(std::forward<T>(value))
		{
		}

		void Set(T&& value)
		{
			mValue = std::forward<T>(value);
		}

		T&& Get() const
		{
			return mValue;
		}

		T mValue;
	};


	typedef struct TTransformKey
	{
		TVector3 mPosition;
		TQuaternion mRotation;
		TVector3 mScale;
	} TTransformKey, *TTransformKeyPtr;


	class IWorld;


	template <>
	struct TAnimatedValueWrapper<TTransformKey>
	{
		TAnimatedValueWrapper() = delete;
		TAnimatedValueWrapper(IWorld* pWorld, TEntityId entityId, TTransformKey&& value) :
			mValue(std::forward<TTransformKey>(value)), mpWorld(pWorld), mEntityId(entityId)
		{
		}

		void Set(TTransformKey&& value)
		{
			mValue = std::forward<TTransformKey>(value);

			if (!mpWorld)
			{
				TDE2_ASSERT(false);
				return;
			}

			if (auto pEntity = mpWorld->FindEntity(mEntityId))
			{
				if (auto pTransform = pEntity->GetComponent<CTransform>())
				{
					pTransform->SetPosition(value.mPosition);
					pTransform->SetRotation(value.mRotation);
					pTransform->SetScale(value.mScale);
				}
			}
		}

		const TTransformKey& Get() const
		{
			return mValue;
		}

		IWorld* mpWorld;

		TEntityId mEntityId;

		TTransformKey mValue;
	};
}