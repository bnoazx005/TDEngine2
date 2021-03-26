/*!
	/file CImGUIContext.h
	/date 01.12.2019
	/authors Kasimov Ildar
*/

#pragma once


#include <core/IImGUIContext.h>
#include <math/TVector2.h>
#include <math/TVector4.h>
#include <math/TMatrix4.h>
#include <utils/Color.h>
#include <stack>


struct ImGuiIO;
struct ImDrawData;
struct ImDrawList;


namespace TDEngine2
{
	class IGraphicsContext;
	class IVertexBuffer;
	class IIndexBuffer;
	class CRenderQueue;
	class IVertexDeclaration;

	enum class TMaterialInstanceId : U32;


	/*!
		\brief A factory function for creation objects of CImGUIContext's type

		\param[in, out] pWindowSystem A pointer to IWindowSystem implementation
		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[in, out] pResourceManager A pointer to IResourceManager implementation
		\param[in, out] pInputContext A pointer to IInputContext implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CImGUIContext's implementation
	*/

	TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
											   IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);


	/*!
		class CImGUIContext

		\brief The class is an implementation of an immediate mode GUI for editor's stuffs
	*/

	class CImGUIContext : public IImGUIContext
	{
		public:
			friend TDE2_API IImGUIContext* CreateImGUIContext(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
															  IResourceManager* pResourceManager, IInputContext* pInputContext, E_RESULT_CODE& result);
		public:
			typedef std::stack<ImDrawList*> TDrawListsStack;
			typedef std::vector<TResourceId> TResourceHandlesArray;

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

			TDE2_API E_RESULT_CODE Init(IWindowSystem* pWindowSystem, IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager,
										IResourceManager* pResourceManager, IInputContext* pInputContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method configures the immediate GUI context for WIN32 platform

				\param[in] pWindowSystem A pointer to CWin32WindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureForWin32Platform(const CWin32WindowSystem* pWindowSystem);

			/*!
				\brief The method configures the immediate GUI context for UNIX platform

				\param[in] pWindowSystem A pointer to CUnixWindowSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE ConfigureForUnixPlatform(const CUnixWindowSystem* pWindowSystem);
			
			/*!
				\brief The method begins to populate immediage GUI state. Any UI element should be drawn during
				BeginFrame/EndFrame scope

				\param[in] dt Time elapsed from last frame was rendered
			*/

			TDE2_API void BeginFrame(float dt) override;

			/*!
				\brief The method flushes current state and send all the data onto GPU to render it
			*/

			TDE2_API void EndFrame() override;

			/*!
				\brief The method returns a type of the subsystem

				\return A type, which is represented with E_ENGINE_SUBSYSTEM_TYPE's value
			*/

			TDE2_API E_ENGINE_SUBSYSTEM_TYPE GetType() const override;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
			*/

			TDE2_API void Label(const std::string& text) override;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
				\param[in] color Text's color
			*/

			TDE2_API void Label(const std::string& text, const TColor32F& color) override;

			/*!
				\brief The method creates a label within current active window

				\param[in] text A string which should be rendered onto the screen
				\param[in] pos A position of a label
				\param[in] color Text's color
			*/

			TDE2_API void Label(const std::string& text, const TVector2& pos, const TColor32F& color) override;

			/*!
				\brief The method creates a button with a given size and label

				\param[in] text A text that will be displayed on top of the button
				\param[in] sizes A vector that defines width and height of the button
				\param[in] onClicked A callback that's called when a user clicks over the menu item
				\param[in] makeInvisible If true the button won't be displayed, but still processes all clicks over it

				\return The method returns true if the button was pressed, false in other cases
			*/

			TDE2_API bool Button(const std::string& text, const TVector2& sizes, const std::function<void()>& onClicked = {}, bool makeInvisible = false) override;

			/*!
				\brief The method creates a check box

				\param[in] text A text that will be displayed on top of the check box
				\param[in, out] isSelected A flag that is set up to true if the check box is selected

				\return
			*/

			TDE2_API bool Checkbox(const std::string& text, bool& isSelected) override;

			/*!
				\brief The method creates a slider for integers. If a value's changed onValueChanged
				callback is called

				\param[in] text  A text that will be displayed with the slider
				\param[in, out] value The current value for the slider
				\param[in] minValue A minimal value that can be assigned with this slider
				\param[in] maxValue A maximal value that can be assigned with this slider
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API void IntSlider(const std::string& text, I32& value, I32 minValue, I32 maxValue,
									const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a slider for floating point values. If a value's changed onValueChanged
				callback is called

				\param[in] text  A text that will be displayed with the slider
				\param[in, out] value The current value for the slider
				\param[in] minValue A minimal value that can be assigned with this slider
				\param[in] maxValue A maximal value that can be assigned with this slider
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API void FloatSlider(const std::string& text, F32& value, F32 minValue, F32 maxValue,
									  const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a float field which is a text box specialized for float single precision numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API void FloatField(const std::string& text, F32& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a integer field which is a text box specialized for integral numbers

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API void IntField(const std::string& text, I32& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a text box

				\param[in] text A text that will be displayed with this text box
				\param[in] value The current and output value for the text box
				\param[in] onValueChanged A callback that is called when the value of the field has changed
			*/

			TDE2_API void TextField(const std::string& text, std::string& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a field for TVector2 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector2 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API void Vector2Field(const std::string& text, TVector2& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a field for TVector3 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector3 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/


			TDE2_API void Vector3Field(const std::string& text, TVector3& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a field for TVector4 type

				\param[in] text A text that will be displayed with the field
				\param[in, out] value A value of TVector4 type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API void Vector4Field(const std::string& text, TVector4& value, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method displays a color picker for common RGBA color type

				\param[in] text A text that will be displayed with the field
				\param[in, out] color A value of TColor32F type
				\param[in] onValueChanged A callback that is called when the value of the slider has changed
			*/

			TDE2_API void ColorPickerField(const std::string& text, TColor32F& color, const std::function<void()>& onValueChanged = {}) override;

			/*!
				\brief The method creates a main menu on top of the screen

				\param[in] onDrawCallback A callback within which a user defines sub-menus of the menu
			*/

			TDE2_API void DisplayMainMenu(const std::function<void(IImGUIContext&)>& onDrawCallback) override;

			/*!
				\brief The method creates a context menu

				\param[in] onDrawCallback A callback within which a user defines sub-menus of the menu
			*/

			TDE2_API void DisplayContextMenu(const std::string& id, const std::function<void(IImGUIContext&)>& onDrawCallback) override;

			/*!
				\brief The method creates a sub-menu

				\param[in] name A name of the sub-menu
				\param[in] onDrawCallback A callback within which a user defines menu items
			*/

			TDE2_API void MenuGroup(const std::string& name, const std::function<void(IImGUIContext&)>& onDrawCallback) override;

			/*!
				\brief The method creates a new menu item, should be called within MenuGroup's onDrawCallback

				\param[in] name A name of the menu item
				\param[in] shortcut A shortcut for keyboard to activate the menu item
				\param[in] onClicked A callback that's called when a user clicks over the menu item

				\return The method returns true when a user clicks over the menu item
			*/

			TDE2_API bool MenuItem(const std::string& name, const std::string& shortcut, const std::function<void()>& onClicked = {}) override;

			/*!
				\brief The method creates a histogram

				\param[in] name A name of the histogram
				\param[in] values An array of samples that should be drawn
				\param[in] minScale Minimal scale's value
				\param[in] maxScale Maximum scale's value
				\param[in] sizes A sizes of the displayed histogram
				\param[in] overlayedText Text that will be displayed over the histogram's samples
			*/

			TDE2_API void Histogram(const std::string& name, const std::vector<F32>& values, F32 minScale, F32 maxScale,
									const TVector2& sizes = ZeroVector2, const std::string& overlayedText = "") override;

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

			TDE2_API void Histogram(const std::string& name, const F32* pValues, U32 valuesCount, F32 minScale, F32 maxScale,
									const TVector2& sizes = ZeroVector2, const std::string& overlayedText = "") override;
			
			/*!
				\brief The method draws a line within the current active window

				\param[in] start An initial position of the line
				\param[in] end Finish position of the line
				\param[in] color The line's color
				\param[in] thickness A line's thickness
			*/

			TDE2_API void DrawLine(const TVector2& start, const TVector2& end, const TColor32F& color, F32 thickness = 1.0f) override;

			/*!
				\brief The method draws a cubic Bezier line
			*/

			TDE2_API void DrawCubicBezier(const TVector2& p0, const TVector2& t0, const TVector2& p1, const TVector2& t1, const TColor32F& color, F32 thickness = 1.0f) override;

			/*!
				\brief The method draws a rectangle within the current active window

				\param[in] rect A rectangle's parameters
				\param[in] color The rectangle's color
			*/

			TDE2_API void DrawRect(const TRectF32& rect, const TColor32F& color) override;

			TDE2_API void DrawCircle(const TVector2& center, F32 radius, bool isFilled, const TColor32F& color, F32 thickness = 1.0f) override;

			/*!
				\brief The method draws given text
			*/

			TDE2_API void DrawText(const TVector2& pos, const TColor32F& color, const std::string& text) override;

			/*!
				\brief The method draws manipulators for the transform based on ImGuizmo library

				\return The method returns true when the gizmo is actually being moving or selected
			*/

			TDE2_API bool DrawGizmo(E_GIZMO_TYPE type, const TMatrix4& view, const TMatrix4& proj, const TMatrix4& transform,
									const std::function<void(const TVector3&, const TQuaternion&, const TVector3)>& onUpdate) override;

			/*!
				\brief The method draws a plot's grid with input handling, scaling, moving, etc
			*/

			TDE2_API void DrawPlotGrid(const std::string& name, const TPlotGridParams& params, const std::function<void(const TVector2&)>& onGridCallback = {}) override;

			/*!
				\brief The method displays a combo box with a set of items

				\param[in] label A caption of the combo box
				\param[in] currSelectedItem An index of the item that is selected by default
				\param[in] items A list of available items

				\return The method returns an index of the selected item
			*/

			TDE2_API I32 Popup(const std::string& label, I32 currSelectedItem, const std::vector<std::string>& items) override;

			/*!
				\brief The method displays an image with the given parameters set

				\param[in] textureHandle A pointer to resource handler that corresponds to texture that should be displayed
				\param[in] sizes A sizes of displayed image
				\param[in] uvRect A uv coordinates of displayed texture
			*/

			TDE2_API void Image(TResourceId textureHandle, const TVector2& sizes, const TRectF32& uvRect = { 0.0f, 0.0f, 1.0f, 1.0f }) override;

			/*!
				\brief The method displays selectable label item

				\param[in] id A label of an item
				\param[in] isSelected A flag determines whether or not the current node is highlighted

				\return Returns true if the current item was selected, false in other cases
			*/

			TDE2_API bool SelectableItem(const std::string& id, bool isSelected) override;

			/*!
				\brief The method display tree's element. Always should be called in pair with EndTreeNode at end

				\param[in] id A label of a node
				\param[in] isSelected A flag determines whether or not the current node is highlighted

				\return A tuple the first argument is opened/hidden state of a node, the second is whether selected or not the node
			*/

			TDE2_API std::tuple<bool, bool> BeginTreeNode(const std::string& id, bool isSelected = false) override;

			/*!
				\brief The method should be called after BeginTreeNode
			*/

			TDE2_API void EndTreeNode() override;

			/*!
				\brief The method displays header which looks same as BeginTreeNode.

				\param[in] id A label that will be displayed
				\param[in] isOpened The flag defines whether or not the group is unwinded or not
				\param[in] isSelected The flag defines whether the header be highlighted or not
				\param[in] itemClicked The callback is called when a user clicks over the header
				\param[in] contentAction You can invoke any ImGUIContext command there which should be displayed when the header is unwinded

				\return The method return true if it was unwinded, false in other cases
			*/

			TDE2_API bool CollapsingHeader(const std::string& id, bool isOpened, bool isSelected = false,
											const std::function<void()>& itemClicked = nullptr, 
											const std::function<void()>& contentAction = nullptr) override;

			/*!
				\brief The method creates a new window on the screen. Every call after this one
				will be related with this window

				\param[in] name A window's name
				\param[in, out] isOpened A flag that determines whether the window is shown or not
				\param[in] params A set of additonal parameters which can be applied to the window

				\return The method returns false if the window is collapsed or not visible
			*/

			TDE2_API bool BeginWindow(const std::string& name, bool& isOpened, const TWindowParams& params) override;

			/*!
				\brief The method finishes populating of current window
			*/

			TDE2_API void EndWindow() override;

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

			TDE2_API void BeginHorizontal() override;

			/*!
				\brief The method closes a horizontal group
			*/

			TDE2_API void EndHorizontal() override;

			/*!
				\brief The method creates child window's region within the current one

				\return The method returns true if the window is shown
			*/

			TDE2_API bool BeginChildWindow(const std::string& name, const TVector2& sizes) override;

			/*!
				\brief The method finalizes current child window
			*/

			TDE2_API void EndChildWindow() override;

			/*!
				\brief The method moves current position of the cursor to given point
			*/

			TDE2_API void SetCursorScreenPos(const TVector2& pos) override;

			TDE2_API void DisplayIDGroup(I32 id, const std::function<void()>& idGroupCallback = {}) override;

			/*!
				\brief The method provides implementation of drag & drop source

				\param[in] action A callback in which all the stuff for register should be executed
			*/

			TDE2_API void RegisterDragAndDropSource(const std::function<void()>& action = nullptr) override;

			/*!
				\brief The method provides implementation of drag & drop target

				\param[in] action A callback in which all the stuff for register should be executed
			*/

			TDE2_API void RegisterDragAndDropTarget(const std::function<void()>& action = nullptr) override;

			/*!
				\brief The method returns a width of current active window

				\return The method returns a width of current active window
			*/

			TDE2_API F32 GetWindowWidth() const override;

			/*!
				\brief The method returns a height of current active window

				\return The method returns a height of current active window
			*/

			TDE2_API F32 GetWindowHeight() const override;

			/*!
				\brief The method returns cursor position in a viewport space. The method is useful when
				you work with DrawX methods

				\return The method returns cursor position in a viewport space
			*/

			TDE2_API TVector2 GetCursorScreenPos() const override;

			/*!
				\brief The method returns sizes which a given text string occupies

				\param[in] text A text string

				\return The static function returns sizes which a given text string occupies
			*/

			TDE2_API TVector2 GetTextSizes(const std::string& text) const override;

			TDE2_API bool IsItemActive() const override;

			/*!
				\return The method returns true when the cursor is over some ImGUI element
			*/

			TDE2_API bool IsMouseOverUI() const override;

			TDE2_API bool IsMouseDoubleClicked(U8 buttonId) const override;

			TDE2_API bool IsMouseDragging(U8 buttonId) const override;

			TDE2_API TVector2 GetMouseDragDelta(U8 buttonId) const override;

			TDE2_API TVector2 GetMousePosition() const override;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CImGUIContext)

			TDE2_API E_RESULT_CODE _initInternalImGUIContext(ImGuiIO& io);

			TDE2_API void _updateInputState(ImGuiIO& io, IInputContext* pInputContext);

			TDE2_API E_RESULT_CODE _initGraphicsResources(ImGuiIO& io, IGraphicsContext* pGraphicsContext, IGraphicsObjectManager* pGraphicsManager,
														  IResourceManager* pResourceManager);

			TDE2_API E_RESULT_CODE _initSystemFonts(ImGuiIO& io, IResourceManager* pResourceManager, IGraphicsObjectManager* pGraphicsManager);

			TDE2_API void _engineInternalRender(ImDrawData* pImGUIData, CRenderQueue* pRenderQueue);

			TDE2_API void _initInputMappings(ImGuiIO& io);

			TDE2_API void _prepareLayout();

			TDE2_API ImDrawList* _getCurrActiveDrawList() const;

			TDE2_API void _setDragAndDropData(const std::string& id, const void* pData, U32 size) override;
			TDE2_API const void* _getDragAndDropData(const std::string& id) const override;
		protected:
			std::atomic_bool        mIsInitialized;

			IWindowSystem*          mpWindowSystem;

			IGraphicsContext*       mpGraphicsContext;

			IGraphicsObjectManager* mpGraphicsObjectManager;

			IResourceManager*       mpResourceManager;

			IInputContext*          mpInputContext;

			ImGuiIO*                mpIOContext;

			TResourceId             mFontTextureHandle;

			TResourceHandlesArray   mUsedResourcesRegistry;

			TResourceId             mDefaultEditorMaterialHandle;

			IVertexBuffer*          mpVertexBuffer;

			IIndexBuffer*           mpIndexBuffer;

			CRenderQueue*           mpEditorUIRenderQueue;

			IVertexDeclaration*     mpEditorUIVertexDeclaration;

			bool                    mIsHorizontalGroupEnabled = false;

			TDrawListsStack         mpDrawListsContext;

			std::unordered_map<uintptr_t, TMaterialInstanceId> mUsingMaterials;
	};
}