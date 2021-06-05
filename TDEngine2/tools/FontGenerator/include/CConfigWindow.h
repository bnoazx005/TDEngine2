/*!
	\file CConfigWindow.h
	\date 05.06.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include <string>
#include <vector>


namespace TDEngine2
{
	typedef struct TConfigWindowParams
	{
		IResourceManager* mpResourceManager;
		IInputContext* mpInputContext;
		IWindowSystem* mpWindowSystem;
		IFileSystem* mpFileSystem;
	} TConfigWindowParams;


	/*!
		\brief A factory function for creation objects of CConfigWindow's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to IConfigWindow's implementation
	*/

	TDE2_API IEditorWindow* CreateConfigWindow(const TConfigWindowParams& params, E_RESULT_CODE& result);


	/*!
		class CConfigWindow

		\brief The class is an implementation of a window for the editor
	*/

	class CConfigWindow : public CBaseEditorWindow
	{
		public:
			friend TDE2_API IEditorWindow* CreateConfigWindow(const TConfigWindowParams& params, E_RESULT_CODE&);

		public:
			/*!
				\brief The method initializes internal state of the editor

				\param[in, out] desc An object that determines parameters of the window

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(const TConfigWindowParams& params);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			TDE2_API TResourceId GetFontAtlasHandle() const;

		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CConfigWindow)

			/*!
				\brief The method should be should be implemented in all derived classes. It's called
				once per frame only if the editor's window is visible
			*/

			TDE2_API void _onDraw() override;

			TDE2_API void _fontSelectionToolbar();

			TDE2_API void _fontSDFConfiguration();
			TDE2_API void _fontAtlasSettings();

			TDE2_API void _updateFontsAtlas();

		protected:
			IResourceManager*     mpResourceManager;
			IDesktopInputContext* mpInputContext;
			IWindowSystem*        mpWindowSystem;
			IFileSystem*          mpFileSystem;

			std::string mFontFileName;
			std::string mLastSavedPath;
			std::string mFontAtlasName;
			std::string mTTFFontFilePath;
			std::string mFontAlphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-.,!@#$%^&*()_=+";

			F32 mGlyphHeight = 24.0f;

			I32 mAtlasWidth = 256;
			I32 mAtlasHeight = 256;

			TResourceId mFontResourceId;
			TResourceId mFontTextureAtlasId;
	};
}