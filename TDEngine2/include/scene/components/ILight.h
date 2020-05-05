/*!
	\file ILight.h
	\date 04.05.2020
	\authors Kasimov Ildar
*/

#pragma once


#include "../../utils/Types.h"
#include "../../utils/Config.h"
#include "../../utils/Color.h"
#include "../../math/TVector3.h"
#include "../../ecs/IComponentFactory.h"


namespace TDEngine2
{
	/*!
		struct TBaseLightParameters

		\brief The structure contains parameters for creation of a base light's component
	*/

	typedef struct TBaseLightParameters : public TBaseComponentParameters
	{
		TColor32F mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		F32       mIntensity = 1.0f;
	} TBaseLightParameters;


	/*!
		interface ILight

		\brief The interface describes a common functionality of a light source
	*/

	class ILight
	{
		public:
			/*!
				\brief The method assigns color of the light

				\param[in] color A color of the light source

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetColor(const TColor32F& color) = 0;
			
			/*!
				\brief The method specifies intensity of the light

				\param[in] intensity An intensity of the light source

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetIntensity(F32 instensity) = 0;

			/*!
				\brief The method returns a color of the light source

				\return The method returns a color of the light source
			*/

			TDE2_API virtual const TColor32F& GetColor() const = 0;

			/*!
				\brief The method returns an intensity of the light source

				\return The method returns an intensity of the light source
			*/

			TDE2_API virtual F32 GetIntensity() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILight)
	};


	/*!
		struct TDirectionalLightParameters

		\brief The structure contains parameters for creation of a directional light's component
	*/

	typedef struct TDirectionalLightParameters : public TBaseLightParameters
	{
		TVector3 mDirection;
	} TDirectionalLightParameters;


	/*!
		interface IDirectionalLight

		\brief The interface describes a functionality of a directional light source component
	*/

	class IDirectionalLight : public virtual ILight
	{
		public:
			/*!
				\brief The method specifies direction of the light source

				\param[in] direction A normalized 3d vector that determines direction of the sun light

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetDirection(const TVector3& direction) = 0;
			
			/*!
				\brief The method returns a sun light's direction
				\return The method returns a sun light's direction
			*/

			TDE2_API virtual const TVector3& GetDirection() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IDirectionalLight)
	};


	/*!
		struct TPointLightParameters

		\brief The structure contains parameters for creation of a point light's component
	*/

	typedef struct TPointLightParameters : public TBaseLightParameters
	{
		F32 mRange = 1.0f;
	} TPointLightParameters;


	/*!
		interface IPointLight

		\brief The interface describes a functionality of a perspective camera component
	*/

	class IPointLight : public virtual ILight
	{
		public:
			/*!
				\brief The method specifies radius of the point light

				\param[in] range A maximum distance at which the light affects to objects

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE SetRange(F32 range) = 0;

			/*!
				\brief The method returns a radius of the point light
				\return The method returns a radius of the point light
			*/

			TDE2_API virtual F32 GetRange() const = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(IPointLight)
	};


	/*!
		interface ILightFactory

		\brief The interface represents a functionality of a factory of ILight objects
	*/

	class ILightFactory : public IComponentFactory
	{
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API virtual E_RESULT_CODE Init() = 0;
		protected:
			DECLARE_INTERFACE_PROTECTED_MEMBERS(ILightFactory)
	};
}