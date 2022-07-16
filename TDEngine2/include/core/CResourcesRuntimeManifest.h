/*!
	\file CResourcesRuntimeManifest.h
	\date 12.07.2022
	\authors Kasimov Ildar
*/

#pragma once


#include "IResourcesRuntimeManifest.h"
#include "CBaseObject.h"
#include <mutex>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CResourcesRuntimeManifest's type

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CResourcesRuntimeManifest's implementation
	*/

	TDE2_API IResourcesRuntimeManifest* CreateResourcesRuntimeManifest(E_RESULT_CODE& result);


	/*!
		class CResourcesRuntimeManifest

		\brief The class is used to configure resources at runtime
	*/

	class CResourcesRuntimeManifest : public CBaseObject, public IResourcesRuntimeManifest
	{
		public:
			friend TDE2_API IResourcesRuntimeManifest* CreateResourcesRuntimeManifest(E_RESULT_CODE&);
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

			TDE2_API E_RESULT_CODE AddResourceMeta(const std::string& resourceId, std::unique_ptr<TBaseResourceParameters> pResourceMeta) override;

			TDE2_API E_RESULT_CODE SetBaseResourcesPath(const std::string& value) override;

			/*!
				\return The method returns true if there is configuration for the given resource's identifier
			*/

			TDE2_API bool HasResourceMeta(const std::string& resourceId) const override;

			TDE2_API const TBaseResourceParameters* const GetResourceMeta(const std::string& resourceId) const override;
		private:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CResourcesRuntimeManifest)

		private:
			mutable std::mutex                                                        mMutex;

			static constexpr U16                                                      mVersionTag = 0x1;

			std::unordered_map<std::string, std::unique_ptr<TBaseResourceParameters>> mpResourcesMetaInfos;

			std::string                                                               mBaseResourcesPathPrefix;
	};
}