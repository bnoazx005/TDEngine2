#include "meshConverter.h"
#include "deps/argparse/argparse.h"
#include <experimental/filesystem>
#include <unordered_set>
#include <iostream>
#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace fs = std::experimental::filesystem;

namespace TDEngine2
{

	constexpr const char* Usage[] =
	{
		"tde2_mesh_converter <input> .. <input> [options]",
		"where <input> - single file path or directory",
		0
	};


	TResult<TUtilityOptions> ParseOptions(int argc, const char** argv) TDE2_NOEXCEPT
	{
		int showVersion = 0;

		// flags
		int suppressLogOutput = 0;
		int debugMode = 0;
		int skipNormals = 0;
		int skipTangents = 0;
		int skipJoints = 0;

		const char* pOutputDirectory = nullptr;
		const char* pOutputFilename = nullptr;

		struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("Options"),
			OPT_BOOLEAN('V', "version", &showVersion, "Print version info and exit"),
			OPT_STRING(0, "outdir", &pOutputDirectory, "Write output into specified <dirname>"),
			OPT_STRING('o', "output", &pOutputFilename, "Output file's name <filename>"),
			OPT_BOOLEAN(0, "quiet", &suppressLogOutput, "Enables suppresion of program's output"),
			OPT_BOOLEAN('D', "debug", &debugMode, "Enables debug output of the utility"),
			OPT_BOOLEAN(0, "skip_normals", &skipNormals, "If defined object\'s normals will be skipped"),
			OPT_BOOLEAN(0, "skip_tangents", &skipTangents, "If defined object\'s tangents will be skipped"),
			OPT_BOOLEAN(0, "skip_joints", &skipJoints, "If defined object\'s joints information will be skipped"),
			OPT_END(),
		};

		struct argparse argparse;
		argparse_init(&argparse, options, Usage, 0);
		argparse_describe(&argparse, "\nThe utility is an mesh converter tool that processes FBX, OBJ files into internal engine format", "\n");
		argc = argparse_parse(&argparse, argc, argv);

		if (showVersion)
		{
			std::cout << "tde2_mesh_converter, version " << ToolVersion.mMajor << "." << ToolVersion.mMinor << std::endl;
			exit(0);
		}

		TUtilityOptions utilityOptions;

		// \note parse input files before any option, because argparse library will remove all argv's values after it processes that
		if (argc >= 1)
		{
			auto& sources = utilityOptions.mInputFiles;
			sources.clear();

			for (int i = 0; i < argc; ++i)
			{
				sources.push_back(argparse.out[i]);
			}
		}

		if (utilityOptions.mInputFiles.empty())
		{
			std::cerr << "Error: no input found\n";
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
		}

		if (pOutputDirectory)
		{
			utilityOptions.mOutputDirname = fs::path(pOutputDirectory).string();
		}

		if (pOutputFilename)
		{
			utilityOptions.mOutputFilename = pOutputFilename;
		}

		utilityOptions.mShouldSkipNormals  = static_cast<bool>(skipNormals);
		utilityOptions.mShouldSkipTangents = static_cast<bool>(skipTangents);
		utilityOptions.mShouldSkipJoints   = static_cast<bool>(skipJoints);

		return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
	}


	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT
	{
		if (directories.empty())
		{
			return {};
		}

		static const std::array<std::string, 4> extensions{ ".obj", ".OBJ", ".fbx", ".FBX" };

		auto&& hasValidExtension = [=](const std::string& ext)
		{
			for (auto&& currExtension : extensions)
			{
				if (currExtension == ext)
				{
					return true;
				}
			}

			return false;
		};

		std::unordered_set<std::string> processedPaths; // contains absolute paths that already have been processed 

		std::vector<std::string> filesPaths;

		for (auto&& currSource : directories)
		{
			// files
			if (!fs::is_directory(currSource))
			{
				auto&& path = fs::path{ currSource };

				auto&& absPathStr = fs::canonical(currSource).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(currSource);
					processedPaths.emplace(absPathStr);
				}

				continue;
			}

			// directories
			for (auto&& directory : fs::recursive_directory_iterator{ currSource })
			{
				auto&& path = directory.path();

				auto&& absPathStr = fs::canonical(path).string();

				if (processedPaths.find(absPathStr) == processedPaths.cend() && hasValidExtension(path.extension().string()))
				{
					filesPaths.emplace_back(path.string());
					processedPaths.emplace(absPathStr);
				}
			}
		}

		return filesPaths;
	}


	static TMatrix4 ConvertAssimpMatrix(const aiMatrix4x4& mat)
	{
		const std::array<F32, 16> elements
		{
			mat.a1, mat.a2, mat.a3, mat.a4,
			mat.b1, mat.b2, mat.b3, mat.b4,
			mat.c1, mat.c2, mat.c3, mat.c4,
			mat.d1, mat.d2, mat.d3, mat.d4,
		};

		return TMatrix4(&elements.front());
	}


	static E_RESULT_CODE ReadSkeletonData(IEngineCore* pEngineCore, const std::string& filePath, const TUtilityOptions& options, const aiMesh* pMesh) TDE2_NOEXCEPT
	{
		const aiBone* pCurrBone = nullptr;

		E_RESULT_CODE result = RC_OK;

		CScopedPtr<CSkeleton> pSkeleton 
		{
			dynamic_cast<CSkeleton*>(CreateSkeleton(pEngineCore->GetSubsystem<IResourceManager>(), pEngineCore->GetSubsystem<IGraphicsContext>(), "NewSkeleton.skeleton", result))
		};

		if (RC_OK != result)
		{
			return result;
		}

		pSkeleton->SetInvBindPoseUsing(true);

		std::unordered_map<U32, std::string> jointLinksInfo; /// U32 is joint index, std::string contains parent's name

		for (U32 i = 0; i < pMesh->mNumBones; ++i)
		{
			pCurrBone = pMesh->mBones[i];
			if (!pCurrBone)
			{
				continue;
			}

			if (auto jointIdResult = pSkeleton->CreateJoint(pCurrBone->mName.C_Str()))
			{
				jointLinksInfo[jointIdResult.Get()] = pCurrBone->mNode->mParent->mName.C_Str();

				if (TJoint* pJoint = pSkeleton->GetJoint(jointIdResult.Get()))
				{
					pJoint->mInvBindTransform = ConvertAssimpMatrix(pCurrBone->mOffsetMatrix);
				}
			}
		}

		/// \note Generate links between joints
		pSkeleton->ForEachJoint([pSkeleton, &jointLinksInfo](TJoint* pJoint)
		{
			if (auto pParentJoint = pSkeleton->GetJointByName(jointLinksInfo[pJoint->mIndex]))
			{
				pJoint->mParentIndex = pParentJoint->mIndex;
			}
		});

		/// \note Write down the resource into file sytem
		if (IFileSystem* pFileSystem = pEngineCore->GetSubsystem<IFileSystem>())
		{
			auto&& skeletonFilePath = fs::path(filePath).replace_extension("skeleton").string();

			auto skeletonFileResult = pFileSystem->Open<IYAMLFileWriter>(skeletonFilePath, true);
			if (skeletonFileResult.HasError())
			{
				return skeletonFileResult.GetError();
			}

			if (IYAMLFileWriter* pSkeletonArchiveWriter = pFileSystem->Get<IYAMLFileWriter>(skeletonFileResult.Get()))
			{
				if (RC_OK != (result = pSkeleton->Save(pSkeletonArchiveWriter)))
				{
					return result;
				}

				pSkeletonArchiveWriter->Close();
			}
		}

		return RC_OK;
	}


	static E_RESULT_CODE ProcessSingleMeshFile(IEngineCore* pEngineCore, const std::string& filePath, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		Assimp::Importer importer;
		
		const aiScene *pScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PopulateArmatureData);
		if (!pScene)
		{
			return RC_FAIL;
		}

		if (pScene->HasMeshes())
		{
			E_RESULT_CODE result = ReadSkeletonData(pEngineCore, filePath, options, pScene->mMeshes[0]);
			if (RC_OK != result)
			{
				return result;
			}
		}

		for (U32 i = 0; i < pScene->mNumMeshes; ++i)
		{
			auto pMesh = pScene->mMeshes[i];

		}

		return RC_OK;
	}


	E_RESULT_CODE ProcessMeshFiles(IEngineCore* pEngineCore, std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		if (files.empty())
		{
			return RC_INVALID_ARGS;
		}

		if (!fs::exists(options.mOutputDirname))
		{
			fs::create_directory(options.mOutputDirname);
		}

		E_RESULT_CODE result = RC_OK;

		for (auto&& currFilePath : files)
		{
			result = result | ProcessSingleMeshFile(pEngineCore, currFilePath, options);
		}

		return result;
	}
}