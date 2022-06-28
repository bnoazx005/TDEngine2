#include "meshConverter.h"
#include "deps/argparse/argparse.h"
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <queue>
#include <cstring>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../../deps/Wrench/source/result.hpp"


#if _HAS_CXX17
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	#include <experimental/filesystem>
	namespace fs = std::experimental::filesystem;
#endif

namespace TDEngine2
{

	constexpr const char* Usage[] =
	{
		"tde2_mesh_converter <input> .. <input> [options]\nwhere <input> - single file path or directory",
		"\nExample:\nRun tde2_mesh_converter --resources_manifest \"path_to_manifest\" \nto run bucketed mode and convert all registered mesh files",
		0
	};


	static constexpr USIZE FileHeaderSize = 16;
	static constexpr const C8 LODInstanceSuffix[] = "_LOD";


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
		const char* pAnimationsInfoFilename = nullptr;
		const char* pResourcesManifestFilepath = nullptr;

		struct argparse_option options[] = {
			OPT_HELP(),
			OPT_GROUP("Options"),
			OPT_BOOLEAN('V', "version", &showVersion, "Print version info and exit"),
			OPT_STRING(0, "outdir", &pOutputDirectory, "Write output into specified <dirname>"),
			OPT_STRING('o', "output", &pOutputFilename, "Output file's name <filename>"),
			OPT_STRING(0, "anim_import_info", &pOutputFilename, "A file which contains information about imported animations"),
			OPT_STRING(0, "resources_manifest", &pResourcesManifestFilepath, "A path to a manifest with build configurations of resources (if resources_manifest is used the bucket mode is enabled)"),
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

		if (utilityOptions.mInputFiles.empty() && !pResourcesManifestFilepath)
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

		if (pAnimationsInfoFilename)
		{
			utilityOptions.mAnimationImportInfoFilename = pAnimationsInfoFilename;
		}

		if (pResourcesManifestFilepath)
		{
			utilityOptions.mResourcesBuildManifestFilename = pResourcesManifestFilepath;
		}

		utilityOptions.mShouldSkipNormals   = static_cast<bool>(skipNormals);
		utilityOptions.mShouldSkipTangents  = static_cast<bool>(skipTangents);
		utilityOptions.mShouldSkipJoints    = static_cast<bool>(skipJoints);
		utilityOptions.mIsBucketModeEnabled = pResourcesManifestFilepath && utilityOptions.mInputFiles.empty();

		return Wrench::TOkValue<TUtilityOptions>(utilityOptions);
	}


	std::vector<std::string> BuildFilesList(const std::vector<std::string>& directories) TDE2_NOEXCEPT
	{
		if (directories.empty())
		{
			return {};
		}

		static const std::array<std::string, 6> extensions{ ".obj", ".OBJ", ".fbx", ".FBX", ".dae", ".DAE" };

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


	static TVector3 ConvertAssimpVector3(const aiVector3D& vec)
	{
		return TVector3(vec.x, vec.y, vec.z);
	}


	static TQuaternion ConvertAssimpQuaternion(const aiQuaternion& rotation)
	{
		return TQuaternion(rotation.x, rotation.y, rotation.z, rotation.w);
	}


	static E_RESULT_CODE ReadSkeletonData(IEngineCore* pEngineCore, const CScopedPtr<CSkeleton>& pSkeleton, const std::string& filePath, const TUtilityOptions& options, 
										const aiScene* pScene, bool& hasAnimationData) TDE2_NOEXCEPT
	{
		const aiBone* pCurrBone = nullptr;

		E_RESULT_CODE result = RC_OK;

		hasAnimationData = false;

		if (RC_OK != result)
		{
			return result;
		}

		pSkeleton->SetInvBindPoseUsing(true);

		std::unordered_map<U32, std::string> jointLinksInfo; /// U32 is joint index, std::string contains parent's name

		std::queue<aiNode*> nodesToVisit;
		nodesToVisit.emplace(pScene->mRootNode);

		/// \note Traverse the scene's hierarchy
		while (!nodesToVisit.empty())
		{
			aiNode* pCurrNode = nodesToVisit.front();
			nodesToVisit.pop();

			for (U32 i = 0; i < pCurrNode->mNumMeshes; ++i)
			{
				aiMesh* pMesh = pScene->mMeshes[pCurrNode->mMeshes[i]];
				if (!pMesh)
				{
					TDE2_ASSERT(false);
					continue;
				}

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
							pJoint->mInvBindTransform   = ConvertAssimpMatrix(pCurrBone->mOffsetMatrix);
							pJoint->mLocalBindTransform = ConvertAssimpMatrix(pCurrBone->mNode->mTransformation);
						}
					}
				}
			}

			for (U32 i = 0; i < pCurrNode->mNumChildren; ++i)
			{
				nodesToVisit.push(pCurrNode->mChildren[i]);
			}
		}

		if (pSkeleton->GetJointsCount() < 1) /// \note If there are no joints just skip the serialization step
		{
			return RC_OK;
		}

		hasAnimationData = true;

		/// \note Generate links between joints
		pSkeleton->ForEachJoint([pSkeleton, &jointLinksInfo](TJoint* pJoint)
		{
			if (auto pParentJoint = pSkeleton->GetJointByName(jointLinksInfo[pJoint->mIndex]))
			{
				pJoint->mParentIndex = pParentJoint->mIndex;
			}
		});

		/// \note Write down the resource into file sytem
		if (auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>())
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


	struct TAnimationClipInfo
	{
		std::string mName;
		U32 mStartFrame;
		U32 mEndFrame;
		bool mIsLooped;
	};


	typedef std::unordered_map<std::string, std::vector<TAnimationClipInfo>> TAnimationsInfoTable;


	static TResult<TAnimationsInfoTable> ReadAnimationsInfoTable(IEngineCore* pEngineCore, const std::string& filepath) TDE2_NOEXCEPT
	{
		TAnimationsInfoTable output;

		if (auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>())
		{
			auto importDatabaseOpenResult = pFileSystem->Open<IYAMLFileReader>(filepath);
			if (importDatabaseOpenResult.HasError())
			{
				return Wrench::TErrValue<E_RESULT_CODE>(importDatabaseOpenResult.GetError());
			}

			if (IYAMLFileReader* pAnimationsDatabaseReader = pFileSystem->Get<IYAMLFileReader>(importDatabaseOpenResult.Get()))
			{
				/// \note Check meta information
				pAnimationsDatabaseReader->BeginGroup("meta");
				{
					if (pAnimationsDatabaseReader->GetString("resource-type") != "animations-import-info")
					{
						return Wrench::TErrValue<E_RESULT_CODE>(RC_FAIL);
					}
				}
				pAnimationsDatabaseReader->EndGroup();

				pAnimationsDatabaseReader->BeginGroup("entities");
				{
					while (pAnimationsDatabaseReader->HasNextItem())
					{
						pAnimationsDatabaseReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
						{
							auto&& filePath = pAnimationsDatabaseReader->GetString("filepath");

							auto& animations = output[filePath];

							/// \note Read information about imported animation clips
							pAnimationsDatabaseReader->BeginGroup("animations");
							{
								while (pAnimationsDatabaseReader->HasNextItem())
								{
									pAnimationsDatabaseReader->BeginGroup(Wrench::StringUtils::GetEmptyStr());
									{
										animations.push_back(
											{ 
												pAnimationsDatabaseReader->GetString("id"),
												pAnimationsDatabaseReader->GetUInt32("start"),
												pAnimationsDatabaseReader->GetUInt32("end"),
												pAnimationsDatabaseReader->GetBool("looped"),
											});
									}
									pAnimationsDatabaseReader->EndGroup();
								}
							}
							pAnimationsDatabaseReader->EndGroup();
						}
						pAnimationsDatabaseReader->EndGroup();
					}
				}
				pAnimationsDatabaseReader->EndGroup();

				pAnimationsDatabaseReader->Close();
			}
		}

		return Wrench::TOkValue<TAnimationsInfoTable>(output);
	}

	
	static TResult<TAnimationsInfoTable> GenerateAnimationsInfoFromResourcesManifest(IEngineCore* pEngineCore, const TUtilityOptions& options)
	{
		auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>();

		auto pResourcesManifest = LoadResourcesBuildManifest(pFileSystem, options.mResourcesBuildManifestFilename);
		if (!pResourcesManifest)
		{
			return Wrench::TErrValue<E_RESULT_CODE>(RC_FILE_NOT_FOUND);
		}

		const std::string& basePath = pResourcesManifest->GetBaseResourcesPath();

		TAnimationsInfoTable infoTable;

		pResourcesManifest->ForEachRegisteredResource<TMeshResourceBuildInfo>([&basePath, &infoTable, pEngineCore, pFileSystem](const TResourceBuildInfo& resourceInfo)
		{
			const TMeshResourceBuildInfo& meshInfo = dynamic_cast<const TMeshResourceBuildInfo&>(resourceInfo);
			
			std::vector<TAnimationClipInfo> importInfo;

			for (auto&& animationClipInfo : meshInfo.mAnimations)
			{
				TAnimationClipInfo clipInfo;
				clipInfo.mName       = pFileSystem->CombinePath(basePath, animationClipInfo.mOutputAnimationPath);
				clipInfo.mIsLooped   = animationClipInfo.mIsLooped;
				clipInfo.mStartFrame = animationClipInfo.mStartRange;
				clipInfo.mEndFrame   = animationClipInfo.mEndRange;

				importInfo.emplace_back(clipInfo);
			}

			infoTable.emplace(pFileSystem->CombinePath(basePath, meshInfo.mRelativePathToResource), importInfo);

			return true;
		});

		return Wrench::TOkValue<TAnimationsInfoTable>(infoTable);
	}


	static TResult<TAnimationsInfoTable> ReadAnimationsInfoTable(IEngineCore* pEngineCore, const TUtilityOptions& options)
	{
		if (options.mIsBucketModeEnabled)
		{
			return GenerateAnimationsInfoFromResourcesManifest(pEngineCore, options);
		}

		return ReadAnimationsInfoTable(pEngineCore, options.mAnimationImportInfoFilename);
	}



	static E_RESULT_CODE ReadAnimationsData(IEngineCore* pEngineCore, const std::string& filePath, const TUtilityOptions& options, const aiScene* pScene) TDE2_NOEXCEPT
	{
		auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>();

		auto animationsInfoTableResult = ReadAnimationsInfoTable(pEngineCore, options);
		if (animationsInfoTableResult.HasError())
		{
			return animationsInfoTableResult.GetError();
		}

		TAnimationsInfoTable animationsInfo = animationsInfoTableResult.Get();

		const aiAnimation* pCurrAnimation = nullptr;
		const aiNodeAnim* pCurrChannel = nullptr;

		auto it = animationsInfo.find(filePath);
		if (it == animationsInfo.cend())
		{
			return RC_FAIL;
		}

		auto& animationsClipInfo = it->second;

		E_RESULT_CODE result = RC_OK;

		for (const TAnimationClipInfo& currAnimationClip : animationsClipInfo)
		{
			/// \note Create a new Animation resource
			CScopedPtr<CAnimationClip> pAnimation
			{
				dynamic_cast<CAnimationClip*>(
					CreateAnimationClip(
						pEngineCore->GetSubsystem<IResourceManager>().Get(), 
						pEngineCore->GetSubsystem<IGraphicsContext>().Get(), 
						pFileSystem->CombinePath(options.mOutputDirname, currAnimationClip.mName), 
						result))
			};

			for (U32 i = 0; i < pScene->mNumAnimations; ++i)
			{
				pCurrAnimation = pScene->mAnimations[i];

				const F64 ticksPerSecond = std::max<F64>(1e-3, pCurrAnimation->mTicksPerSecond);
				const U32 duration = static_cast<U32>(pCurrAnimation->mDuration / ticksPerSecond); /// animation's duration in seconds

				pAnimation->SetDuration(static_cast<F32>(currAnimationClip.mEndFrame - currAnimationClip.mStartFrame));
				pAnimation->SetWrapMode(currAnimationClip.mIsLooped ? E_ANIMATION_WRAP_MODE_TYPE::LOOP : E_ANIMATION_WRAP_MODE_TYPE::PLAY_ONCE);

				for (U32 channelId = 0; channelId < pCurrAnimation->mNumChannels; ++channelId)
				{
					pCurrChannel = pCurrAnimation->mChannels[channelId];

					const std::string jointId = pCurrChannel->mNodeName.data;

					auto* pPositionTrack = pAnimation->GetTrack<CVector3AnimationTrack>(pAnimation->CreateTrack<CVector3AnimationTrack>(Wrench::StringUtils::Format("track_pos_{0}", channelId + 1)));
					pPositionTrack->SetPropertyBinding(Wrench::StringUtils::Format("{1}." + CMeshAnimatorComponent::GetPositionJointChannelPattern(), jointId, CMeshAnimatorComponent::GetComponentTypeName()));
					pPositionTrack->SetInterpolationMode(E_ANIMATION_INTERPOLATION_MODE_TYPE::LINEAR);

					for (U32 k = 0; k < pCurrChannel->mNumPositionKeys; ++k)
					{
						auto&& position = pCurrChannel->mPositionKeys[k];

						const F32 time = static_cast<F32>(position.mTime / ticksPerSecond);

						if (!CMathUtils::IsInInclusiveRange(static_cast<F32>(currAnimationClip.mStartFrame), static_cast<F32>(currAnimationClip.mEndFrame), time))
						{
							continue;
						}
						
						if (auto pKeyValue = pPositionTrack->GetKey(pPositionTrack->CreateKey(time)))
						{
							pKeyValue->mValue = ConvertAssimpVector3(position.mValue);
						}
					}

					auto* pRotationTrack = pAnimation->GetTrack<CQuaternionAnimationTrack>(pAnimation->CreateTrack<CQuaternionAnimationTrack>(Wrench::StringUtils::Format("track_rot_{0}", channelId + 1)));
					pRotationTrack->SetPropertyBinding(Wrench::StringUtils::Format("{1}." + CMeshAnimatorComponent::GetRotationJointChannelPattern(), jointId, CMeshAnimatorComponent::GetComponentTypeName()));
					pRotationTrack->SetInterpolationMode(E_ANIMATION_INTERPOLATION_MODE_TYPE::LINEAR);

					for (U32 k = 0; k < pCurrChannel->mNumRotationKeys; ++k)
					{
						auto&& rotation = pCurrChannel->mRotationKeys[k];

						const F32 time = static_cast<F32>(rotation.mTime / ticksPerSecond);

						if (!CMathUtils::IsInInclusiveRange(static_cast<F32>(currAnimationClip.mStartFrame), static_cast<F32>(currAnimationClip.mEndFrame), time))
						{
							continue;
						}

						if (auto pKeyValue = pRotationTrack->GetKey(pRotationTrack->CreateKey(time)))
						{
							pKeyValue->mValue = ConvertAssimpQuaternion(rotation.mValue);
						}
					}
				}
			}

			/// \note Write down the resource into file sytem
			if (auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>())
			{
				auto animationFileResult = pFileSystem->Open<IYAMLFileWriter>(pAnimation->GetName(), true);
				if (animationFileResult.HasError())
				{
					return animationFileResult.GetError();
				}

				if (IYAMLFileWriter* pAnimationArchiveWriter = pFileSystem->Get<IYAMLFileWriter>(animationFileResult.Get()))
				{
					if (RC_OK != (result = pAnimation->Save(pAnimationArchiveWriter)))
					{
						return result;
					}

					pAnimationArchiveWriter->Close();
				}
			}
		}

		return RC_OK;
	}


	struct TMeshDataEntity
	{
		typedef std::vector<U32> TJointIndicesArray;

		std::string                     mName;
		I32                             mParentId = -1;

		std::vector<TVector4>           mVertices;
		std::vector<TVector4>           mNormals;
		std::vector<TVector4>           mTangents;
		std::vector<TVector2>           mTexcoords;
		std::vector<TVector4>           mColors;
		std::vector<U32>                mFaces;
		std::vector<std::vector<F32>>   mJointWeights;
		std::vector<TJointIndicesArray> mJointIndices;
	};


	static std::tuple<std::vector<F32>, std::vector<std::string>> ReadVertexJointData(const aiScene* pScene, const aiMesh* pMesh, U32 vertexId)
	{
		std::vector<F32> weights;
		std::vector<std::string> indices;

		for (U32 i = 0; i < pMesh->mNumBones; ++i)
		{
			auto pCurrBone = pMesh->mBones[i];

			for (U32 k = 0; k < pCurrBone->mNumWeights; ++k)
			{
				auto& weight = pCurrBone->mWeights[k];

				if (static_cast<U32>(weight.mVertexId) != vertexId || weight.mWeight < 1e-2f)
				{
					continue;
				}

				/// \note Find the least significant weight and replace it if it's less than a new one
				if (weights.size() >= MaxJointsCountPerVertex)
				{
					auto it = std::min_element(weights.begin(), weights.end());
					if (*it < weight.mWeight)
					{
						indices[std::distance(weights.begin(), it)] = pCurrBone->mName.C_Str();
						*it = weight.mWeight;
					}

					continue;
				}

				indices.push_back(pCurrBone->mName.C_Str());
				weights.push_back(weight.mWeight);
			}
		}

		return { weights, indices };
	}


	static TMeshDataEntity::TJointIndicesArray ResolveBoneIndices(const CScopedPtr<CSkeleton>& pSkeleton, const std::vector<std::string>& boneIdentifiers)
	{
		TMeshDataEntity::TJointIndicesArray resolvedIndices;

		for (const std::string& boneId : boneIdentifiers)
		{
			auto pJoint = pSkeleton->GetJointByName(boneId);
			resolvedIndices.push_back(pJoint ? pJoint->mIndex : static_cast<U32>(ISkeleton::mMaxNumOfJoints));
		}

		return resolvedIndices;
	}


	static TMeshDataEntity ReadMeshData(const aiScene* pScene, const aiMesh* pMesh, const CScopedPtr<CSkeleton>& pSkeleton, U32 baseIndex, const TUtilityOptions& options)
	{
		TMeshDataEntity meshData;

		std::vector<F32> boneWeights;
		std::vector<std::string> boneIndices;

		meshData.mName = pMesh->mName.C_Str();

		for (U32 i = 0; i < pMesh->mNumVertices; ++i)
		{
			auto& v = pMesh->mVertices[i];

			meshData.mVertices.emplace_back(v.x, v.y, v.z, 1.0f);

			if (pMesh->mTextureCoords[0])
			{
				auto& uv = pMesh->mTextureCoords[0][i];

				meshData.mTexcoords.emplace_back(uv.x, uv.y);
			}

			if (pMesh->mNormals && !options.mShouldSkipNormals)
			{
				auto& normal = pMesh->mNormals[i];

				meshData.mNormals.emplace_back(normal.x, normal.y, normal.z, 0.0f);
			}

			if (pMesh->mTangents && !options.mShouldSkipTangents)
			{
				auto& tangent = pMesh->mTangents[i];

				meshData.mTangents.emplace_back(tangent.x, tangent.y, tangent.z, 0.0f);
			}

			/// \todo Add retrieving information about joints
			std::tie(boneWeights, boneIndices) = ReadVertexJointData(pScene, pMesh, i);

			if (!options.mShouldSkipJoints && !boneWeights.empty())
			{
				meshData.mJointWeights.push_back(boneWeights);
				meshData.mJointIndices.push_back(ResolveBoneIndices(pSkeleton, boneIndices));
			}
		}

		for (U32 i = 0; i < pMesh->mNumFaces; ++i)
		{
			auto& pFace = pMesh->mFaces[i];

			meshData.mFaces.emplace_back(baseIndex + pFace.mIndices[0]);
			meshData.mFaces.emplace_back(baseIndex + pFace.mIndices[1]);
			meshData.mFaces.emplace_back(baseIndex + pFace.mIndices[2]);
		}

		return std::move(meshData);
	}


	static E_RESULT_CODE WriteFileHeader(IBinaryFileWriter* pMeshFileWriter, U32 meshesCount)
	{
		const U8 Version[4] = { 0, 0, 3, 0 }; /// \todo Move to header 

		E_RESULT_CODE result = pMeshFileWriter->SetPosition(0);

		result = result | pMeshFileWriter->Write("MESH", 4);
		result = result | pMeshFileWriter->Write(Version, sizeof(Version));
		result = result | pMeshFileWriter->Write(&meshesCount, sizeof(U32));
		result = result | pMeshFileWriter->Write(&meshesCount, sizeof(U32)); /// \note write any data here just for padding

		return result;
	}


	static E_RESULT_CODE WriteSingleMeshBlock(IBinaryFileWriter* pMeshFileWriter, const TMeshDataEntity& meshEntity, const TUtilityOptions& options)
	{
		const U16 MeshBlockTag             = 0x4D48;
		const U16 MeshVerticesBlockTag     = 0x01CD;
		const U16 MeshNormalsBlockTag      = 0xA10E;
		const U16 MeshTangentsBlockTag     = 0xA2DF;
		const U16 MeshTexcoords0BlockTag   = 0x02F0;
		const U16 MeshJointWeightsBlockTag = 0xA401;
		const U16 MeshJointIndicesBlockTag = 0xA502;
		const U16 MeshFacesBlockTag        = 0x03FF;

		E_RESULT_CODE result = RC_OK;

		C8 meshId[64]{ '\0' };

#ifdef TDE2_USE_WINPLATFORM
		strcpy_s(meshId, sizeof(meshId) / sizeof(meshId[0]), meshEntity.mName.data());
#else
		strcpy(meshId, meshEntity.mName.data());
#endif

		result = result | pMeshFileWriter->Write(&MeshBlockTag, sizeof(MeshBlockTag));
		result = result | pMeshFileWriter->Write(meshId, sizeof(meshId));

		const U32 vertexCount = static_cast<U32>(meshEntity.mVertices.size());
		const U32 facesCount  = static_cast<U32>(meshEntity.mFaces.size()) / 3;

		result = result | pMeshFileWriter->Write(&vertexCount, sizeof(vertexCount));
		result = result | pMeshFileWriter->Write(&facesCount, sizeof(facesCount));

		result = result | pMeshFileWriter->Write(&meshEntity.mParentId, sizeof(meshEntity.mParentId)); /// \node child-parent relationship's index

		/// \note Write vertices
		result = result | pMeshFileWriter->Write(&MeshVerticesBlockTag, sizeof(MeshVerticesBlockTag));
		
		for (const TVector4& v : meshEntity.mVertices)
		{
			result = result | pMeshFileWriter->Write(&v.x, sizeof(F32));
			result = result | pMeshFileWriter->Write(&v.y, sizeof(F32));
			result = result | pMeshFileWriter->Write(&v.z, sizeof(F32));
			result = result | pMeshFileWriter->Write(&v.w, sizeof(F32));
		}

		/// \note Write normal (optional chuch that can be omitted)
		if (!options.mShouldSkipNormals)
		{
			result = result | pMeshFileWriter->Write(&MeshNormalsBlockTag, sizeof(MeshNormalsBlockTag));
		
			for (const TVector4& normal : meshEntity.mNormals)
			{
				result = result | pMeshFileWriter->Write(&normal.x, sizeof(F32));
				result = result | pMeshFileWriter->Write(&normal.y, sizeof(F32));
				result = result | pMeshFileWriter->Write(&normal.z, sizeof(F32));
				result = result | pMeshFileWriter->Write(&normal.w, sizeof(F32));
			}
		}

		/// \note Write tangents (optional)
		if (!options.mShouldSkipTangents)
		{
			result = result | pMeshFileWriter->Write(&MeshTangentsBlockTag, sizeof(MeshTangentsBlockTag));

			for (const TVector4& tangent : meshEntity.mTangents)
			{
				result = result | pMeshFileWriter->Write(&tangent.x, sizeof(F32));
				result = result | pMeshFileWriter->Write(&tangent.y, sizeof(F32));
				result = result | pMeshFileWriter->Write(&tangent.z, sizeof(F32));
				result = result | pMeshFileWriter->Write(&tangent.w, sizeof(F32));
			}
		}

		/// \note Write first uv channel
		result = result | pMeshFileWriter->Write(&MeshTexcoords0BlockTag, sizeof(MeshTexcoords0BlockTag));

		for (const TVector2& uv : meshEntity.mTexcoords)
		{
			result = result | pMeshFileWriter->Write(&uv.x, sizeof(F32));
			result = result | pMeshFileWriter->Write(&uv.y, sizeof(F32));
			result = result | pMeshFileWriter->Write(&uv.x, sizeof(F32)); /// \note Unused
			result = result | pMeshFileWriter->Write(&uv.x, sizeof(F32));
		}

		/// \note Write joints weights (optional)
		if (!options.mShouldSkipJoints && !meshEntity.mJointWeights.empty())
		{
			result = result | pMeshFileWriter->Write(&MeshJointWeightsBlockTag, sizeof(MeshJointWeightsBlockTag));

			for (auto&& jointWeights : meshEntity.mJointWeights)
			{
				const U16 weightsCount = static_cast<U16>(jointWeights.size());
				result = result | pMeshFileWriter->Write(&weightsCount, sizeof(U16));

				TDE2_ASSERT(weightsCount <= MaxJointsCountPerVertex);
				
				for (F32 currWeight : jointWeights)
				{
					result = result | pMeshFileWriter->Write(&currWeight, sizeof(F32));
				}
			}

			/// \note Write joint indices
			result = result | pMeshFileWriter->Write(&MeshJointIndicesBlockTag, sizeof(MeshJointIndicesBlockTag));

			for (auto&& jointIndices : meshEntity.mJointIndices)
			{
				const U16 indicesCount = static_cast<U16>(jointIndices.size());
				result = result | pMeshFileWriter->Write(&indicesCount, sizeof(U16));

				TDE2_ASSERT(indicesCount <= MaxJointsCountPerVertex);

				for (U32 currJointIndex : jointIndices)
				{
					result = result | pMeshFileWriter->Write(&currJointIndex, sizeof(currJointIndex));
				}
			}
		}

		/// \note Write faces information
		const U16 indexFormat = options.mIndexFormat;

		result = result | pMeshFileWriter->Write(&MeshFacesBlockTag, sizeof(MeshFacesBlockTag));
		result = result | pMeshFileWriter->Write(&indexFormat, sizeof(indexFormat));

		for (U32 index : meshEntity.mFaces)
		{
			result = result | pMeshFileWriter->Write(&index, indexFormat);
		}

		return result;
	}


	/// returns offset to the end of the block or an error code
	static E_RESULT_CODE WriteMeshesData(IBinaryFileWriter* pMeshFileWriter, const std::vector<TMeshDataEntity>& meshes, const TUtilityOptions& options)
	{
		E_RESULT_CODE result = pMeshFileWriter->SetPosition(FileHeaderSize);
		
		for (const TMeshDataEntity& currMeshEntity : meshes)
		{
			result = result | WriteSingleMeshBlock(pMeshFileWriter, currMeshEntity, options);
		}

		return result;
	}


	static void ProcessHierarchyTable(const aiScene* pScene, std::vector<TMeshDataEntity>& meshes)
	{
		std::queue<aiNode*> nodesToVisit;
		nodesToVisit.emplace(pScene->mRootNode);

		std::stack<I32> parents;
		parents.push(-1);

		/// \note Traverse the scene's hierarchy
		while (!nodesToVisit.empty())
		{
			aiNode* pCurrNode = nodesToVisit.front();
			nodesToVisit.pop();

			I32 parentId = parents.top();
			parents.pop();

			for (U32 i = 0; i < pCurrNode->mNumMeshes; ++i)
			{
				aiMesh* pMesh = pScene->mMeshes[pCurrNode->mMeshes[i]];
				if (!pMesh)
				{
					TDE2_ASSERT(false);
					continue;
				}

				auto meshIt = std::find_if(meshes.begin(), meshes.end(), [meshId = pMesh->mName.C_Str()](const TMeshDataEntity& entity) { return meshId == entity.mName; });
				meshIt->mParentId = parentId;

				parentId = static_cast<I32>(std::distance(meshes.begin(), meshIt));
			}

			for (U32 i = 0; i < pCurrNode->mNumChildren; ++i)
			{
				nodesToVisit.push(pCurrNode->mChildren[i]);
				parents.push(parentId);
			}
		}
	}


	static E_RESULT_CODE SaveMeshFile(IEngineCore* pEngineCore, const aiScene* pScene, std::vector<TMeshDataEntity> meshes, const std::string& filePath, const TUtilityOptions& options)
	{
		if (auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>())
		{
			auto meshFileResult = pFileSystem->Open<IBinaryFileWriter>(filePath, true);
			if (meshFileResult.HasError())
			{
				return meshFileResult.GetError();
			}

			if (IBinaryFileWriter* pMeshFileWriter = pFileSystem->Get<IBinaryFileWriter>(meshFileResult.Get()))
			{
				E_RESULT_CODE result = WriteMeshesData(pMeshFileWriter, meshes, options);
				if (RC_OK != result)
				{
					return result;
				}

				if (RC_OK != (result = WriteFileHeader(pMeshFileWriter, static_cast<U32>(meshes.size()))))
				{
					return result;
				}

				pMeshFileWriter->Close();
			}
		}

		return RC_OK;
	}


	static E_RESULT_CODE ProcessSingleMeshFile(IEngineCore* pEngineCore, const std::string& filePath, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PopulateArmatureData);
		if (!pScene)
		{
			return RC_FAIL;
		}

		E_RESULT_CODE result = RC_OK;

		CScopedPtr<CSkeleton> pSkeleton
		{
			dynamic_cast<CSkeleton*>(CreateSkeleton(pEngineCore->GetSubsystem<IResourceManager>().Get(), pEngineCore->GetSubsystem<IGraphicsContext>().Get(), "NewSkeleton.skeleton", result))
		};

		bool hasAnimationData = false;

		if (RC_OK != (result = ReadSkeletonData(pEngineCore, pSkeleton, filePath, options, pScene, hasAnimationData)))
		{
			return result;
		}

		if (hasAnimationData && RC_OK != (result = ReadAnimationsData(pEngineCore, filePath, options, pScene)))
		{
			return result;
		}

		std::vector<const aiMesh*> sceneInternalMeshes;

		for (U32 i = 0; i < pScene->mNumMeshes; ++i)
		{
			auto pMesh = pScene->mMeshes[i];
			if (!pMesh)
			{
				continue;
			}

			/// \note Sort all internal meshes to move all LOD instances to the end of the file
			auto it = std::find_if(sceneInternalMeshes.begin(), sceneInternalMeshes.end(), [](const aiMesh* pMesh) { return strstr(pMesh->mName.C_Str(), LODInstanceSuffix); });
			sceneInternalMeshes.insert(it, pMesh);
		}

		std::vector<TMeshDataEntity> meshes;
		U32 baseIndex = 0;

		for (auto&& pMesh : sceneInternalMeshes)
		{
			meshes.emplace_back(ReadMeshData(pScene, pMesh, pSkeleton, baseIndex, options));
			baseIndex += pMesh->mNumVertices;
		}

		ProcessHierarchyTable(pScene, meshes);

		TUtilityOptions updatedOptions = options;
		updatedOptions.mIndexFormat = (baseIndex < 0xFFFF) ? sizeof(U16) : sizeof(U32);

		auto&& originalPath = fs::path(filePath);

		if (RC_OK != (result = SaveMeshFile(pEngineCore, pScene, std::move(meshes), (originalPath.parent_path() / originalPath.filename().replace_extension("mesh")).string(), updatedOptions)))
		{
			return result;
		}

		return RC_OK;
	}


	static E_RESULT_CODE ProcessMeshFilesWithResourcesManifest(IEngineCore* pEngineCore, const std::string& resourcesManifestFilepath) TDE2_NOEXCEPT
	{
		auto pFileSystem = pEngineCore->GetSubsystem<IFileSystem>();

		auto pResourcesManifest = LoadResourcesBuildManifest(pFileSystem, resourcesManifestFilepath);
		if (!pResourcesManifest)
		{
			return RC_FILE_NOT_FOUND;
		}

		E_RESULT_CODE result = RC_OK;

		const std::string& basePath = pResourcesManifest->GetBaseResourcesPath();

		pResourcesManifest->ForEachRegisteredResource<TMeshResourceBuildInfo>([&result, &basePath, pEngineCore, pFileSystem, &resourcesManifestFilepath](const TResourceBuildInfo& resourceInfo)
		{
			const TMeshResourceBuildInfo& meshInfo = dynamic_cast<const TMeshResourceBuildInfo&>(resourceInfo);

			TUtilityOptions options;
			options.mResourcesBuildManifestFilename = resourcesManifestFilepath;
			options.mShouldSkipNormals   = !meshInfo.mImportTangents;
			options.mShouldSkipTangents  = !meshInfo.mImportTangents;
			options.mShouldSkipJoints    = !meshInfo.mIsSkinned;
			options.mIsBucketModeEnabled = true;

			result = result | ProcessSingleMeshFile(pEngineCore, pFileSystem->CombinePath(basePath, resourceInfo.mRelativePathToResource), options);
			return true;
		});

		return result;
	}


	E_RESULT_CODE ProcessMeshFiles(IEngineCore* pEngineCore, std::vector<std::string>&& files, const TUtilityOptions& options) TDE2_NOEXCEPT
	{
		if (files.empty())
		{
			if (options.mIsBucketModeEnabled)
			{
				return ProcessMeshFilesWithResourcesManifest(pEngineCore, options.mResourcesBuildManifestFilename);
			}

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