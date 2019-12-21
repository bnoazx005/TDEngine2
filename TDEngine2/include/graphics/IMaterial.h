/*!
	\file IMaterial.h
	\date 18.12.2018
	\author Kasimov Ildar
*/

#pragma once


#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include "./../core/IResourceLoader.h"
#include "./../core/IResourceFactory.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class IResourceHandler;
	class ITexture;


	/*!
		struct TMaterialParameters

		\brief The stucture contains fields for creation IMaterial objects
	*/

	typedef struct TMaterialParameters : TBaseResourceParameters
	{
		TDE2_API TMaterialParameters() = default;

		/*!
			\brief The main constructor of the struct

			\note TMaterialParameters's definition is placed in CBaseMaterial.cpp file
		*/

		TDE2_API TMaterialParameters(const std::string& shaderName, bool isTransparent = false);

		std::string     mShaderName;

		TBlendStateDesc mBlendingParams;
	} TMaterialParameters, *TMaterialParametersPtr;


	/*!
		interface IMaterial

		\brief The interface describes a functionality of a material
	*/

	class IMaterial
	{
		public:
			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method assigns a shader object with a given name to the material

				\param[in] shaderName Shader's identifier
			*/

			TDE2_API virtual void SetShader(const std::string& shaderName) = 0;

			/*!
				\brief The method binds a material to a rendering pipeline
			*/

			TDE2_API virtual void Bind() = 0;
			
			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture) = 0;

			/*!
				\brief The method specifies whether the material should be transparent or not

				\param[in] isTransparent A new state of the material
			*/

			TDE2_API virtual void SetTransparentState(bool isTransparent) = 0;

			/*!
				\brief The method sets up source and destination blending factors if blending stage is enabled

				\param[in] srcFactor Source blending factor
				\param[in] destFactor Destination blending factor
				\param[in] srcAlphaFactor Source's alpha factor
				\param[in] destAlphaFactor Destination's alpha factor
			*/

			TDE2_API virtual void SetBlendFactors(const E_BLEND_FACTOR_VALUE& srcFactor, const E_BLEND_FACTOR_VALUE& destFactor,
												  const E_BLEND_FACTOR_VALUE& srcAlphaFactor = E_BLEND_FACTOR_VALUE::SOURCE_ALPHA,
												  const E_BLEND_FACTOR_VALUE& destAlphaFactor = E_BLEND_FACTOR_VALUE::ONE_MINUS_SOURCE_ALPHA) = 0;

			/*!
				\brief The methods sets up blending operation for the material

				\param[in] opType An operation that should be executed over source and destination colors
				\param[in] alphaOpType An operation that should be executed over source and destination alpha values
			*/

			TDE2_API virtual void SetBlendOp(const E_BLEND_OP_TYPE& opType, const E_BLEND_OP_TYPE& alphaOpType = E_BLEND_OP_TYPE::ADD) = 0;

			/*!
				\brief The method sets a given value to shader's uniform variable

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE SetVariable(const std::string& name, const T& value)
			{
				// \todo add validation of an input type

				return _setVariable(name, static_cast<const void*>(&value), sizeof(T));
			}

			/*!
				\brief The method returns a pointer to IResourceHandler of an attached shader

				\return The method returns a pointer to IResourceHandler of an attached shader
			*/

			TDE2_API virtual IResourceHandler* GetShaderHandler() const = 0;

			/*!
				\brief The method returns true if the material's instance uses alpha blending
				based transparency

				\return The method returns true if the material's instance uses alpha blending
				based transparency
			*/

			TDE2_API virtual bool IsTransparent() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterial)

			TDE2_API virtual E_RESULT_CODE _setVariable(const std::string& name, const void* pValue, U32 size) = 0;
	};


	/*!
		interface IMaterialLoader

		\brief The interface describes a functionality of a material loader
	*/

	class IMaterialLoader : public IResourceLoader
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in, out] pFileSystem A pointer to IFileSystem's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, IFileSystem* pFileSystem) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterialLoader)
	};


	/*!
		interface IMaterialFactory

		\brief The interface describes a functionality of a material factory
	*/

	class IMaterialFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a material factory

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterialFactory)
	};
}
