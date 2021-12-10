/*!
	\file CMeshAnimatorComponent.h
	\date 18.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/IComponentFactory.h"
#include "../../math/TMatrix4.h"
#include "../../math/TVector3.h"
#include "../../math/TQuaternion.h"
#include <unordered_map>
#include <vector>


namespace TDEngine2
{
	/*!
		struct TMeshAnimatorComponentParameters

		\brief The structure contains parameters for creation of an animation container's component
	*/

	typedef struct TMeshAnimatorComponentParameters : public TBaseComponentParameters
	{
	} TMeshAnimatorComponentParameters;


	/*!
		\brief A factory function for creation objects of CMeshAnimatorComponent's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMeshAnimatorComponent's implementation
	*/

	TDE2_API IComponent* CreateMeshAnimatorComponent(E_RESULT_CODE& result);


	/*!
		class CMeshAnimatorComponent

		\brief The class represents animator system for skinned meshes
	*/

	class CMeshAnimatorComponent : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateMeshAnimatorComponent(E_RESULT_CODE&);
		public:
			TDE2_API static const std::string mPositionJointChannelPattern;
			TDE2_API static const std::string mRotationJointChannelPattern;

			typedef std::vector<TMatrix4> TJointPose;
			typedef std::unordered_map<std::string, U32> TJointsMap;
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CMeshAnimatorComponent)

			/*!
				\brief The method initializes an internal state of the light

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			void SetDirtyFlag(bool value);

			bool IsDirty() const;

			TJointsMap& GetJointsTable();

			TJointPose& GetCurrAnimationPose();

			const std::vector<TVector3>& GetJointPositionsArray() const;
			const std::vector<TQuaternion>& GetJointRotationsArray() const;

			/*!
				\return The method returns a pointer to a type's property if the latter does exist or null pointer in other cases
			*/

			TDE2_API IPropertyWrapperPtr GetProperty(const std::string& propertyName) override;

			/*!
				\brief The method returns an array of properties names that are available for usage
			*/

			TDE2_API const std::vector<std::string>& GetAllProperties() const override;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

			TDE2_API static const std::string& GetPositionJointChannelPattern();
			TDE2_API static const std::string& GetRotationJointChannelPattern();
			TDE2_API static const std::string& GetComponentTypeName();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMeshAnimatorComponent)

			TDE2_API void _setPositionForJoint(const std::string& jointId, const TVector3& position);
			TDE2_API void _setRotationForJoint(const std::string& jointId, const TQuaternion& rotation);
		protected:
			TJointPose               mCurrAnimationPose;
			TJointsMap               mJointsTable;

			std::vector<TVector3>    mJointsCurrPositions;
			std::vector<TQuaternion> mJointsCurrRotation;

			bool                     mIsDirty;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(MeshAnimatorComponent, TMeshAnimatorComponentParameters);
}
