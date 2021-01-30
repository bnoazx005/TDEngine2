/*!
	\file CAnimationContainerComponent.h
	\date 30.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		struct TAnimationContainerComponentParameters

		\brief The structure contains parameters for creation of an animation container's component
	*/

	typedef struct TAnimationContainerComponentParameters : public TBaseComponentParameters
	{
	} TAnimationContainerComponentParameters;


	/*!
		\brief A factory function for creation objects of CAnimationContainerComponent's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationContainerComponent's implementation
	*/

	TDE2_API IComponent* CreateAnimationContainerComponent(E_RESULT_CODE& result);


	/*!
		class CAnimationContainerComponent

		\brief The class represents directed light source which is an analogue of sun light
	*/

	class CAnimationContainerComponent : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateAnimationContainerComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CAnimationContainerComponent)

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

			TDE2_API E_RESULT_CODE SetAnimationClipId(const std::string& id);
			TDE2_API E_RESULT_CODE SetAnimationClipResourceId(const TResourceId& resourceId);

			TDE2_API E_RESULT_CODE SetTime(F32 value);

			TDE2_API F32 GetTime() const;

			TDE2_API const std::string& GetAnimationClipId() const;

			TDE2_API TResourceId GetAnimationClipResourceId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationContainerComponent)
		protected:
			std::string mAnimationClipId;

			F32 mCurrTime;

			TResourceId mAnimationClipResourceId = TResourceId::Invalid;
	};


	/*!
		interface IAnimationContainerComponentFactory

		\brief The interface represents a functionality of a factory of IAnimationContainerComponent objects
	*/

	class IAnimationContainerComponentFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimationContainerComponentFactory)
	};


	/*!
		\brief A factory function for creation objects of CAnimationContainerComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationContainerComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateAnimationContainerComponentFactory(E_RESULT_CODE& result);


	/*!
		class CAnimationContainerComponentFactory

		\brief The interface represents a functionality of a factory of IAnimationContainerComponent objects
	*/

	class CAnimationContainerComponentFactory : public IAnimationContainerComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateAnimationContainerComponentFactory(E_RESULT_CODE& result);
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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationContainerComponentFactory)
	};
}
