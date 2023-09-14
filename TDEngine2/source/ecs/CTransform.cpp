#include "../../include/ecs/CTransform.h"
#include "../../include/core/Serialization.h"
#include "../../include/ecs/CWorld.h"
#include "../../include/ecs/CEntity.h"


namespace TDEngine2
{
	CTransform::CTransform() :
		CBaseComponent(), 
		mLocalToWorldMatrix(IdentityMatrix4), 
		mHasChanged(true),
		mPosition(ZeroVector3),
		mRotation(UnitQuaternion),
		mScale(1.0f)
	{
	}

	E_RESULT_CODE CTransform::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}

		pReader->BeginGroup("pivot");
		if (auto pivotResult = LoadVector3(pReader))
		{
			mPivot = pivotResult.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup("position");
		if (auto positionResult = LoadVector3(pReader))
		{
			mPosition = positionResult.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup("rotation");
		if (auto rotationResult = LoadQuaternion(pReader))
		{
			mRotation = rotationResult.Get();
		}
		pReader->EndGroup();

		pReader->BeginGroup("scale");
		if (auto scaleResult = LoadVector3(pReader))
		{
			mScale = scaleResult.Get();
		}
		pReader->EndGroup();

		// \note Load children's identifiers
		pReader->BeginGroup("Children");
		{
			while (pReader->HasNextItem())
			{
				pReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
				{
					mChildrenEntities.push_back(static_cast<TEntityId>(pReader->GetUInt32("child_id")));
				}
				pReader->EndGroup();
			}
		}
		pReader->EndGroup();

		mParentEntityId = static_cast<TEntityId>(pReader->GetUInt32("parent_id", static_cast<U32>(TEntityId::Invalid)));
		mOwnerId = static_cast<TEntityId>(pReader->GetUInt32("owner_id", static_cast<U32>(TEntityId::Invalid)));
		
		mPrevParentEntityId = TEntityId::Invalid;

		mIsFirstFrameAfterCreation = true;

		return RC_OK;
	}

	E_RESULT_CODE CTransform::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CTransform::GetTypeId()));

			pWriter->BeginGroup("pivot", false);
			SaveVector3(pWriter, mPivot);
			pWriter->EndGroup();

			pWriter->BeginGroup("position", false);
			SaveVector3(pWriter, mPosition);
			pWriter->EndGroup();
			
			pWriter->BeginGroup("rotation", false);
			SaveQuaternion(pWriter, mRotation);
			pWriter->EndGroup();

			pWriter->BeginGroup("scale", false);
			SaveVector3(pWriter, mScale);
			pWriter->EndGroup();

			// \note Save children's identifiers
			pWriter->BeginGroup("Children", true);
			{
				for (const TEntityId currChildId : mChildrenEntities)
				{
					pWriter->BeginGroup(Wrench::StringUtils::GetEmptyStr());
					{
						pWriter->SetUInt32("child_id", static_cast<U32>(currChildId));
					}
					pWriter->EndGroup();
				}
			}
			pWriter->EndGroup();

			pWriter->SetUInt32("parent_id", static_cast<U32>(mParentEntityId));
			pWriter->SetUInt32("owner_id", static_cast<U32>(mOwnerId));
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	E_RESULT_CODE CTransform::PostLoad(CEntityManager* pEntityManager, const TEntitiesMapper& entitiesIdentifiersRemapper)
	{
		if (!pEntityManager)
		{
			return RC_INVALID_ARGS;
		}

		mOwnerId = entitiesIdentifiersRemapper.Resolve(mOwnerId);

		for (TEntityId& currChildId : mChildrenEntities)
		{
			currChildId = entitiesIdentifiersRemapper.Resolve(currChildId);

			if (auto pChildEntity = pEntityManager->GetEntity(currChildId))
			{
				if (auto pTransform = pChildEntity->GetComponent<CTransform>())
				{
					pTransform->SetParent(mOwnerId);
				}
			}
		}

		return RC_OK;
	}
	
	E_RESULT_CODE CTransform::Clone(IComponent*& pDestObject) const
	{
		if (CTransform* pDestComponent = dynamic_cast<CTransform*>(pDestObject))
		{
			pDestComponent->mPivot = mPivot;
			pDestComponent->mPosition = mPosition;
			pDestComponent->mRotation = mRotation;
			pDestComponent->mScale = mScale;
			pDestComponent->mParentEntityId = mParentEntityId;
			pDestComponent->mOwnerId = mOwnerId;

			pDestComponent->mLocalToWorldMatrix = mLocalToWorldMatrix;
			pDestComponent->mWorldToLocalMatrix = mWorldToLocalMatrix;
			pDestComponent->mHasChanged = mHasChanged;

			std::copy(mChildrenEntities.begin(), mChildrenEntities.end(), std::back_inserter(pDestComponent->mChildrenEntities));

			return RC_OK;
		}

		return RC_FAIL;
	}

	void CTransform::Reset()
	{
		mPivot = ZeroVector3;

		mPosition = ZeroVector3;

		mRotation = UnitQuaternion;

		mScale = TVector3(1.0f, 1.0f, 1.0f);

		mHasChanged = true;
		mIsFirstFrameAfterCreation = true;
	}

	void CTransform::SetPivot(const TVector3& pivot)
	{
		mPivot = pivot;

		mHasChanged = true;
	}

	void CTransform::SetPosition(const TVector3& position)
	{
		mPosition = position;

		mHasChanged = true;
	}

	void CTransform::SetRotation(const TVector3& eulerAngles)
	{
		mRotation = TQuaternion(eulerAngles);

		mHasChanged = true;
	}

	void CTransform::SetRotation(const TQuaternion& q)
	{
		mRotation = q;

		mHasChanged = true;
	}

	void CTransform::SetScale(const TVector3& scale)
	{
		mScale = scale;

		mHasChanged = true;
	}

	void CTransform::SetTransform(const TMatrix4& local2World, const TMatrix4& child2Parent)
	{
		mLocalToWorldMatrix = local2World;
		mWorldToLocalMatrix = Inverse(local2World);

		mChild2ParentMatrix = child2Parent;
	}

	E_RESULT_CODE CTransform::AttachChild(TEntityId childEntityId)
	{
		if (childEntityId == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto it = std::find_if(mChildrenEntities.cbegin(), mChildrenEntities.cend(), [childEntityId](const TEntityId& id) { return id == childEntityId; });
		if (it != mChildrenEntities.cend())
		{
			return RC_FAIL;
		}

		mChildrenEntities.push_back(childEntityId);

		return RC_OK;
	}

	E_RESULT_CODE CTransform::DettachChild(TEntityId childEntityId)
	{
		if (childEntityId == TEntityId::Invalid)
		{
			return RC_INVALID_ARGS;
		}

		auto it = std::find_if(mChildrenEntities.cbegin(), mChildrenEntities.cend(), [childEntityId](const TEntityId& id) { return id == childEntityId; });
		if (it == mChildrenEntities.cend())
		{
			return RC_FAIL;
		}

		mChildrenEntities.erase(it);

		return RC_OK;
	}

	E_RESULT_CODE CTransform::SetParent(TEntityId parentEntityId)
	{
		mParentEntityId = parentEntityId;
		mHasChanged = true;

		return RC_OK;
	}

	void CTransform::SetHierarchyChangedFlag(TEntityId parentEntityId)
	{
		mPrevParentEntityId = parentEntityId;
	}

	void CTransform::SetDirtyFlag(bool value)
	{
		mHasChanged = value;
	}

	E_RESULT_CODE CTransform::SetOwnerId(TEntityId id)
	{
		mOwnerId = id;
		return RC_OK;
	}

	TEntityId CTransform::GetOwnerId() const
	{
		return mOwnerId;
	}

	TEntityId CTransform::GetParent() const
	{
		return mParentEntityId;
	}

	TEntityId CTransform::GetPrevParent() const
	{
		return mPrevParentEntityId;
	}

	const std::vector<TEntityId>& CTransform::GetChildren() const
	{
		return mChildrenEntities;
	}

	const TVector3& CTransform::GetPivot() const
	{
		return mPivot;
	}

	const TVector3& CTransform::GetPosition() const
	{
		return mPosition;
	}

	const TQuaternion& CTransform::GetRotation() const
	{
		return mRotation;
	}

	const TVector3& CTransform::GetScale() const
	{
		return mScale;
	}

	const TMatrix4& CTransform::GetLocalToWorldTransform() const
	{
		return mLocalToWorldMatrix;
	}

	const TMatrix4& CTransform::GetWorldToLocalTransform() const
	{
		return mWorldToLocalMatrix;
	}

	const TMatrix4& CTransform::GetChildToParentTransform() const
	{
		return mChild2ParentMatrix;
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
		return mHasChanged;
	}

	bool CTransform::HasHierarchyChanged() const
	{
		return mPrevParentEntityId != mParentEntityId;
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
		mIsFirstFrameAfterCreation = false;
	}

	bool CTransform::IsFirstFrameAfterCreation() const
	{
		return mIsFirstFrameAfterCreation;
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