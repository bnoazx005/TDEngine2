/*!
	\file CSplashScreenItemComponent.h
	\date 19.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../core/memory/CPoolAllocator.h"
#include "../../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TSplashScreenComponentData
	{
		FIELD_META(name = elapsed_time) F32 mElapsedTime = 0.0f;
		FIELD_META(name = duration) F32     mDuration = 1.0f;

		TDE2_DECLARE_COMPONENT_META(TSplashScreenComponentData);
	};


	/*!
		\brief A factory function for creation objects of CSplashScreenItemComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IComponent's implementation
	*/

	TDE2_API IComponent* CreateSplashScreenItemComponent(E_RESULT_CODE& result);


	class CSplashScreenItemComponent : public CBaseComponentT<CSplashScreenItemComponent, TSplashScreenComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateSplashScreenItemComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CSplashScreenItemComponent)

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSplashScreenItemComponent)
	};


	typedef struct TSplashScreenItemComponentParameters : public TBaseComponentParameters
	{
	} TSplashScreenItemComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(SplashScreenItemComponent, TSplashScreenItemComponentParameters);


	TDE2_DECLARE_FLAG_COMPONENT(SplashScreenContainerRoot);
}