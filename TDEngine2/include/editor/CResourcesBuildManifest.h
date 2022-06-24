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


#if TDE2_EDITORS_ENABLED

namespace TDEngine2
{
	class CResourcesBuildManifest;


	typedef struct TResourceBuildInfo: ISerializable
	{
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
			bool mIsLooped;
			U32 mStartRange = 0;
			U32 mEndRange = 0;
		};

		bool mIsSkinned = false;
		bool mImportTangents = true;

		std::vector<TAnimationClipImportInfo> mAnimations;
	} TMeshResourceBuildInfo, *TMeshResourceBuildInfoPtr;


	typedef struct TTexture2DResourceBuildInfo : TResourceBuildInfo
	{
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

	} TTexture2DResourceBuildInfo, *TTexture2DResourceBuildInfoPtr;


	/*!
		\brief A factory function for creation objects of CResourcesBuildManifest's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourcesBuildManifest's implementation
	*/

	TDE2_API CResourcesBuildManifest* CreateResourcesBuildManifest(E_RESULT_CODE& result);


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

			TDE2_API E_RESULT_CODE ForEachRegisteredResource(const std::function<bool(const TResourceBuildInfo&)>& action = nullptr);
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourcesBuildManifest)
		private:
			static constexpr U16    mVersionTag = 0x1;

			std::vector<std::unique_ptr<TResourceBuildInfo>> mpResourcesBuildConfigs;
	};
}

#endif