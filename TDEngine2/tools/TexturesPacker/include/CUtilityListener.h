/*!
	\file CUtilityListener.h
	\date 19.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include <TDEngine2.h>
#include "CEditorWindow.h"
#include <vector>
#include <string>



struct TUtilityOptions
{
	std::vector<std::string> mInputFiles;
	std::string mOutputFilename;
	bool mIsDefault = true;

	std::string mAtlasName;
	std::string mFormatStr;
	std::string mBasePath; ///< The path that will be excluded from input files paths

	TDEngine2::U32 mAtlasWidth;
	TDEngine2::U32 mAtlasHeight;
};


class CUtilityListener : public TDEngine2::IEngineListener
{
	public:
		explicit CUtilityListener(const TUtilityOptions& options);

		/*!
			\brief The event occurs after the engine was initialized

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnStart() override;

		/*!
			\brief The event occurs every frame

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnUpdate(const float& dt) override;

		/*!
			\brief The event occurs before the engine will be destroyed

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDEngine2::E_RESULT_CODE OnFree() override;

		/*!
				\brief The method is used to inject a pointer to the engine into the object of this type

				\param[in] An instance of the engine's core
		*/

		void SetEngineInstance(TDEngine2::IEngineCore* pEngineCore) override;

	protected:
		void _drawMainMenu();
		void _createNewAtlasModalWindow();

		TDEngine2::E_RESULT_CODE _processInNonGraphicalMode();

	protected:
		TDEngine2::IEngineCore*      mpEngineCoreInstance;

		TDEngine2::TPtr<TDEngine2::IGraphicsContext> mpGraphicsContext;

		TDEngine2::TPtr<TDEngine2::IWindowSystem>    mpWindowSystem;

		TDEngine2::TPtr<TDEngine2::IResourceManager> mpResourceManager;

		TDEngine2::CScopedPtr<TDEngine2::CEditorWindow> mpEditorWindow;

		TUtilityOptions              mOptions;

		TDEngine2::TResourceId       mCurrEditableAtlasId;

		std::string                  mLastSavedPath;

		std::string                  mNewAtlasName;
		TDEngine2::I32               mNewAtlasWidth = 2;
		TDEngine2::I32               mNewAtlasHeight = 2;
		TDEngine2::E_FORMAT_TYPE     mNewAtlasFormat = TDEngine2::E_FORMAT_TYPE::FT_NORM_UBYTE4;

		static std::vector<std::string> mAvailableFormats;

};


static struct TVersion
{
	const uint32_t mMajor = 0;
	const uint32_t mMinor = 1;
} ToolVersion;


TDEngine2::TResult<TUtilityOptions> ParseOptions(int argc, const char** argv);