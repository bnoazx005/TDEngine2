/*!
	\file CAnimationContainerComponent.h
	\date 30.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include "../../ecs/IComponentFactory.h"
#include "../../core/Meta.h"
#include "../../core/memory/CPoolAllocator.h"
#include <unordered_map>


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

	class CAnimationContainerComponent : public CBaseComponent, public CPoolMemoryAllocPolicy<CAnimationContainerComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateAnimationContainerComponent(E_RESULT_CODE&);

		public:
			typedef std::unordered_map<U32, IPropertyWrapperPtr> TPropertiesTable;

		public:
			TDE2_REGISTER_COMPONENT_TYPE(CAnimationContainerComponent)

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

			TDE2_API E_RESULT_CODE Play();

			TDE2_API E_RESULT_CODE SetAnimationClipId(const std::string& id);
			TDE2_API E_RESULT_CODE SetAnimationClipResourceId(const TResourceId& resourceId);

			TDE2_API void SetStartedFlag(bool value);
			TDE2_API void SetPlayingFlag(bool value);
			TDE2_API void SetStoppedFlag(bool value);
			TDE2_API void SetPausedFlag(bool value);

			TDE2_API E_RESULT_CODE SetTime(F32 value);
			TDE2_API E_RESULT_CODE SetDuration(F32 value);

			TDE2_API F32 GetTime() const;
			TDE2_API F32 GetDuration() const;

			TDE2_API bool IsStarted() const;
			TDE2_API bool IsPlaying() const;
			TDE2_API bool IsStopped() const;
			TDE2_API bool IsPaused() const;

			TDE2_API TPropertiesTable& GetCachedPropertiesTable();

			TDE2_API const std::string& GetAnimationClipId() const;

			/*!
				\return The method returns type name (lowercase is preffered)
			*/

			TDE2_API const std::string& GetTypeName() const override;

			TDE2_API TResourceId GetAnimationClipResourceId() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimationContainerComponent)
		protected:
			std::string mAnimationClipId;

			bool mIsStarted = false;
			bool mIsPlaying = false;
			bool mIsStopped = false;
			bool mIsPaused = false;

			F32 mCurrTime;
			F32 mDuration;

			TResourceId mAnimationClipResourceId = TResourceId::Invalid;

			TPropertiesTable mCachedProperties;
	};


	TDE2_DECLARE_COMPONENT_FACTORY(AnimationContainerComponent, TAnimationContainerComponentParameters);
}
