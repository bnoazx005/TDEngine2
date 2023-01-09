/*!
	\file CProjectSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
#include "../math/TVector3.h"
#include <thread>


namespace TDEngine2
{
	class IArchiveReader;


	enum class TLocaleId : U32;


	/*!
		class CProjectSettings

		\brief The class is a singleton which contains all bunch of project related settings 
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
			struct
			{
#if defined(TDE2_USE_UNIXPLATFORM)
				std::string mRendererPluginFilePath = "GLGraphicsContext";
#else
				std::string mRendererPluginFilePath = "D3D11GraphicsContext";
#endif

				struct
				{
					U32  mShadowMapSizes = 512;
					bool mIsShadowMappingEnabled = true;
				} mRendererSettings;

				std::string mDefaultSkyboxMaterial = "DefaultResources/Materials/DefaultSkybox.material";
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
				std::string mCurrActiveLocaleId;
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
				F32  mShowDuration = 4.0f;
			} mSplashScreenSettings;


			struct T3DPhysicsSettings
			{
				TVector3 mGravity = TVector3(0.0f, -10.0f, 0.0f);
			} m3DPhysicsSettings;
	};
}