/*!
	\file IShaderCompiler.h
	\date 22.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IBaseObject.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	class IFileSystem;


	/*!
		enum class E_UNIFORM_BUFFER_DESC_FLAGS

		\brief The enumeration contains all available flags that can be assigned to a buffer's description
	*/

	enum class E_UNIFORM_BUFFER_DESC_FLAGS: U32
	{
		UBDF_INTERNAL = 0x1,		///< The flag shows whether the uniform buffer is internal or it's available for user 
		UBDF_EMPTY    = 0x0
	};

	TDE2_DECLARE_BITMASK_OPERATORS_INTERNAL(E_UNIFORM_BUFFER_DESC_FLAGS)


	/*!
		enum class E_SHADER_RESOURCE_TYPE

		\brief The enumeration contains a list of all available shader resources types
	*/

	enum class E_SHADER_RESOURCE_TYPE : U8
	{
		SRT_TEXTURE2D,
		SRT_TEXTURE3D,
		SRT_TEXTURE2D_ARRAY,
		SRT_TEXTURECUBE,
		SRT_SAMPLER_STATE,
		SRT_RW_IMAGE2D,
		SRT_STRUCTURED_BUFFER,
		SRT_RW_STRUCTURED_BUFFER,
		SRT_UNKNOWN
	};


	/*!
		struct TShaderResourceDesc

		\brief The structure contains an information that describes a shader resource 
	*/


	typedef struct TShaderResourceDesc
	{
		E_SHADER_RESOURCE_TYPE mType;

		U8                     mSlot;
	} TShaderResourceDesc, *TShaderResourceDescPtr;


	/*!
		struct TShaderUniformDesc

		\brief The structure contains meta information about a single shader's uniform
	*/

	typedef struct TShaderUniformDesc
	{
		std::string mName;

		USIZE       mOffset = 0; ///< \note Stores an offset from the beginning of a uniforms buffer
		USIZE       mSize   = 0;
		TypeId      mTypeId = TypeId::Invalid;
		bool        mIsArray = false;
	} TShaderUniformDesc, *TShaderUniformDescPtr;



	/*!
		struct TUniformBufferDesc

		\brief The structure contains an information about a uniform buffer
	*/

	typedef struct TUniformBufferDesc
	{
		U8                              mSlot;

		USIZE                           mSize;

		E_UNIFORM_BUFFER_DESC_FLAGS     mFlags;			/// The field is used to provide additional information about a buffer

		U32                             mBufferIndex; /// for OpenGL usage only

		std::vector<TShaderUniformDesc> mVariables;
	} TUniformBufferDesc, *TUniformBufferDescPtr;


	typedef struct TShaderCacheBytecodeEntry
	{
		USIZE mOffset = 0; ///< start position of the bytecode within the shader cache blob
		USIZE mSize = 0; ///< size of the bytecode
	} TShaderCacheBytecodeEntry, *TShaderCacheBytecodeEntryPtr;


	/*!
		struct TShaderCompilerOutput

		\brief The structure contains shader compiler's output data
	*/

	typedef struct TShaderCompilerOutput
	{
		virtual ~TShaderCompilerOutput() = default;

		std::unordered_map<std::string, TUniformBufferDesc>  mUniformBuffersInfo; /// first key is a buffer's name, the value is the buffer's slot index and its size

		std::unordered_map<std::string, TShaderResourceDesc> mShaderResourcesInfo;	/// the key is a resource's name, the value is an information about resource  
	} TShaderCompilerOutput, *TShaderCompilerOutputPtr;


	/*!
		interface IShaderCompiler

		\brief The interface represents a functionality of a shaders compiler
	*/

	class IShaderCompiler : public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an initial state of a compiler

				\param[in, out] pFileSystem A pointer to IFileSystem implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IFileSystem* pFileSystem) = 0;

			/*!
				\brief The method compiles specified source code into the bytecode representation.
				Note that the method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually

				\param[in] source A string that contains a source code of a shader

				\return An object that contains either bytecode or some error code. Note that the
				method allocates memory for TShaderCompilerOutput object on heap so it should be
				released manually
			*/

			TDE2_API virtual TResult<TShaderCompilerOutput*> Compile(const std::string& source) const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderCompiler)
	};
}