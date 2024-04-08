/*!
	\file CProjectSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "../math/TVector3.h"
#include "../math/TVector4.h"
#include <thread>


namespace TDEngine2
{
	class IArchiveReader;


	enum class TLocaleId : U32;


	ENUM_META(SECTION=core)
	enum class E_QUALITY_PRESET_TYPE : U8
	{
		LOW,
		MEDIUM,
		HIGH,
		ULTRA,
		PRESETS_COUNT
	};


	/*!
		class CProjectSettings

		\brief The class is a singleton which contains all bunch of project related settings.
		Consider this values as build time settings for the project when CGameUserSettings as current applied ones
	*/

	class CProjectSettings : public CBaseObject
	{
		public:
			TDE2_API E_RESULT_CODE Init(IArchiveReader* pFileReader);

			/*!
				\brief The function is replacement of factory method for instances of this type.
				The only instance will be created per program's lifetime.

				\return A pointer to an instance of CProjectSettings type
			*/

			TDE2_API static TPtr<CProjectSettings> Get();
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CProjectSettings)
		public:
			/*!
				\brief The type contains all settings that define a particular quality level for graphical part of the application
			*/
			struct TQualityPreset
			{
				bool mIsShadowMappingEnabled = true;
				U32  mShadowMapSizes = 512;
				U32  mShadowCascadesCount = 4;
				TVector4 mShadowCascadesSplits = TVector4(0.25f, 0.5f, 0.75f, 1.0f);

				bool mIsVolumetricCloudsEnabled = false;
			};

			std::array<TQualityPreset, static_cast<U32>(E_QUALITY_PRESET_TYPE::PRESETS_COUNT)> mQualityPresets;

			struct
			{
#if defined(TDE2_USE_UNIXPLATFORM)
				std::string mGraphicsPluginFilePath = "GLGraphicsContext";
#else
				std::string mGraphicsPluginFilePath = "D3D11GraphicsContext";
#endif

				std::string mDefaultSkyboxMaterial = "DefaultResources/Materials/DefaultSkybox.material";
				std::string mVolumetricCloudsMainShader = "Shaders/Default/VolumetricClouds.cshader";

				std::string mShaderCachePathPattern = "ProjectResources/ShadersPack_{0}.cache";
				bool        mIsShaderCacheEnabled = true;
			} mGraphicsSettings;

			struct
			{
				std::string mAudioPluginFilePath = "FmodAudioContext";
			} mAudioSettings;

			struct TCommonSettings
			{
				struct TFilesystemPathAlias
				{
					std::string mAlias;
					std::string mPath;
				};

				U32 mMaxNumOfWorkerThreads = std::thread::hardware_concurrency() - 1;

				std::string mApplicationName;

				U32 mFlags = static_cast<U32>(P_RESIZEABLE | P_ZBUFFER_ENABLED);

				std::vector<TFilesystemPathAlias> mAdditionalMountedDirectories;
				std::vector<std::string> mUserPluginsToLoad;

				std::string mPathToResourcesRuntimeManifest = "ProjectResources/ResourcesRuntimeMeta.manifest";
				std::string mBaseResourcesPath = "../../Resources/";

				std::string mPathToDefaultPrefabsManifest = "ProjectResources/PrefabsCollection.manifest";

				bool mBinaryResourcesActive = false; ///< If the given project settings are stored as text the field is false, for binarized version it's true
			} mCommonSettings;


			struct TLocalizationSettings
			{
				struct TLocaleInfo
				{
					std::string mName;
					std::string mPackagePath;
					TLocaleId mId;
				};

				std::vector<TLocaleInfo> mRegisteredLocales;
			} mLocalizationSettings;


			struct TWorldSettings
			{
				F32 mEntitiesBoundsUpdateInterval = 0.5f;
			} mWorldSettings;


			struct TSceneManagerSettings
			{
				std::string mMainLevelScenePath = "MainScene.scene";
			} mScenesSettings;


			struct TSplashScreenSettings
			{
				bool mIsEnabled = false;
				bool mIsSkipAllowed = true;
				F32  mMaxShowDuration = 4.0f;
				std::string mSplashScreenSceneId = "ProjectResources/Scenes/SplashScreen.scene";
			} mSplashScreenSettings;


			struct T3DPhysicsSettings
			{
				TVector3 mGravity = TVector3(0.0f, -10.0f, 0.0f);
			} m3DPhysicsSettings;
	};
}