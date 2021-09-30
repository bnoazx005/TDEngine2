/*!
	\file AudioComponents.h
	\date 07.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../ecs/CBaseComponent.h"
#include <string>


namespace TDEngine2
{
	TDE2_DECLARE_FLAG_COMPONENT(AudioListenerComponent);


	/*!
		struct TAudioSourceComponentParameters

		\brief The structure contains parameters for creation of CAudioSourceComponent
	*/

	typedef struct TAudioSourceComponentParameters : public TBaseComponentParameters
	{
		std::string mAudioClipId;

		bool mIsMuted = false;
		bool mIsPaused = false;
		bool mIsLooped = false;

		F32 mVolume = 1.0f;
		F32 mPanning = 0.0f;
	} TAudioSourceComponentParameters;


	/*!
		\brief A factory function for creation objects of CAudioSourceComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAudioSourceComponent's implementation
	*/

	TDE2_API IComponent* CreateAudioSourceComponent(E_RESULT_CODE& result);


	/*!
		class CAudioSourceComponent

		\brief The class represents a component of a 3D audio source 
	*/

	class CAudioSourceComponent : public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateAudioSourceComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CAudioSourceComponent)

			/*!
				\brief The method initializes an internal state

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

			TDE2_API E_RESULT_CODE SetAudioClipId(const std::string& id);

			TDE2_API void SetMuted(bool value);
			TDE2_API void SetPaused(bool value);
			TDE2_API void SetLooped(bool value);
			TDE2_API void SetPlaying(bool value);

			TDE2_API E_RESULT_CODE SetVolume(F32 value);
			TDE2_API E_RESULT_CODE SetPanning(F32 value);

			TDE2_API const std::string& GetAudioClipId() const;

			TDE2_API bool IsMuted() const;
			TDE2_API bool IsPaused() const;
			TDE2_API bool IsLooped() const;
			TDE2_API bool IsPlaying() const;

			TDE2_API F32 GetVolume() const;
			TDE2_API F32 GetPanning() const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAudioSourceComponent)
		protected:
			std::string mAudioClipId;

			bool mIsMuted;
			bool mIsPaused;
			bool mIsLooped;
			bool mIsPlaying;

			F32 mVolume;
			F32 mPanning;
	};


	/*!
		\brief A factory function for creation objects of CAudioSourceComponentFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAudioSourceComponentFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateAudioSourceComponentFactory(E_RESULT_CODE& result);


	/*!
		class CAudioSourceComponentFactory

		\brief The class is factory facility to create a new objects of CAudioSourceComponent type
	*/

	class CAudioSourceComponentFactory : public IComponentFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateAudioSourceComponentFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAudioSourceComponentFactory)
	};
}