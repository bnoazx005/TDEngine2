/*!
	\file CMeshAnimatorComponent.h
	\date 18.07.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/IComponentFactory.h"
#include "../../math/TMatrix4.h"


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
			typedef std::vector<TMatrix4> TJointPose;
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

			TJointPose& GetCurrAnimationPose();
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMeshAnimatorComponent)
		protected:
			TJointPose mCurrAnimationPose;
	};


	/*!
		interface IMeshAnimatorComponentFactory

		\brief The interface represents a functionality of a factory of IMeshAnimatorComponent objects
	*/

	class IMeshAnimatorComponentFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMeshAnimatorComponentFactory)
	};


	/*!
		\brief A factory function for creation objects of CMeshAnimatorComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CMeshAnimatorComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateMeshAnimatorComponentFactory(E_RESULT_CODE& result);


	/*!
		class CMeshAnimatorComponentFactory

		\brief The interface represents a functionality of a factory of IMeshAnimatorComponent objects
	*/

	class CMeshAnimatorComponentFactory : public IMeshAnimatorComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateMeshAnimatorComponentFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CMeshAnimatorComponentFactory)
	};
}
