/*!
	/file IImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../math/TVector2.h"
#include "../math/TVector3.h"
#include "../math/TVector4.h"
#include "../utils/Color.h"
#include "../math/TRect.h"
#include "IEngineSubsystem.h"
#include <functional>
#include <tuple>


namespace TDEngine2
{
	class IWindowSystem;
	class IGraphicsObjectManager;
	class IInputContext;
	class CWin32WindowSystem;
	class CUnixWindowSystem;
	class IResourceManager;
	class IRenderer;
	struct TQuaternion;
	union TMatrix4;
	class CGradientColor;
	class IGraphicsContext;

	enum class E_GIZMO_TYPE : U8;


	TDE2_DECLARE_SCOPED_PTR(IWindowSystem)
	TDE2_DECLARE_SCOPED_PTR(IGraphicsContext)
	TDE2_DECLARE_SCOPED_PTR(IInputContext)
	TDE2_DECLARE_SCOPED_PTR(IResourceManager)
	TDE2_DECLARE_SCOPED_PTR(IRenderer)


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


	struct TImGUIContextInitParams
	{
		TPtr<IWindowSystem>     mpWindowSystem;
		TPtr<IRenderer>         mpRenderer;
		IGraphicsObjectManager* mpGraphicsObjectManager;
		TPtr<IResourceManager>  mpResourceManager;
		TPtr<IInputContext>     mpInputContext;
	};


	/*!
		interface IImGUIContext

		\brief The interface represents context of immediate mode GUI which is 
		responsible for implemnentation of all editor's stuffs
	*/

	class IImGUIContext : public IEngineSubsystem, public IImGUIContextVisitor
	{
		public:
			typedef struct TWindowParams
			{
				TVector2 mSizes    = ZeroVector2;
				TVector2 mMinSizes = ZeroVector2;
				TVector2 mMaxSizes = ZeroVector2;

				bool     mIsAutoResizeable = false;
				bool     mIsTransparent = false;
				bool     mIsInputTransparent = false;
				bool     mIsFullscreen = false;
			} TWindowParams, *TWindowParamsPtr;

			typedef struct TPlotGridParams
			{
				F32 mWidth;
				F32 mHeight;
				
				U8 mColsCount = 5;
				U8 mRowsCount = 5;

				TRectF32 mFrame;

				TColor32F mBackgroundColor = { 0.5f, 0.5f, 0.5f, 1.0f };
				TColor32F mAxesColor = { 0.4f, 0.4f, 0.4f, 1.0f };
				TColor32F mCurveColor = TColorUtils::mGreen;

				bool mIsGridEnabled = true;
				bool mIsBackgroundEnabled = true;
			} TPlotGridParams, *TPlotGridParamsPtr;

			typedef std::function<void()> TImGUIContextAction;
			template <typename T> using TImGUIContextParamAction = std::function<void(const T&)>;

		public:
			/*!
				\brief The method initializes an internal state of a context

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TImGUIContextInitParams& params) = 0;

			/*!
				\brief The method begins to populate immediage GUI state. Any UI element should be drawn during
				BeginFrame/EndFrame scope

				\param[in] dt Time elapsed from last frame was rendered
			*/

			TDE2_API virtual void BeginFrame(F32 dt) = 0;

			/*!
				\brief The method flushes current state and send all the data onto GPU to render it
			*/

			TDE2_API virtual void EndFrame() = 0;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
			*/

			TDE2_API virtual void Label(const std::string& text, bool useDisabledStyle = false) = 0;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
				\param[in] color Text's color
			*/

			TDE2_API virtual void Label(const std::string& text, const TColor32F& color) = 0;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
				\param[in] pos A position of a label
				\param[in] color Text's color
			*/

			TDE2_API virtual void Label(const std::string& text, const TVector2& pos, const TColor32F& color) = 0;

			/*!
				\brief The method creates a button with a given size and label

				\param[in] text A text that will be displayed on top of the button
				\param[in] sizes A vector that defines width and height of the button
				\param[in] onClicked A callback that's called when a user clicks over the menu item
				\param[in] makeInvisible If true the button won't be displayed, but still processes all clicks over it
				\param[in] allowOverlapping If true the button will allow to overlap itself by other input elements

				\return The method returns true if the button was pressed, false in other cases
			*/

			TDE2_API virtual bool Button(const std::string& text, const TVector2& sizes, const TImGUIContextAction& onClicked = {}, bool makeInvisible = false,
										 bool allowOverlapping = false) = 0;

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
											const TImGUIContextAction& onValueChanged = {}) = 0;

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
											  const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a float field which is a text box specialized for float single precision numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual void FloatField(const std::string& text, F32& value, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a integer field which is a text box specialized for integral numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual void IntField(const std::string& text, I32& value, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a text box

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] setFocus Determines whether or not the focus of a keyboard is set
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API virtual bool TextField(const std::string& text, std::string& value, const TImGUIContextParamAction<std::string>& onValueChanged = nullptr, 
											const TImGUIContextAction& onCancel = nullptr, bool setFocus = false) = 0;

			/*!
				\brief The method creates a field for TVector2 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector2 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void Vector2Field(const std::string& text, TVector2& value, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a field for TVector3 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector3 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/


			TDE2_API virtual void Vector3Field(const std::string& text, TVector3& value, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a field for TVector4 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector4 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void Vector4Field(const std::string& text, TVector4& value, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method displays a color picker for common RGBA color type 

				\param[in] text A text that will be displayed with the field
				\param[in, out] color A value of TColor32F type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API virtual void ColorPickerField(const std::string& text, TColor32F& color, const TImGUIContextAction& onValueChanged = {}) = 0;

			TDE2_API virtual void GradientColorPicker(const std::string& text, CGradientColor& color, const TImGUIContextAction& onValueChanged = {}) = 0;

			/*!
				\brief The method creates a main menu on top of the screen

				\param[in] onDrawCallback A callback within which a user defines sub-menus of the menu
			*/

			TDE2_API virtual void DisplayMainMenu(const std::function<void(IImGUIContext&)>& onDrawCallback) = 0;

			/*!
				\brief The method creates a context menu
				
				\param[in] onDrawCallback A callback within which a user defines sub-menus of the menu
			*/

			TDE2_API virtual void DisplayContextMenu(const std::string& id, const std::function<void(IImGUIContext&)>& onDrawCallback) = 0;

			/*!
				\brief The method creates a sub-menu

				\param[in] name A name of the sub-menu
				\param[in] onDrawCallback A callback within which a user defines menu items
			*/

			TDE2_API virtual void MenuGroup(const std::string& name, const std::function<void(IImGUIContext&)>& onDrawCallback) = 0;

			/*!
				\brief The method creates a new menu item, should be called within MenuGroup's onDrawCallback
				
				\param[in] name A name of the menu item
				\param[in] shortcut A shortcut for keyboard to activate the menu item
				\param[in] onClicked A callback that's called when a user clicks over the menu item

				\return The method returns true when a user clicks over the menu item
			*/

			TDE2_API virtual bool MenuItem(const std::string& name, const std::string& shortcut, const TImGUIContextAction& onClicked = {}) = 0;

			/*!
				\brief The method creates a histogram

				\param[in] name A name of the histogram
				\param[in] values An array of samples that should be drawn
				\param[in] minScale Minimal scale's value
				\param[in] maxScale Maximum scale's value
				\param[in] sizes A sizes of the displayed histogram
				\param[in] overlayedText Text that will be displayed over the histogram's samples
			*/

			TDE2_API virtual void Histogram(const std::string& name, const std::vector<F32>& values, F32 minScale, F32 maxScale,
											const TVector2& sizes = ZeroVector2, const std::string& overlayedText = "") = 0;

			/*!
				\brief The method creates a histogram

				\param[in] name A name of the histogram
				\param[in] pValues An array of samples that should be drawn
				\param[in] valuesCount A size of the array 
				\param[in] minScale Minimal scale's value
				\param[in] maxScale Maximum scale's value
				\param[in] sizes A sizes of the displayed histogram
				\param[in] overlayedText Text that will be displayed over the histogram's samples
			*/

			TDE2_API virtual void Histogram(const std::string& name, const F32* pValues, U32 valuesCount, F32 minScale, F32 maxScale,
											const TVector2& sizes = ZeroVector2, const std::string& overlayedText = "") = 0;

			/*!
				\brief The method draws a line within the current active window

				\param[in] start An initial position of the line
				\param[in] end Finish position of the line
				\param[in] color The line's color
				\param[in] thickness A line's thickness
			*/

			TDE2_API virtual void DrawLine(const TVector2& start, const TVector2& end, const TColor32F& color, F32 thickness = 1.0f) = 0;

			/*!
				\brief The method draws a cubic Bezier line
			*/

			TDE2_API virtual void DrawCubicBezier(const TVector2& p0, const TVector2& t0, const TVector2& p1, const TVector2& t1, const TColor32F& color, F32 thickness = 1.0f) = 0;

			/*!
				\brief The method draws a rectangle within the current active window

				\param[in] rect A rectangle's parameters
				\param[in] color The rectangle's color
			*/

			TDE2_API virtual void DrawRect(const TRectF32& rect, const TColor32F& color, bool isFilled = true, F32 thickness = 1.0f) = 0;

			TDE2_API virtual void DrawCircle(const TVector2& center, F32 radius, bool isFilled, const TColor32F& color, F32 thickness = 1.0f) = 0;

			TDE2_API virtual void DrawTriangle(const TVector2& p0, const TVector2& p1, const TVector2& p2, const TColor32F& color, bool isFilled = false, F32 thickness = 1.0f) = 0;

			/*!
				\brief The method draws given text
			*/

			TDE2_API virtual void DrawText(const TVector2& pos, const TColor32F& color, const std::string& text) = 0;

			/*!
				\brief The method draws manipulators for the transform based on ImGuizmo library

				\return The method returns true when the gizmo is actually being moving or selected
			*/

			TDE2_API virtual bool DrawGizmo(E_GIZMO_TYPE type, const TMatrix4& view, const TMatrix4& proj, const TMatrix4& transform,
											const std::function<void(const TVector3&, const TQuaternion&, const TVector3)>& onUpdate) = 0;

			/*!
				\brief The method displays a combo box with a set of items

				\param[in] label A caption of the combo box
				\param[in] currSelectedItem An index of the item that is selected by default
				\param[in] items A list of available items

				\return The method returns an index of the selected item
			*/

			TDE2_API virtual I32 Popup(const std::string& label, I32 currSelectedItem, const std::vector<std::string>& items) = 0;

			/*!
				\brief The method displays an image with the given parameters set

				\param[in] textureHandle A pointer to resource handler that corresponds to texture that should be displayed
				\param[in] sizes A sizes of displayed image
				\param[in] uvRect A uv coordinates of displayed texture
			*/

			TDE2_API virtual void Image(TResourceId textureHandle, const TVector2& sizes, const TRectF32& uvRect = { 0.0f, 0.0f, 1.0f, 1.0f }) = 0;

			/*!
				\brief The method displays selectable label item

				\param[in] id A label of an item
				\param[in] isSelected A flag determines whether or not the current node is highlighted
				\param[in] shouldClosePopup A flag defines whether the selectable's input events cause closing of the pop up window

				\return Returns true if the current item was selected, false in other cases
			*/

			TDE2_API virtual bool SelectableItem(const std::string& id, bool isSelected = false, bool shouldClosePopup = true) = 0;
			TDE2_API virtual bool SelectableItem(const std::string& id, const TColor32F& color, bool isSelected = false, bool shouldClosePopup = true) = 0;

			/*!
				\brief The method display tree's element. Always should be called in pair with EndTreeNode at end

				\param[in] id A label of a node
				\param[in] isSelected A flag determines whether or not the current node is highlighted

				\return A tuple the first argument is opened/hidden state of a node, the second is whether selected or not the node
			*/

			TDE2_API virtual std::tuple<bool, bool> BeginTreeNode(const std::string& id, const TColor32F& color = TColorUtils::mWhite, bool isSelected = false) = 0;

			/*!
				\brief The method should be called after BeginTreeNode
			*/

			TDE2_API virtual void EndTreeNode() = 0;

			/*!
				\brief The method displays header which looks same as BeginTreeNode. 

				\param[in] id A label that will be displayed
				\param[in] isOpened The flag defines whether or not the group is unwinded or not
				\param[in] isSelected The flag defines whether the header be highlighted or not
				\param[in] itemClicked The callback is called when a user clicks over the header
				\param[in] contentAction You can invoke any ImGUIContext command there which should be displayed when the header is unwinded

				\return The method return true if it was unwinded, false in other cases
			*/

			TDE2_API virtual bool CollapsingHeader(const std::string& id, bool isOpened, bool isSelected = false, 
												   const TImGUIContextAction& itemClicked = nullptr, 
												   const TImGUIContextAction& contentAction = nullptr) = 0;

			/*!
				\brief The method creates a new window on the screen. Every call after this one
				will be related with this window

				\param[in] name A window's name
				\param[in, out] isOpened A flag that determines whether the window is shown or not
				\param[in] params A set of additonal parameters which can be applied to the window

				\return The method returns false if the window is collapsed or not visible
			*/

			TDE2_API virtual bool BeginWindow(const std::string& name, bool& isOpened, const TWindowParams& params) = 0;

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

			/*!
				\brief The method creates child window's region within the current one

				\return The method returns true if the window is shown
			*/

			TDE2_API virtual bool BeginChildWindow(const std::string& name, const TVector2& sizes) = 0;

			/*!
				\brief The method finalizes current child window
			*/

			TDE2_API virtual void EndChildWindow() = 0;

			TDE2_API virtual void ShowModalWindow(const std::string& name) = 0;
			TDE2_API virtual void CloseCurrentModalWindow() = 0;

			TDE2_API virtual bool BeginModalWindow(const std::string& name, bool isNonBlocking = false) = 0;

			TDE2_API virtual void EndModalWindow() = 0;

			/*!
				\brief The method draws a plot's grid with input handling, scaling, moving, etc
			*/

			TDE2_API virtual void DrawPlotGrid(const std::string& name, const TPlotGridParams& params, const std::function<void(const TVector2&)>& onGridCallback = {}) = 0;

			TDE2_API virtual void Tooltip(const std::string& message) = 0;

			TDE2_API virtual void VerticalSeparator(F32 initialLeftColumnWidth, const std::function<void(F32)>& leftRegionCallback, const std::function<void(F32)>& rightRegionCallback) = 0;

			/*!
				\brief The method moves current position of the cursor to given point
			*/

			TDE2_API virtual void SetCursorScreenPos(const TVector2& pos) = 0;

			/*!
				\brief The method changes width of items that are called within action callbacks
			*/

			TDE2_API virtual void SetItemWidth(F32 width, const TImGUIContextAction& action = {}) = 0;

			TDE2_API virtual void DisplayIDGroup(I32 id, const TImGUIContextAction& idGroupCallback = {}) = 0;

			/*!
				\brief The method provides implementation of drag & drop source

				\param[in] action A callback in which all the stuff for register should be executed
			*/

			TDE2_API virtual void RegisterDragAndDropSource(const TImGUIContextAction& action = nullptr) = 0;

			/*!
				\brief The method provides implementation of drag & drop target

				\param[in] action A callback in which all the stuff for register should be executed
			*/

			TDE2_API virtual void RegisterDragAndDropTarget(const TImGUIContextAction& action = nullptr) = 0;

			template <typename T>
			TDE2_API void SetDragAndDropData(const std::string& id, const T& data)
			{
				_setDragAndDropData(id, static_cast<const void*>(&data), sizeof(T));
			}

			template <typename T>
			TDE2_API const T* GetDragAndDropData(const std::string& id) const
			{
				return static_cast<const T*>(_getDragAndDropData(id));
			}

			/*!
				\brief The method returns cursor position in a viewport space. The method is useful when
				you work with DrawX methods

				\return The method returns cursor position in a viewport space
			*/

			TDE2_API virtual TVector2 GetCursorScreenPos() const = 0;

			/*!
				\brief The method returns a width of current active window

				\return The method returns a width of current active window
			*/

			TDE2_API virtual F32 GetWindowWidth() const = 0;
			
			/*!
				\brief The method returns a height of current active window

				\return The method returns a height of current active window
			*/

			TDE2_API virtual F32 GetWindowHeight() const = 0;

			/*!
				\brief The method returns sizes which a given text string occupies

				\param[in] text A text string

				\return The static function returns sizes which a given text string occupies
			*/

			TDE2_API virtual TVector2 GetTextSizes(const std::string& text) const = 0;

			TDE2_API virtual bool IsItemActive() const = 0;

			/*!
				\return The method returns true when the cursor is over some ImGUI element
			*/

			TDE2_API virtual bool IsMouseOverUI() const = 0;

			TDE2_API virtual bool IsItemHovered() const = 0;

			TDE2_API virtual bool IsMouseDoubleClicked(U8 buttonId) const = 0;

			TDE2_API virtual bool IsMouseClicked(U8 buttonId) const = 0;

			TDE2_API virtual bool IsMouseReleased(U8 buttonId) const = 0;

			TDE2_API virtual bool IsMouseDragging(U8 buttonId) const = 0;

			TDE2_API virtual TVector2 GetMousePosition() const = 0;
			TDE2_API virtual TVector2 GetInvertedMousePosition() const = 0;

			TDE2_API virtual TVector2 GetMouseDragDelta(U8 buttonId) const = 0;

			TDE2_API virtual TVector2 GetScrollPosition() const = 0;

			TDE2_API static E_ENGINE_SUBSYSTEM_TYPE GetTypeID() { return EST_IMGUI_CONTEXT; }
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IImGUIContext)

			TDE2_API virtual void _setDragAndDropData(const std::string& id, const void* pData, U32 size) = 0;
			TDE2_API virtual const void* _getDragAndDropData(const std::string& id) const = 0;
	};


	template <> inline void IImGUIContext::SetDragAndDropData<std::string>(const std::string& id, const std::string& data)
	{
		_setDragAndDropData(id, data.c_str(), static_cast<U32>(data.size() * sizeof(std::string::value_type)));
	}


	TDE2_DECLARE_SCOPED_PTR_INLINED(IImGUIContext)
}