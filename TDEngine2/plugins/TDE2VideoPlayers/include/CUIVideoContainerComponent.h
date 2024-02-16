/*!
	\file CUIVideoContainerComponent.h
	\date 14.02.2024
	\authors Kasimov Ildar
*/

#pragma once


#include <core/memory/CPoolAllocator.h>
#include <ecs/CBaseComponent.h>
#include <editor/IEditorsManager.h>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CUIVideoContainerComponent's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIVideoContainerComponent's implementation
	*/

	TDE2_API IComponent* CreateUIVideoContainerComponent(E_RESULT_CODE& result);


	/*!
		class CUIVideoContainerComponent
	*/

	class CUIVideoContainerComponent: public CBaseComponent, public CPoolMemoryAllocPolicy<CUIVideoContainerComponent, 1 << 20>
	{
		public:
			friend TDE2_API IComponent* CreateUIVideoContainerComponent(E_RESULT_CODE&);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CUIVideoContainerComponent)

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

			/*!
				\brief The method creates a new deep copy of the instance and returns a smart pointer to it.
				The original state of the object stays the same

				\param[in] pDestObject A valid pointer to an object which the properties will be assigned into
			*/

			TDE2_API E_RESULT_CODE Clone(IComponent*& pDestObject) const override;

			TDE2_API void StartPlayback();
			TDE2_API void StopPlayback();

			TDE2_API void ResetState();

#if TDE2_EDITORS_ENABLED
			TDE2_API static void DrawInspectorGUI(const TEditorContext& context);
#endif
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIVideoContainerComponent)
		
		public:
			std::string mVideoResourceId;

			U16         mFPS = 30;

			bool        mIsLooped = false;
			bool        mIsAutoplayEnabled = true;
			bool        mIsDirty = true;

			bool        mIsPlaying = false;

			// control flags
			bool        mStopPlayback = false;
			

			F32         mCurrTime = 0.0f;
			U32         mFrameTime = 0; // equals to 1000.0f / mFPS
	};


	/*!
		struct TUIVideoContainerComponentParameters

		\brief The structure contains parameters for creation of CUIVideoContainerComponent
	*/

	typedef struct TUIVideoContainerComponentParameters : public TBaseComponentParameters
	{
	} TUIVideoContainerComponentParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(UIVideoContainerComponent, TUIVideoContainerComponentParameters);
}