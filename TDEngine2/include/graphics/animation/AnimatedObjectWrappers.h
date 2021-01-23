/*!
	\file AnimatedValueWrappers.h
	\date 21.01.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "../../math/TVector3.h"
#include "../../math/TQuaternion.h"
#include "../../core/CBaseObject.h"


namespace TDEngine2
{
	/*!
		interface IAnimatedObjectWrapper

		\brief The interface describes an element that allows to get an access to internal values of engine's components.
		Set/Get methods are not fully safe to use, but we assume that's up to end programmer to work with them carefully
		to prevent these situations
	*/

	class IAnimatedObjectWrapper : public virtual IBaseObject
	{
		public:
			template <typename T>
			E_RESULT_CODE Set(const std::string& propertyName, const T& value)
			{
				return _setInternal(propertyName, static_cast<const void*>(&value), sizeof(value));
			}

			template <typename T>
			const T& Get(const std::string& propertyName) const
			{
				return *static_cast<const T*>(_getInternal(propertyName));
			}

		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IAnimatedObjectWrapper)

			TDE2_API virtual E_RESULT_CODE _setInternal(const std::string& propertyName, const void* pValue, size_t valueSize) = 0;
			TDE2_API virtual const void* _getInternal(const std::string& propertyName) const = 0;
	};


	/*!
		\brief Implement your own implementations of this class to provide access to
	*/

	class CBaseAnimatedObjectWrapper : public IAnimatedObjectWrapper, public CBaseObject
	{
		public:
			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CBaseAnimatedObjectWrapper)

			TDE2_API virtual E_RESULT_CODE _setInternal(const std::string& propertyName, const void* pValue, size_t valueSize) = 0;
			TDE2_API virtual const void* _getInternal(const std::string& propertyName) const = 0;
	};


	class IWorld;


	/*!
		\brief A factory function for creation objects of CAnimationClipLoader's type

		\param[in, out] pWorld A pointer to IWorld's implementation
		\param[in, out] entityId An identifier of an entity which transform should be animated 
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CAnimationClipLoader's implementation
	*/

	TDE2_API IAnimatedObjectWrapper* CreateAnimatedEntityTransformWrapper(IWorld* pWorld, TEntityId entityId, E_RESULT_CODE& result);

	/*!
		\brief CAnimatedEntityTransformWrapper
	*/

	class CAnimatedEntityTransformWrapper : public CBaseAnimatedObjectWrapper
	{
		public:
			friend TDE2_API IAnimatedObjectWrapper* CreateAnimatedEntityTransformWrapper(IWorld*, TEntityId, E_RESULT_CODE&);
		public:
			TDE2_API virtual E_RESULT_CODE Init(IWorld* pWorld, TEntityId entityId);
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CAnimatedEntityTransformWrapper)

			TDE2_API E_RESULT_CODE _setInternal(const std::string& propertyName, const void* pValue, size_t valueSize) override;
			TDE2_API const void* _getInternal(const std::string& propertyName) const override;
		protected:
			IWorld* mpWorld;

			TEntityId mEntityId;
	};
}