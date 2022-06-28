/*!
	\file meshConverter.h
	\date 26.06.2021
	\author Ildar Kasimov

	\brief 
*/

#pragma once


#include <TDEngine2.h>
#include <vector>
#include <string>


namespace TDEngine2
{

#if TDE2_USE_NOEXCEPT
	#define TDE2_NOEXCEPT noexcept
#else 
	#define TDE2_NOEXCEPT 
#endif


	static struct TVersion
	{
		const uint32_t mMajor = 0;
		const uint32_t mMinor = 1;
	} ToolVersion;


	struct TUtilityOptions
	{
		std::vector<std::string> mInputFiles;

		std::string mOutputDirname = ".";
		std::string mOutputFilename = "NewMesh.mesh";
		std::string mAnimationImportInfoFilename = "AnimationsList.info";
		std::string mResourcesBuildManifestFilename = "ResourcesBuildSettings.manifest";

		bool mShouldSkipNormals  = false;
		bool mShouldSkipTangents = false;
		bool mShouldSkipJoints   = false;
		bool mIsBucketModeEnabled = false;

		U32 mIndexFormat = sizeof(U16);
	};


	TResult<TUtilityOptions> ParseOptions(int argc, const char** argv) TDE2_NOEXCEPT;

	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT;

	E_RESULT_CODE ProcessMeshFiles(IEngineCore* pEngineCore, std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT;

}