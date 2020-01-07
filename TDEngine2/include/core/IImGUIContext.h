/*!
	/file IImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "./../math/TVector2.h"
#include "./../math/TVector3.h"
#include "./../math/TVector4.h"
#include "IEngineSubsystem.h"
#include <functional>


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IInputContext;
	class CWin32WindowSystem;
	class CUnixWindowSystem;
	class IResourceManager;
	class IRenderer;


	/*!
		interface IImGUIContextVisitor

		\brief The interface describes functionality of a visitor which is used
		to proper configure of IImGUIContext object
	*/

	class IImGUIContextVisitor
	{
		public:
			/*!
				\brief The method configures the immediate GUI context for WIN32 platform

				\param[in] pWindowSystem A pointer to CWin32WindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem) = 0;
			
			/*!
				\brief The method configures the immediate GUI context for UNIX platform

				\param[in] pWindowSystem A pointer to CUnixWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE ConfigureForUnixPlatform(const CUnixWindowSystem* pWindowSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContextVisitor)
	};


	/*!
		interface IImGUIContext

		\brief The interface represents context of immediate mode GUI which is 
		responsible for implemnentation of all editor's stuffs
	*/

	class IImGUIContext : public IEngineSubsystem, public IImGUIContextVisitor
	{
		public:
			/*!
				\brief The method initializes an internal state of a context

				\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
				\param[in, out] pRenderer A pointer to IRenderer implementation
				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
				\param[in, out] pResourceManager A pointer to IResourceManager implementation
				\param[in, out] pInputContext A pointer to IInputContext implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
												IResourceManager* pResourceManager, IInputContext* pInputContext) = 0;

			/*!
				\brief The method begins to populate immediage GUI state. Any UI element should be drawn during
				BeginFrame/EndFrame scope

				\param[in] dt Time elapsed from last frame was rendered
			*/

			TDE2_API virtual void BeginFrame(float dt) = 0;

			/*!
				\brief The method flushes current state and send all the data onto GPU to render it
			*/

			TDE2_API virtual void EndFrame() = 0;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
			*/

			TDE2_API virtual void Label(const std::string& text) = 0;

			/*!
				\brief The method creates a button with a given size and label

				\param[in] text A text that will be displayed on top of the button
				\param[in] sizes A vector that defines width and height of the button

				\return The method returns true if the button was pressed, false in other cases
			*/

			TDE2_API virtual bool Button(const std::string& text, const TVector2& sizes) = 0;

			/*!
				\brief The method creates a check box 

				\param[in] text A text that will be displayed on top of the check box
				\param[in, out] isSelected A flag that is set up to true if the check box is selected

				\return 
			*/

			TDE2_API virtual bool Checkbox(const std::string& text, bool& isSelected) = 0;

			/*!
				\brief The method creates a slider for integers. If a value's changed onValueChanged
				callback is called

				\param[in] text  A text that will be displayed with the slider
				\param[in, out] value The current value for the slider
				\param[in] minValue A minimal value that can be assigned with this slider
				\param[in] maxValue A maximal value that can be assigned with this slider
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void IntSlider(const std::string& text, I32& value, I32 minValue, I32 maxValue, 
											const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a slider for floating point values. If a value's changed onValueChanged
				callback is called

				\param[in] text  A text that will be displayed with the slider
				\param[in, out] value The current value for the slider
				\param[in] minValue A minimal value that can be assigned with this slider
				\param[in] maxValue A maximal value that can be assigned with this slider
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void FloatSlider(const std::string& text, F32& value, F32 minValue, F32 maxValue,
											  const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a float field which is a text box specialized for float single precision numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual void FloatField(const std::string& text, F32& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a integer field which is a text box specialized for integral numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual void IntField(const std::string& text, I32& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a text box

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual void TextField(const std::string& text, std::string& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a field for TVector2 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector2 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void Vector2Field(const std::string& text, TVector2& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a field for TVector3 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector3 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/


			TDE2_API virtual void Vector3Field(const std::string& text, TVector3& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a field for TVector4 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector4 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void Vector4Field(const std::string& text, TVector4& value, const std::function<void()>& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a new window on the screen. Every call after this one
				will be related with this window

				\param[in] name A window's name
				\param[in, out] isOpened A flag that determines whether the window is shown or not

				\return The method returns false if the window is collapsed or not visible
			*/

			TDE2_API virtual bool BeginWindow(const std::string& name, bool& isOpened) = 0;

			/*!
				\brief The method finishes populating of current window
			*/

			TDE2_API virtual void EndWindow() = 0;

			/*!
				\brief The method starts horizontal group's layout. All elements that are
				rendered between BeginHorizontal/EndHorizontal will be placed in the following
				way 
				EL1 ... ELN
				instead of 
				EL1
				...
				ELN
			*/

			TDE2_API virtual void BeginHorizontal() = 0;

			/*!
				\brief The method closes a horizontal group
			*/

			TDE2_API virtual void EndHorizontal() = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_IMGUI_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContext)
	};
}