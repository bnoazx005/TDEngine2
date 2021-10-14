/*!
	\file CProjectSettings.h
	\date 07.10.2021
	\author Kasimov Ildar
*/

#pragma once


#include "CBaseObject.h"
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
				E_GRAPHICS_CONTEXT_GAPI_TYPE mGraphicsContextType = E_GRAPHICS_CONTEXT_GAPI_TYPE::GCGT_DIRECT3D11;
			} mGraphicsSettings;

			struct
			{
				E_AUDIO_CONTEXT_API_TYPE mAudioContextType = E_AUDIO_CONTEXT_API_TYPE::FMOD;
			} mAudioSettings;

			struct
			{
				U32 mMaxNumOfWorkerThreads = std::thread::hardware_concurrency() - 1;
				U32 mTotalPreallocatedMemorySize = DefaultGlobalMemoryBlockSize;

				std::string mApplicationName;
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
	};
}