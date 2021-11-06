/*!
	\file IMaterial.h
	\date 18.12.2018
	\author Kasimov Ildar
*/

#pragma once


#include "../utils/Types.h"
#include "../utils/Utils.h"
#include "../core/IResourceLoader.h"
#include "../core/IResourceFactory.h"
#include "../core/Serialization.h"
#include "../core/IBaseObject.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IFileSystem;
	class ITexture;
	class IMaterialInstance;


	TDE2_DECLARE_SCOPED_PTR(IMaterialInstance)


	TDE2_DECLARE_HANDLE_TYPE(TMaterialInstanceId)

	constexpr TMaterialInstanceId DefaultMaterialInstanceId = TMaterialInstanceId(0);


	enum class E_GEOMETRY_SUBGROUP_TAGS : U32;


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

		TDE2_API TMaterialParameters(const std::string& shaderName, bool isTransparent = false, const TDepthStencilStateDesc& depthStencilState = {},
									 const TRasterizerStateDesc& rasterizerState = {},
									 const TBlendStateDesc& blendState = {});

		std::string            mShaderName;

		TBlendStateDesc        mBlendingParams;

		TDepthStencilStateDesc mDepthStencilParams;

		TRasterizerStateDesc   mRasterizerParams;
	} TMaterialParameters, *TMaterialParametersPtr;


	/*!
		interface IMaterial

		\brief The interface describes a functionality of a material
	*/

	class IMaterial: public ISerializable, public virtual IBaseObject
	{
		public:
			TDE2_REGISTER_TYPE(IMaterial);

			/*!
				\brief The method initializes an internal state of a material

				\param[in, out] pResourceManager A pointer to IResourceManager's implementation

				\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

				\param[in] name A resource's name
				
				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IResourceManager* pResourceManager, IGraphicsContext* pGraphicsContext, const std::string& name) = 0;

			/*!
				\brief The method creates a new instance of this material. Remember that the material instance is
				intended for runtime usage only 

				\return A pointer to IMaterialInstance implementation which represents a new instance of this material
			*/

			TDE2_API virtual TPtr<IMaterialInstance> CreateInstance() = 0;

			/*!
				\brief The method assigns a shader object with a given name to the material

				\param[in] shaderName Shader's identifier
			*/

			TDE2_API virtual void SetShader(const std::string& shaderName) = 0;

			/*!
				\brief The method binds a material to a rendering pipeline

				\param[in] materialInstanceId An identifier of an instance of this material. 0 means a default instance, which is used by default
			*/

			TDE2_API virtual void Bind(TMaterialInstanceId instanceId = DefaultMaterialInstanceId) = 0;
			
			/*!
				\brief The method assigns a given texture to a given resource's name

				\param[in] resourceName A name of a resource within a shader's code

				\param[in, out] pTexture A pointer to ITexture implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetTextureResource(const std::string& resourceName, ITexture* pTexture, TMaterialInstanceId instanceId = DefaultMaterialInstanceId) = 0;
			
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
				\brief The method sets a given value to shader's uniform variable. Be aware that 
				this version of SetVariable creates a new instance of the material each time you call it

				\param[in] name A name of user's uniform variable within a shader
				\param[in] value A value that should be assigned into the given variable

				\return A new material instance if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API TResult<TPtr<IMaterialInstance>> SetVariable(const std::string& name, const T& value)
			{
				// \todo add validation of an input type

				return _setVariable(name, static_cast<const void*>(&value), sizeof(T));
			}

			/*!
				\brief The method sets a given value to shader's uniform variable. This overloaded version doesn't
				create a new instance of the material

				\param[in] instanceId An identifier of a material instance for which we make this assignment
				\param[in] name A name of user's uniform variable within a shader
				\param[in] value A value that should be assigned into the given variable

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE SetVariableForInstance(TMaterialInstanceId instanceId, const std::string& name, const T& value)
			{
				return _setVariableForInstance(instanceId, name, static_cast<const void*>(&value), sizeof(T));
			}

			TDE2_API virtual E_RESULT_CODE SetVariableForInstance(TMaterialInstanceId instanceId, const std::string& name, const void* pValue, U32 size) = 0;

			/*!
				\brief The method sets up a state of depth buffer usage

				\param[in] state The argument defines whether or not the depth buffer is used
			*/

			TDE2_API virtual void SetDepthBufferEnabled(bool state) = 0;
			
			/*!
				\brief The method sets up a state of stencil buffer usage

				\param[in] state The argument defines whether or not the stencil buffer is used
			*/

			TDE2_API virtual void SetStencilBufferEnabled(bool state) = 0;

			/*!
				\brief The method allows to enable or disable writing into the depth buffer

				\param[in] state The argument defines whether or not shader can write into the depth buffer
			*/

			TDE2_API virtual void SetDepthWriteEnabled(bool state) = 0;

			/*!
				\brief The method specifies the type of a comparison function for depth testing
			*/

			TDE2_API virtual void SetDepthComparisonFunc(const E_COMPARISON_FUNC& funcType) = 0;

			/*!
				\brief The method specifies read mask for stencil operations

				\param[in] value A value of the stencil read mask
			*/

			TDE2_API virtual void SetStencilReadMask(U8 value) = 0;

			/*!
				\brief The method specifies write mask for stencil operations

				\param[in] value A value of the stencil write mask
			*/

			TDE2_API virtual void SetStencilWriteMask(U8 value) = 0;

			TDE2_API virtual void SetStencilFrontOp(const TStencilOperationDesc& op) = 0;
			TDE2_API virtual void SetStencilBackOp(const TStencilOperationDesc& op) = 0;

			/*!
				\brief The method specifies culling mode for faces for the current material

				\param[in] cullMode A value of E_CULL_MODE type which defines which face should be culled
			*/

			TDE2_API virtual void SetCullMode(const E_CULL_MODE& cullMode) = 0;

			/*!
				\brief The method allows to enable or disable scissor test, which by defaul is disabled

				\param[in] state The argument defines whether or not scissor test will be used
			*/

			TDE2_API virtual void SetScissorEnabled(bool state) = 0;
			
			/*!
				\brief The method enables\disables wireframe output of primitives. Note that not all graphics context
				support this kind of output especially mobile ones

				\param[in] state The argument defines whether or not primitives should be rendered in wireframe mode
			*/

			TDE2_API virtual void SetWireframeMode(bool state) = 0;

			/*!
				\brief The method enables\disables front CCW order for culling faces
				
				\param[in] state If true than front CCW order is used
			*/

			TDE2_API virtual void SetFrontCCWOrderEnabled(bool state) = 0;

			/*!
				\brief The method specifies depth bias and maximal depth bias

				\param[in] bias A depth value added to a given pixel
				\param[in] maxBias Maximum depth bias of a pixel
			*/

			TDE2_API virtual void SetDepthBias(F32 bias, F32 maxBias) = 0;

			TDE2_API virtual void SetDepthClippingEnabled(bool value) = 0;

			/*!
				\brief The method specifies sub-group within used geometry group

				\param[in] tag A tag which specifies a sub-group within used geometry group
			*/

			TDE2_API virtual void SetGeometrySubGroupTag(const E_GEOMETRY_SUBGROUP_TAGS& tag) = 0;

			/*!
				\brief The method returns hash value which corresponds to a given variable's name

				\return The method returns hash value which corresponds to a given variable's name
			*/

			TDE2_API virtual U32 GetVariableHash(const std::string& name) const = 0;

			/*!
				\brief The method returns an identifier of attached shader of an attached shader

				\return The method returns a pointer to IResourceHandler of an attached shader
			*/

			TDE2_API virtual TResourceId GetShaderHandle() const = 0;

			/*!
				\brief The method returns true if the material's instance uses alpha blending
				based transparency

				\return The method returns true if the material's instance uses alpha blending
				based transparency
			*/

			TDE2_API virtual bool IsTransparent() const = 0;

			/*!
				\brief The method returns assigned tag 

				\return The method returns assigned tag 
			*/

			TDE2_API virtual const E_GEOMETRY_SUBGROUP_TAGS& GetGeometrySubGroupTag() const = 0;

			/*!
				\brief The method returns either a pointer to IMaterialInstance or an error code 

				\return The method returns either a pointer to IMaterialInstance or an error code
			*/

			TDE2_API virtual TResult<TPtr<IMaterialInstance>> GetMaterialInstance(TMaterialInstanceId instanceId) const = 0;

			/*!
				\param[in] id An identifier of a texture (Empty literal will return first (main) texture)

				\return The pointer to ITexture resource
			*/

			TDE2_API virtual ITexture* GetTextureResource(const std::string& id, TMaterialInstanceId instanceId = DefaultMaterialInstanceId) const = 0;

			TDE2_API virtual bool IsScissorTestEnabled() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterial)

			TDE2_API virtual TResult<TPtr<IMaterialInstance>> _setVariable(const std::string& name, const void* pValue, U32 size) = 0;
			TDE2_API virtual E_RESULT_CODE _setVariableForInstance(TMaterialInstanceId instanceId, const std::string& name, const void* pValue, U32 size) = 0;
	};


	TDE2_DECLARE_SCOPED_PTR(IMaterial)


	class IMaterialLoader : public IGenericResourceLoader<IResourceManager*, IGraphicsContext*, IFileSystem*> {};
	class IMaterialFactory : public IGenericResourceFactory<IResourceManager*, IGraphicsContext*> {};


	/*!
		interface IMaterialInstance

		\brief The interface describes a functionality of a material's instance which is an identifier
		of set of particular material's parameters
	*/

	class IMaterialInstance: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method initializes an internal state of the instance

				\param[in, out] pMaterial A pointer to IMaterial implementation, which is a shared material for this instance
				\param[in] id An identifier of the instance, which should be defined by a material

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IMaterial* pMaterial, TMaterialInstanceId id) = 0;

			/*!
				\brief The method binds a material's instance to a rendering pipeline. Prefer to use this one
				instead of using IMaterial::Bind(TMaterialInstanceId) directly
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
				\brief The method sets a given value to shader's uniform variable

				\param[in] name A name of user's uniform variable within a shader
				\param[in] value A value that should be assigned into the given variable

				\return A new material instance if everything went ok, or some other code, which describes an error
			*/

			template <typename T>
			TDE2_API E_RESULT_CODE SetVariable(const std::string& name, const T& value)
			{
				return _setVariable(name, static_cast<const void*>(&value), sizeof(T));
			}

			/*!
				\brief The method returns an identifier of the instance

				\return The method returns an identifier of the instance
			*/

			TDE2_API virtual TMaterialInstanceId GetInstanceId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IMaterialInstance)

			TDE2_API virtual E_RESULT_CODE _setVariable(const std::string& name, const void* pValue, U32 size) = 0;
	};


	TDE2_DECLARE_SCOPED_PTR(IMaterialInstance)
}
