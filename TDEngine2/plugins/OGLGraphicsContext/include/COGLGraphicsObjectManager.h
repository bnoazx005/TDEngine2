/*!
	\file COGLGraphicsObjectManager.h
	\date 30.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include <graphics/CBaseGraphicsObjectManager.h>
#include <unordered_map>
#include <vector>


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of COGLGraphicsObjectManager's type

		\param[in, out] pGraphicsContext A pointer to IGraphicsContext's implementation

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to COGLGraphicsObjectManager's implementation
	*/

	TDE2_API IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);


	/*!
		class COGLGraphicsObjectManager

		\brief The class is an implementation of IGraphicsObjectManager for the OGL graphics context
	*/

	class COGLGraphicsObjectManager : public CBaseGraphicsObjectManager
	{
		public:
			friend TDE2_API IGraphicsObjectManager* CreateOGLGraphicsObjectManager(IGraphicsContext* pGraphicsContext, E_RESULT_CODE& result);
		protected:
			typedef std::vector<GLuint> TTextureSamplersArray;
		public:
			/*!
				\brief The method is a factory for creation objects of IVertexBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IVertexBuffer or an error code
			*/

			TDE2_API TResult<IVertexBuffer*> CreateVertexBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation objects of IIndexBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] indexFormatType A value, which defines single index's stride size

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IIndexBuffer or an error code
			*/

			TDE2_API TResult<IIndexBuffer*> CreateIndexBuffer(E_BUFFER_USAGE_TYPE usageType, E_INDEX_FORMAT_TYPE indexFormatType,
				U32 totalBufferSize, void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation objects of IConstantBuffer's type

				\param[in] usageType A usage type of a buffer

				\param[in] totalBufferSize Total size of a buffer

				\param[in] indexFormatType A value, which defines single index's stride size

				\param[in] pDataPtr A pointer to data that will initialize a buffer

				\return The result object contains either a pointer to IConstantBuffer or an error code
			*/

			TDE2_API TResult<IConstantBuffer*> CreateConstantBuffer(E_BUFFER_USAGE_TYPE usageType, U32 totalBufferSize, void* pDataPtr) override;

			/*!
				\brief The method is a factory for creation objects of IVertexDeclaration's type

				\return The result object contains either a pointer to IVertexDeclaration or an error code
			*/

			TDE2_API TResult<IVertexDeclaration*> CreateVertexDeclaration() override;

			/*!
				\brief The method is a factory for creation of texture samplers objects

				\param[in] samplerDesc A structure that contains parameters which will be assigned to a new created sampler

				\return The result object contains either an identifier of created sampler or an error code
			*/

			TDE2_API TResult<TTextureSamplerId> CreateTextureSampler(const TTextureSamplerDesc& samplerDesc) override;

			/*!
				\brief The method returns a handler of a texture sampler which is related with a given identifier

				\param[in] texSamplerId An identifier of a texture sampler

				\return Either a handler of a texture sampler or an error code
			*/

			TDE2_API TResult<GLuint> GetTextureSampler(TTextureSamplerId texSamplerId) const;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(COGLGraphicsObjectManager)

			TDE2_API E_RESULT_CODE _freeTextureSamplers() override;
		protected:
			TTextureSamplersArray mTextureSamplersArray;
	};
}