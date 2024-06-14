/*!
	\file CResourcesBuildManifest.h
	\date 21.06.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Types.h"
#include "../core/Serialization.h"
#include "../core/CBaseObject.h"
#include <memory>
#include <vector>
#include <functional>
#include <mutex>


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class CResourcesBuildManifest;
	class IFileSystem;


	TDE2_DECLARE_SCOPED_PTR(IFileSystem)


	typedef struct TResourceBuildInfo: ISerializable
	{
		TDE2_REGISTER_VIRTUAL_TYPE_EX(TResourceBuildInfo, GetResourceTypeId)

		TDE2_API virtual ~TResourceBuildInfo() = default;

		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

		std::string mRelativePathToResource;
	} TResourceBuildInfo, *TResourceBuildInfoPtr;


	typedef struct TMeshResourceBuildInfo: TResourceBuildInfo
	{
		TDE2_REGISTER_TYPE(TMeshResourceBuildInfo)
		TDE2_REGISTER_VIRTUAL_TYPE_EX(TMeshResourceBuildInfo, GetResourceTypeId)

		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

		struct TAnimationClipImportInfo
		{
			std::string mOutputAnimationPath;
			bool mIsLooped = false;
			U32 mStartRange = 0;
			U32 mEndRange = 0;
		};

		bool mIsSkinned = false;
		bool mImportTangents = true;

		std::vector<TAnimationClipImportInfo> mAnimations;
	} TMeshResourceBuildInfo, *TMeshResourceBuildInfoPtr;


	typedef struct TTexture2DResourceBuildInfo : TResourceBuildInfo
	{
		TDE2_REGISTER_TYPE(TTexture2DResourceBuildInfo)
		TDE2_REGISTER_VIRTUAL_TYPE_EX(TTexture2DResourceBuildInfo, GetResourceTypeId)

		/*!
			\brief The method deserializes object's state from given reader

			\param[in, out] pReader An input stream of data that contains information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

		/*!
			\brief The method serializes object's state into given stream

			\param[in, out] pWriter An output stream of data that writes information about the object

			\return RC_OK if everything went ok, or some other code, which describes an error
		*/

		TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

		E_TEXTURE_FILTER_TYPE mFilteringType = E_TEXTURE_FILTER_TYPE::FT_BILINEAR;
		E_ADDRESS_MODE_TYPE   mAddressMode = E_ADDRESS_MODE_TYPE::AMT_CLAMP;

		bool                  mGenerateMipMaps = true;
		bool                  mIsDynamicRangeEnabled = false; /// HDR formats
		bool                  mIsAtlasModeEnabled = false;

		U32 mInputSheetColsCount = 1;
		U32 mInputSheetRowsCount = 1;
	} TTexture2DResourceBuildInfo, *TTexture2DResourceBuildInfoPtr;


	/*!
		\brief A factory function for creation objects of CResourcesBuildManifest's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourcesBuildManifest's implementation
	*/

	TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE& result);


	TDE2_API TPtr<CResourcesBuildManifest> LoadResourcesBuildManifest(TPtr<IFileSystem> pFileSystem, const std::string& path);


	/*!
		class CResourcesBuildManifest

		\brief The class is used to gather information about how the resources should be built.
		The manifest is used only on build step for runtime purposes CResourcesRuntimeManifest is used
	*/

	class CResourcesBuildManifest: public CBaseObject, public ISerializable
	{
		public:
			friend TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE&);
		public:
			typedef std::function<bool(const TResourceBuildInfo&)> TResourceInfoVisitFunctior;
		public:
			/*!
				\brief The method initializes internal state of the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init();

			/*!
				\brief The method deserializes object's state from given reader

				\param[in, out] pReader An input stream of data that contains information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load(IArchiveReader* pReader) override;

			/*!
				\brief The method serializes object's state into given stream

				\param[in, out] pWriter An output stream of data that writes information about the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Save(IArchiveWriter* pWriter) override;

			/*!
				\brief The method registers an information for the resource. It there already exists 
				a resource with the same path it's overriden

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE AddResourceBuildInfo(std::unique_ptr<TResourceBuildInfo> pResourceInfo);

			/*!
				\brief The method unregisters resource and removes its information from the manifest

				\param[in] relativePath Exact relative path to the resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE RemoveResourceBuildInfo(const std::string& relativePath);

			TDE2_API TResourceBuildInfo* FindResourceBuildInfo(const std::string& relativePath);
			
			TDE2_API E_RESULT_CODE ForEachRegisteredResource(const TResourceInfoVisitFunctior& action = nullptr);

			template <typename T>
			E_RESULT_CODE ForEachRegisteredResource(const TResourceInfoVisitFunctior& action = nullptr)
			{
				std::lock_guard<std::mutex> lock(mMutex);
				return _forEachTypedResource(T::GetTypeId(), action);
			}

			TDE2_API E_RESULT_CODE SetBaseResourcesPath(const std::string& value);

			TDE2_API const std::string& GetBaseResourcesPath() const;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourcesBuildManifest)

			TDE2_API E_RESULT_CODE _forEachTypedResource(TypeId resourceTypeId, const TResourceInfoVisitFunctior& action = nullptr);
		private:
			mutable std::mutex                               mMutex;

			static constexpr U16                             mVersionTag = 0x1;

			std::vector<std::unique_ptr<TResourceBuildInfo>> mpResourcesBuildConfigs;

			std::string                                      mBaseFilePath; 
	};


	/*!
		\brief The function is used to create concrete type of an information's entity based on file's extension
	*/

	TDE2_API std::unique_ptr<TResourceBuildInfo> CreateResourceBuildInfoForFilePath(const std::string& path);
}

#endif