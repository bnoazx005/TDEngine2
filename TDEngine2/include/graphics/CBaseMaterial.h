/*!
	\file CBaseMaterial.h
	\date 18.12.2018
	\author Kasimov Ildar
*/

#pragma once


#include "IMaterial.h"
#include "./../core/CBaseResource.h"
#include "./../utils/CResult.h"
#include <string>
#include <unordered_map>


namespace TDEngine2
{
	class IShader;
	class IBinaryFileReader;
	class IResourceHandler;


	constexpr U8 BaseMaterialFileTagLength = 3;


	/*!
		\brief The description of a basic material's file structure 

		A material file is a binary file that contains parameters of
		a CBaseMaterial type. In other words its serialized representation of
		an object of CBaseMaterial type.

		This file has the following structure:

		- File header (8 bytes):
			-- MAT tag							(3 bytes)	[ASCII tag tells that the file can be considered as actual material file that is used within TDEngine2]
			-- endian type						(1 byte)    [The flag either 1 or 0, if it equals to 1 than big-endian order is used, little-endian is used otherwise]
			-- shader entries block offset		(4 bytes)	[An offset from the beginning of a file from which the shader entries block is placed]
			-- material properties block offest	(4 bytes)	[An offset from the beginning of a file from which the material properties block is placed]

		- Shader Stage Entry (8 + N bytes, because of padding)
			-- Type						(1 byte)	[A type of a shader stage (0 - vertex, 1 - fragment (pixel), 2 - geometry]
			-- Shader name's length		(4 bytes)	[The member stores a length of a shader's filename]
			-- Shader's name			(N bytes)	[A name of a shader including full path (null-terminated string)]

		- Material property entry (bytes)
			-- constant buffer's index	(1 byte)	[Identifier of a constant buffer]
			-- constant buffer's size	(4 byte)	[A size of a constant buffer]

		Note that the file header is placed at the end of a file (last 8 bytes). Each data block is placed in continuous fashion, but its position can vary from one file to another.
	*/
	

	/*!
		struct TBaseMaterialFileHeader

		\brief The structure describes a members of a material file's header
	*/

	typedef struct TBaseMaterialFileHeader
	{
		C8  mTag[BaseMaterialFileTagLength];

		U8  mEndianType;

		U32 mShaderEntriesBlockOffset;

		U32 mMaterialPropertiesBlockOffset;
	} TBaseMaterialFileHeader, *TBaseMaterialFileHeaderPtr;


	/*!
		struct TBaseMaterialShaderEntry

		\brief The structure contains shader stages data of a particular material
	*/

	typedef struct TBaseMaterialShaderEntry
	{
		U8          mStageType;

		U32         mShaderNameLength;

		std::string mShaderName;
	} TBaseMaterialShaderEntry, *TBaseMaterialShaderEntryPtr;


	/*!
		struct TBaseMaterialPropertyEntry

		\brief The structure contains properties of a material that are specified
		by the user
	*/

	typedef struct TBaseMaterialPropertyEntry
	{
		U8  mConstantBufferSlot;

		U32 mConstantBufferSize;
	} TBaseMaterialPropertyEntry, *TBaseMaterialPropertyEntryPtr;


	/*!
		\brief A factory function for creation objects of CBaseMaterial's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] id An identifier of a resource

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterial's implementation
	*/

	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);


	/*!
		\brief A factory function for creation objects of CBaseMaterial's type

		\param[in, out] pResourceManager A pointer to IGraphicsContext's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in] name A resource's name

		\param[in] params A parameters of created material

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterial's implementation
	*/

	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
										   const TMaterialParameters& params, E_RESULT_CODE& result);


	/*!
		class CBaseMaterial

		\brief The class represents a default implementation of a material
		which is used within the engine
	*/

	class CBaseMaterial: public CBaseResource, public IMaterial
	{
		public:
			friend 	TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name, E_RESULT_CODE& result);

			friend TDE2_API IMaterial* CreateBaseMaterial(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name,
														  const TMaterialParameters& params, E_RESULT_CODE& result);
		protected:
			typedef std::unordered_map<std::string, ITexture*> TTexturesHashTable;
		public:
			TDE2_REGISTER_TYPE(CBaseMaterial)

			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) override;

			/*!
				\brief The method loads resource data into memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Load() override;

			/*!
				\brief The method unloads resource data from memory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Unload() override;

			/*!
				\brief The method resets current internal data of a resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Reset() override;

			/*!
				\brief The method assigns a shader object with a given name to the material

				\param[in] shaderName Shader's identifier
			*/

			TDE2_API void SetShader(const std::string& shaderName) override;

			/*!
				\brief The method specifies whether the material should be transparent or not

				\param[in] isTransparent A new state of the material
			*/

			TDE2_API void SetTransparentState(bool isTransparent) override;

			/*!
				\brief The method sets up source and destination blending factors if blending stage is enabled

				\param[in] srcFactor Source blending factor
				\param[in] destFactor Destination blending factor
			*/

			TDE2_API void SetBlendFactors(const E_BLEND_FACTOR_VALUE& srcFactor, const E_BLEND_FACTOR_VALUE& destFactor,
										  const E_BLEND_FACTOR_VALUE& srcAlphaFactor = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA,
										  const E_BLEND_FACTOR_VALUE& destAlphaFactor = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA) override;

			/*!
				\brief The methods sets up blending operation for the material

				\param[in] opType An operation that should be executed over source and destination colors
				\param[in] alphaOpType An operation that should be executed over source and destination alpha values
			*/

			TDE2_API void SetBlendOp(const E_BLEND_OP_TYPE& opType, const E_BLEND_OP_TYPE& alphaOpType = E_BLEND_OP_TYPE::ADD) override;

			/*!
				\brief The method binds a material to a rendering pipeline
			*/

			TDE2_API void Bind() override;

			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture) override;

			/*!
				\brief The method returns a pointer to IResourceHandler of an attached shader

				\return The method returns a pointer to IResourceHandler of an attached shader
			*/

			TDE2_API IResourceHandler* GetShaderHandler() const override;

			/*!
				\brief The method returns true if the material's instance uses alpha blending
				based transparency

				\return The method returns true if the material's instance uses alpha blending
				based transparency
			*/

			TDE2_API bool IsTransparent() const override;

			/*!
				\brief The static function is a helper which is used for sorting materials based on their type
				either opaque or transparent 

				\param[in] pLeft A pointer to IMaterial implementation
				\param[in] pRight A pointer to IMaterial implementation

				\return The function returns true if the pair is ordered correctly
			*/

			TDE2_API static bool AlphaBasedMaterialComparator(const IMaterial* pLeft, const IMaterial* pRight);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMaterial)

			TDE2_API E_RESULT_CODE _setVariable(const std::string& name, const void* pValue, U32 size) override;
		protected:
			IGraphicsContext*      mpGraphicsContext;

			IResourceHandler*      mpShader;

			TUserUniformBufferData mpUserUniformsData[MaxNumberOfUserConstantBuffers];

			TTexturesHashTable     mpAssignedTextures;

			TBlendStateDesc        mBlendStateParams;

			TBlendStateId          mBlendStateHandle = InvalidBlendStateId;
	};


	/*!
		\brief A factory function for creation objects of CBaseMaterialLoader's type

		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[in, out] pFileSystem A pointer to IFileSystem's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterialLoader's implementation
	*/

	TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
													   E_RESULT_CODE& result);


	/*!
		class CBaseMaterialLoader

		\brief The class implements a functionality of a base material loader
	*/

	class CBaseMaterialLoader : public IMaterialLoader
	{
		public:
			friend TDE2_API IResourceLoader* CreateBaseMaterialLoader(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem,
																	  E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) override;
			
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method loads data into the specified resource based on its
				internal information

				\param[in, out] pResource A pointer to an allocated resource

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE LoadResource(IResource* pResource) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the loader serves

				\return The method returns an identifier of a resource's type, which
				the loader serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMaterialLoader)

			TDE2_API TResult<TBaseMaterialFileHeader> _readMaterialFileHeader(IBinaryFileReader* pFileReader) const;
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IFileSystem*      mpFileSystem;

			IGraphicsContext* mpGraphicsContext;
	};


	/*!
		\brief A factory function for creation objects of CBaseMaterialFactory's type
		
		\param[in, out] pResourceManager A pointer to IResourceManager's implementation

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CBaseMaterialFactory's implementation
	*/

	TDE2_API IResourceFactory* CreateBaseMaterialFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class CBaseMaterialFactory

		\brief The class is an abstract factory of CBaseMaterial objects that
		is used by a resource manager
	*/

	class CBaseMaterialFactory : public IMaterialFactory
	{
		public:
			friend TDE2_API IResourceFactory* CreateBaseMaterialFactory(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext,E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* Create(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const override;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API U32 GetResourceTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseMaterialFactory)
		protected:
			bool              mIsInitialized;

			IResourceManager* mpResourceManager;

			IGraphicsContext* mpGraphicsContext;
	};
}
