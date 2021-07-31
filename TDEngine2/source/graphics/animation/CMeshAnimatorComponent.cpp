#include "../../include/graphics/animation/CMeshAnimatorComponent.h"
#include <tuple>


namespace TDEngine2
{
	static const std::string ComponentTypeName = "mesh_animator";

	const std::string CMeshAnimatorComponent::mPositionJointChannelPattern = "joint_{0}.position";
	const std::string CMeshAnimatorComponent::mRotationJointChannelPattern = "joint_{0}.rotation";

	const std::string& CMeshAnimatorComponent::GetPositionJointChannelPattern()
	{
		return mPositionJointChannelPattern;
	}

	const std::string& CMeshAnimatorComponent::GetRotationJointChannelPattern()
	{
		return mRotationJointChannelPattern;
	}

	const std::string& CMeshAnimatorComponent::GetComponentTypeName()
	{
		return ComponentTypeName;
	}

	CMeshAnimatorComponent::CMeshAnimatorComponent() :
		CBaseComponent()
	{
	}

	E_RESULT_CODE CMeshAnimatorComponent::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsDirty = true;
		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponent::Load(IArchiveReader* pReader)
	{
		if (!pReader)
		{
			return RC_FAIL;
		}



		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponent::Save(IArchiveWriter* pWriter)
	{
		if (!pWriter)
		{
			return RC_FAIL;
		}

		pWriter->BeginGroup("component");
		{
			pWriter->SetUInt32("type_id", static_cast<U32>(CMeshAnimatorComponent::GetTypeId()));
			
		}
		pWriter->EndGroup();

		return RC_OK;
	}

	void CMeshAnimatorComponent::SetDirtyFlag(bool value)
	{
		mIsDirty = value;
	}

	bool CMeshAnimatorComponent::IsDirty() const
	{
		return mIsDirty;
	}

	CMeshAnimatorComponent::TJointsMap& CMeshAnimatorComponent::GetJointsTable()
	{
		return mJointsTable;
	}

	CMeshAnimatorComponent::TJointPose& CMeshAnimatorComponent::GetCurrAnimationPose()
	{
		return mCurrAnimationPose;
	}

	const std::vector<TVector3>& CMeshAnimatorComponent::GetJointPositionsArray() const
	{
		return mJointsCurrPositions;
	}

	const std::vector<TQuaternion>& CMeshAnimatorComponent::GetJointRotationsArray() const
	{
		return mJointsCurrRotation;
	}


	enum class E_JOINT_PROPERTY_TYPE : U8
	{
		POSITION, ROTATION
	};


	static std::tuple<std::string, E_JOINT_PROPERTY_TYPE> GetJointInfoFromProperty(const std::string& propertyId)
	{
		auto p0 = propertyId.find_first_of('_') + 1;
		auto p1 = propertyId.find_first_of('.');

		return { propertyId.substr(p0, p1 - p0), propertyId.substr(p1 + 1) == "position" ? E_JOINT_PROPERTY_TYPE::POSITION : E_JOINT_PROPERTY_TYPE::ROTATION };
	}


	IPropertyWrapperPtr CMeshAnimatorComponent::GetProperty(const std::string& propertyName)
	{
		auto&& properties = GetAllProperties();
		if (std::find_if(properties.cbegin(), properties.cend(), [&propertyName](const std::string& id) { return propertyName == id; }) == properties.cend())
		{
			return CBaseComponent::GetProperty(propertyName);
		}

		std::string jointId;
		E_JOINT_PROPERTY_TYPE propertyType;

		std::tie(jointId, propertyType) = GetJointInfoFromProperty(propertyName);

		switch (propertyType)
		{
			case E_JOINT_PROPERTY_TYPE::POSITION:
				return IPropertyWrapperPtr(CBasePropertyWrapper<TVector3>::Create([this, jointId](const TVector3& pos) { _setPositionForJoint(jointId, pos); return RC_OK; }, nullptr));
			case E_JOINT_PROPERTY_TYPE::ROTATION:
				return IPropertyWrapperPtr(CBasePropertyWrapper<TQuaternion>::Create([this, jointId](const TQuaternion& rot) { _setRotationForJoint(jointId, rot); return RC_OK; }, nullptr));
		}

		return CBaseComponent::GetProperty(propertyName);
	}

	const std::vector<std::string>& CMeshAnimatorComponent::GetAllProperties() const
	{
		static std::vector<std::string> properties;

		if (properties.size() / 2 == mJointsTable.size())
		{
			return properties;
		}

		properties.clear();

		for (auto&& currJointEntity : mJointsTable)
		{
			properties.push_back(Wrench::StringUtils::Format(CMeshAnimatorComponent::mPositionJointChannelPattern, currJointEntity.first));
			properties.push_back(Wrench::StringUtils::Format(CMeshAnimatorComponent::mRotationJointChannelPattern, currJointEntity.first));
		}

		return properties;
	}

	const std::string& CMeshAnimatorComponent::GetTypeName() const
	{
		return ComponentTypeName;
	}

	void CMeshAnimatorComponent::_setPositionForJoint(const std::string& jointId, const TVector3& position)
	{
		auto it = mJointsTable.find(jointId);
		if (it == mJointsTable.cend())
		{
			TDE2_ASSERT(false);
			return;
		}

		if (static_cast<U32>(mJointsCurrPositions.size()) <= it->second)
		{
			mJointsCurrPositions.resize(it->second + 1);
		}

		mJointsCurrPositions[it->second] = position;

		mIsDirty = true;
	}

	void CMeshAnimatorComponent::_setRotationForJoint(const std::string& jointId, const TQuaternion& rotation)
	{
		auto it = mJointsTable.find(jointId);
		if (it == mJointsTable.cend())
		{
			TDE2_ASSERT(false);
			return;
		}

		if (static_cast<U32>(mJointsCurrRotation.size()) <= it->second)
		{
			mJointsCurrRotation.resize(it->second + 1);
		}

		mJointsCurrRotation[it->second] = rotation;

		mIsDirty = true;
	}


	IComponent* CreateMeshAnimatorComponent(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CMeshAnimatorComponent, result);
	}


	CMeshAnimatorComponentFactory::CMeshAnimatorComponentFactory() :
		CBaseObject()
	{
	}

	E_RESULT_CODE CMeshAnimatorComponentFactory::Init()
	{
		if (mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = true;

		return RC_OK;
	}

	E_RESULT_CODE CMeshAnimatorComponentFactory::Free()
	{
		if (!mIsInitialized)
		{
			return RC_FAIL;
		}

		mIsInitialized = false;

		delete this;

		return RC_OK;
	}

	IComponent* CMeshAnimatorComponentFactory::Create(const TBaseComponentParameters* pParams) const
	{
		if (!pParams)
		{
			return nullptr;
		}

		const TMeshAnimatorComponentParameters* pAnimatorParams = static_cast<const TMeshAnimatorComponentParameters*>(pParams);

		E_RESULT_CODE result = RC_OK;

		return CreateMeshAnimatorComponent(result);
	}

	IComponent* CMeshAnimatorComponentFactory::CreateDefault(const TBaseComponentParameters& params) const
	{
		E_RESULT_CODE result = RC_OK;

		return CreateMeshAnimatorComponent(result);
	}

	TypeId CMeshAnimatorComponentFactory::GetComponentTypeId() const
	{
		return CMeshAnimatorComponent::GetTypeId();
	}


	IComponentFactory* CreateMeshAnimatorComponentFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CMeshAnimatorComponentFactory, result);
	}
}