/*!
	\file IResourceFactory.h
	\date 24.10.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "IBaseObject.h"
#include "Serialization.h"
#include "./../utils/Types.h"
#include "./../utils/Utils.h"
#include <string>


namespace TDEngine2
{
	class IResourceManager;
	class IGraphicsContext;
	class IResource;


	/*!
		struct TBaseResourceParameters

		\brief The struct is used to pass different sets of parameters into
		Create method of IResourceFactory and IResourceManager respectively.
		Also the struct contains common parameters for all resources
	*/

	typedef struct TBaseResourceParameters: ISerializable
	{
		TDE2_API virtual ~TBaseResourceParameters() = default;

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

		E_RESOURCE_LOADING_POLICY mLoadingPolicy = E_RESOURCE_LOADING_POLICY::SYNCED;
	} TBaseResourceParameters, *TBaseResourceParametersPtr;


	/*!
		interface IResourceFactory

		\brief The interface describes a functionality of a resource factory
	*/

	class IResourceFactory: public virtual IBaseObject
	{
		public:
			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* Create(const std::string& name, const TBaseResourceParameters& params) const = 0;

			/*!
				\brief The method creates a new instance of a resource based on passed parameters

				\param[in] name A name of a resource

				\param[in] params An object that contains parameters that are needed for the resource's creation

				\return A pointer to a new instance of IResource type
			*/

			TDE2_API virtual IResource* CreateDefault(const std::string& name, const TBaseResourceParameters& params) const = 0;

			/*!
				\brief The method returns an identifier of a resource's type, which
				the factory serves

				\return The method returns an identifier of a resource's type, which
				the factory serves
			*/

			TDE2_API virtual TypeId GetResourceTypeId() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IResourceFactory)
	};


	/*!
		\brief The template is used to declare interfaces of factories for concrete resources types
	*/

	template <typename... TInitArgs>
	class IGenericResourceFactory : public IResourceFactory
	{
		public:
			/*!
				\brief The method initializes an inner state of an object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init(TInitArgs... args) = 0;
	};
}
