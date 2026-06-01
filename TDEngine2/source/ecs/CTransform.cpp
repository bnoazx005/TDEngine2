#include "../../include/ecs/CTransform.h"
#include "../../include/core/Serialization.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"
#include "../../include/utils/CFileLogger.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateTransformFactory)


	TDE2_API TResult<TTransformComponentData> TTransformComponentData::Load(IArchiveReader* pReader)
	{
		TTransformComponentData resultData{};
		E_RESULT_CODE resultCode = RC_OK;

		Meta::VisitSerializableClassFields(resultData, [pReader, &resultCode](const C8* pFieldNamePtr, auto& fieldValue)
			{
				if constexpr (TIsVector<typename std::decay_t<decltype(fieldValue)>>::value)
				{
					auto&& result = Deserialize<typename std::decay_t<decltype(fieldValue)>>(pReader, pFieldNamePtr, "child_id");
					if (result.HasError())
					{
						resultCode = result.GetError();
						return;
					}

					fieldValue = result.Get();
				}
				else
				{
					auto&& result = Deserialize<typename std::decay_t<decltype(fieldValue)>>(pReader, pFieldNamePtr);
					if (result.HasError())
					{
						resultCode = result.GetError();
						return;
					}

					fieldValue = result.Get();
				}
			});

		if (RC_OK != resultCode)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(resultCode);
		}

		return Wrench::TOkValue<TTransformComponentData>(resultData);
	}
	
	
	TDE2_API E_RESULT_CODE TTransformComponentData::Save(IArchiveWriter* pWriter, const TTransformComponentData& data)
	{
		E_RESULT_CODE result = RC_OK;

		Meta::VisitSerializableClassFields(data, [pWriter, &result](const C8* pFieldNamePtr, const auto& fieldValue)
			{
				if constexpr (TIsVector<typename std::decay_t<decltype(fieldValue)>>::value)
				{
					result = result | Serialize(pWriter, pFieldNamePtr, fieldValue, "child_id");
				}
				else
				{
					result = result | Serialize<typename std::decay_t<decltype(fieldValue)>>(pWriter, pFieldNamePtr, fieldValue);
				}
			});

		return result;
	}
	

	CTransform::CTransform() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CTransform::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		auto&& loadResult = TTransformComponentData::Load(pReader);
		if (loadResult.HasError())
		{
			return loadResult.GetError();
		}

		mData = loadResult.GetOrDefault({});

		return RC_OK;
	}

	E_RESULT_CODE CTransform::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		result = result | pWriter->BeginGroup("component");
		{
			result = result | pWriter->SetUInt32("type_id", static_cast<U32>(CTransform::GetTypeId()));
			result = result | TTransformComponentData::Save(pWriter, mData);
		}
		result = result | pWriter->EndGroup();

		return result;
	}

	E_RESULT_CODE CTransform::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		if (!pEntityManager)
		{
			return RC_INVALID_ARGS;
		}

		mData.mOwnerId = entitiesIdentifiersRemapper.Resolve(mData.mOwnerId);

		for (TEntityId& currChildId : mData.mChildrenEntities)
		{
			currChildId = entitiesIdentifiersRemapper.Resolve(currChildId);

			if (auto pChildEntity = pEntityManager->GetEntity(currChildId))
			{
				if (auto pTransform = pChildEntity->GetComponent<CTransform>())
				{
					pTransform->SetParent(mData.mOwnerId);
				}
			}
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CTransform::Clone(IComponent*& pDestObject) const
	{
		if (CTransform* pDestComponent = dynamic_cast<CTransform*>(pDestObject))
		{
			pDestComponent->mData = mData;
			return RC_OK;
		}

		return RC_FAIL;
	}

	void CTransform::Reset()
	{
		mData = {};
	}

	void CTransform::SetPivot(const TVector3& pivot)
	{
		mData.mPivot = pivot;

		mData.mHasChanged = true;
	}

	void CTransform::SetPosition(const TVector3& position)
	{
		mData.mPosition = position;

		mData.mHasChanged = true;
	}

	void CTransform::SetRotation(const TVector3& eulerAngles)
	{
		mData.mRotation = TQuaternion(eulerAngles);

		mData.mHasChanged = true;
	}

	void CTransform::SetRotation(const TQuaternion& q)
	{
		mData.mRotation = q;

		mData.mHasChanged = true;
	}

	void CTransform::SetScale(const TVector3& scale)
	{
		mData.mScale = scale;

		mData.mHasChanged = true;
	}

	void CTransform::SetTransform(const TMatrix4& local2World, const TMatrix4& child2Parent)
	{
		mData.mLocalToWorldMatrix = local2World;
		mData.mWorldToLocalMatrix = Inverse(local2World);

		mData.mChild2ParentMatrix = child2Parent;
	}

	E_RESULT_CODE CTransform::AttachChild(TEntityId childEntityId)
	{
		if (childEntityId == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto it = std::find_if(mData.mChildrenEntities.cbegin(), mData.mChildrenEntities.cend(), [childEntityId](const TEntityId& id) { return id == childEntityId; });
		if (it != mData.mChildrenEntities.cend())
		{
			return RC_FAIL;
		}

		mData.mChildrenEntities.push_back(childEntityId);

		return RC_OK;
	}

	E_RESULT_CODE CTransform::DettachChild(TEntityId childEntityId)
	{
		if (childEntityId == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto it = std::find_if(mData.mChildrenEntities.cbegin(), mData.mChildrenEntities.cend(), [childEntityId](const TEntityId& id) { return id == childEntityId; });
		if (it == mData.mChildrenEntities.cend())
		{
			return RC_FAIL;
		}

		mData.mChildrenEntities.erase(it);

		return RC_OK;
	}

	E_RESULT_CODE CTransform::SetParent(TEntityId parentEntityId)
	{
		mData.mParentEntityId = parentEntityId;
		mData.mHasChanged = true;

		return RC_OK;
	}

	void CTransform::SetHierarchyChangedFlag(TEntityId parentEntityId)
	{
		mData.mPrevParentEntityId = parentEntityId;
	}

	void CTransform::SetDirtyFlag(bool value)
	{
		mData.mHasChanged = value;
	}

	E_RESULT_CODE CTransform::SetOwnerId(TEntityId id)
	{
		mData.mOwnerId = id;
		return RC_OK;
	}

	TEntityId CTransform::GetOwnerId() const
	{
		return mData.mOwnerId;
	}

	TEntityId CTransform::GetParent() const
	{
		return mData.mParentEntityId;
	}

	TEntityId CTransform::GetPrevParent() const
	{
		return mData.mPrevParentEntityId;
	}

	const TEntitiesArray& CTransform::GetChildren() const
	{
		return mData.mChildrenEntities;
	}

	const TVector3& CTransform::GetPivot() const
	{
		return mData.mPivot;
	}

	const TVector3& CTransform::GetPosition() const
	{
		return mData.mPosition;
	}

	const TQuaternion& CTransform::GetRotation() const
	{
		return mData.mRotation;
	}

	const TVector3& CTransform::GetScale() const
	{
		return mData.mScale;
	}

	const TMatrix4& CTransform::GetLocalToWorldTransform() const
	{
		return mData.mLocalToWorldMatrix;
	}

	const TMatrix4& CTransform::GetWorldToLocalTransform() const
	{
		return mData.mWorldToLocalMatrix;
	}

	const TMatrix4& CTransform::GetChildToParentTransform() const
	{
		return mData.mChild2ParentMatrix;
	}
	
	TVector3 CTransform::GetForwardVector() const
	{
		const TMatrix4& local2World = GetLocalToWorldTransform();
		return TVector3(local2World.m[0][2], local2World.m[1][2], local2World.m[2][2]);
	}

	TVector3 CTransform::GetRightVector() const
	{
		const TMatrix4& local2World = GetLocalToWorldTransform();
		return TVector3(local2World.m[0][0], local2World.m[1][0], local2World.m[2][0]);
	}
	
	TVector3 CTransform::GetUpVector() const
	{
		const TMatrix4& local2World = GetLocalToWorldTransform();
		return TVector3(local2World.m[0][1], local2World.m[1][1], local2World.m[2][1]);
	}

	bool CTransform::HasChanged() const
	{
		return mData.mHasChanged;
	}

	bool CTransform::HasHierarchyChanged() const
	{
		return mData.mPrevParentEntityId != mData.mParentEntityId;
	}

	const std::string& CTransform::GetTypeName() const
	{
		static const std::string id{ "transform" };
		return id;
	}

	IPropertyWrapperPtr CTransform::GetProperty(const std::string& propertyName)
	{
		static const std::unordered_map<std::string, std::function<IPropertyWrapperPtr(CTransform*)>> propertiesFactories
		{
			{ "position", [](CTransform* pTransform)
				{ 
					return IPropertyWrapperPtr(CBasePropertyWrapper<TVector3>::Create(
						[pTransform](const TVector3& pos) { pTransform->SetPosition(pos); return RC_OK; },
						[pTransform]() { return &pTransform->GetPosition(); }));
				} 
			},
			{ "rotation", [](CTransform* pTransform)
				{
					return IPropertyWrapperPtr(CBasePropertyWrapper<TQuaternion>::Create(
						[pTransform](const TQuaternion& rot) { pTransform->SetRotation(rot); return RC_OK; },
						[pTransform]() { return &pTransform->GetRotation(); }));
				} 
			},
			{ "scale", [](CTransform* pTransform)
				{ 
					return IPropertyWrapperPtr(CBasePropertyWrapper<TVector3>::Create(
						[pTransform](const TVector3& scale) { pTransform->SetPosition(scale); return RC_OK; },
						[pTransform]() { return &pTransform->GetScale(); }));
				} 
			}
		};

		auto it = propertiesFactories.find(propertyName);

		return (it != propertiesFactories.cend()) ? (it->second)(this) : CBaseComponent::GetProperty(propertyName);
	}

	const std::vector<std::string>& CTransform::GetAllProperties() const
	{
		static const std::vector<std::string> properties
		{
			"position",
			"rotation",
			"scale"
		};

		return properties;
	}
	
	void CTransform::ResetFirstFrameAfterCreationFlag()
	{
		mData.mIsFirstFrameAfterCreation = false;
	}

	bool CTransform::IsFirstFrameAfterCreation() const
	{
		return mData.mIsFirstFrameAfterCreation;
	}

	TTransformComponentData& CTransform::GetData()
	{
		return mData;
	}

	const TTransformComponentData& CTransform::GetData() const
	{
		return mData;
	}


	IComponent* CreateTransform(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CTransform, result);
	}


	/*!
		\brief CTransformFactory's definition
	*/
	
	CTransformFactory::CTransformFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CTransformFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateTransform(result);
	}

	E_RESULT_CODE CTransformFactory::SetupComponent(CTransform* pComponent, const TTransformParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		pComponent->SetPosition(params.mPosition);
		pComponent->SetRotation(params.mRotation);
		pComponent->SetScale(params.mScale);

		return RC_OK;
	}


	IComponentFactory* CreateTransformFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CTransformFactory, result);
	}

}