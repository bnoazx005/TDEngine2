/*!
	\file CBaseShaderCompiler.h
	\date 23.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IShaderCompiler.h"
#include "./../utils/Utils.h"
#include "./../utils/Types.h"
#include "./../utils/CResult.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	/*!
		class CBaseShaderCompiler

		\brief The class is a basis for all other compilers of shaders
	*/

	class CBaseShaderCompiler : public IShaderCompiler
	{
		protected:
			typedef std::pair<std::string, std::string>                         TShaderDefineDesc;

			typedef std::unordered_map<std::string, std::string>                TDefinesMap;

			typedef std::unordered_map<U8, std::unordered_map<std::string, U8>> TUniformBuffersMap;
		
			typedef struct TShaderMetadata
			{
				std::string             mVertexShaderEntrypointName;
				std::string             mPixelShaderEntrypointName;
				std::string             mGeometryShaderEntrypointName;

				E_SHADER_FEATURE_LEVEL  mFeatureLevel;

				TDefinesMap             mDefines;

				TUniformBuffersMap      mUniformBuffers;
			} TShaderMetadata;
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IFileSystem* pFileSystem) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseShaderCompiler)

			TDE2_API virtual const C8* _getShaderStageDefineName(E_SHADER_STAGE_TYPE shaderStage) const;

			TDE2_API virtual TShaderMetadata _parseShader(const std::string& sourceCode) const;

			TDE2_API virtual std::string _removeComments(const std::string& sourceCode) const;

			TDE2_API virtual TDefinesMap _processDefines(const std::string& sourceCode) const;

			TDE2_API virtual TUniformBuffersMap _processUniformBuffersDecls(const std::string& sourceCode) const = 0;

			TDE2_API virtual E_SHADER_FEATURE_LEVEL _getTargetVersionFromStr(const std::string& ver) const = 0;

			TDE2_API virtual bool _isShaderStageEnabled(E_SHADER_STAGE_TYPE shaderStage, const TShaderMetadata& shaderMeta) const;

			TDE2_API const C8* _getTargetVersionDefineName() const;
		protected:
			bool             mIsInitialized;

			static U32       mMaxStepsCount; ///< The value is used within _removeComments to bound a maximum number of steps of an automata

			static const C8* mEntryPointsDefineNames[3];

			static const C8* mTargetVersionDefineName;

			IFileSystem*     mpFileSystem;
	};
}