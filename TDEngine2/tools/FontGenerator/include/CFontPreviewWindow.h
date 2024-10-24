/*!
	\file CFontPreviewWindow.h
	\date 05.06.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CFontPreviewWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IFontPreviewWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateFontPreviewWindow(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem, E_RESULT_CODE& result);


	/*!
		class CFontPreviewWindow

		\brief The class is an implementation of a window for the editor
	*/

	class CFontPreviewWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateFontPreviewWindow(IResourceManager*, IInputContext*, IWindowSystem*, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IInputContext* pInputContext, IWindowSystem* pWindowSystem);

			TDE2_API void SetTextureAtlasResourceHandle(TResourceId atlasHandle);

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CFontPreviewWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

		protected:
			IResourceManager*     mpResourceManager;
			IDesktopInputContext* mpInputContext;
			IWindowSystem*        mpWindowSystem;

			TResourceId           mAtlasResourceHandle;
	};
}