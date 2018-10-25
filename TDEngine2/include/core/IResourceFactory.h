/*!
	\file IResourceFactory.h
	\date 24.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;


	/*!
		struct TBaseResourceParameters

		\brief The struct is used to pass different sets of parameters into
		Create method of IResourceFactory and IResourceManager respectively.
		Also the struct contains common parameters for all resources
	*/

	typedef struct TBaseResourceParameters
	{
		IResourceManager* mpResourceManager;

		std::string       mName;
					      
		TResourceId       mId;
	} TBaseResourceParameters, *TBaseResourceParametersPtr;


	/*!
		struct TShaderParameters

		\brief The stucture contains fields for creation IShader objects
	*/

	typedef struct TShaderParameters: TBaseResourceParameters
	{
		IGraphicsContext* mpGraphicsContext;
	} TShaderParameters, *TShaderParametersPtr;


	/*!
		interface IResourceFactory

		\brief The interface describes a functionality of a resource factory
	*/

	class IResourceFactory: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* Create(const TBaseResourceParameters* pParams) const = 0;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* CreateDefault(const TBaseResourceParameters& params) const = 0;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API virtual U32 GetResourceTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceFactory)
	};


	class IShaderFactory: public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a shader factory

				\param[in] pGraphicsContext A pointer to IGraphicsContext's implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(IGraphicsContext* pGraphicsContext) = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IShaderFactory)
	};
}
