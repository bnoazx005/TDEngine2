/*!
	\file CQuadSprite.h
	\date 18.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ISprite.h"
#include "./../ecs/CBaseComponent.h"


namespace TDEngine2
{
	/*!
		\brief A factory function for creation objects of CQuadSprite's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CQuadSprite's implementation
	*/

	TDE2_API IComponent* CreateQuadSprite(E_RESULT_CODE& result);


	/*!
		interface ISprite

		\brief The interface describes a functionality of a 2D sprite component
		By now only in plans only quad sprites support
	*/

	class CQuadSprite: public ISprite, public CBaseComponent
	{
		public:
			friend TDE2_API IComponent* CreateQuadSprite(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CQuadSprite)

			/*!
				\brief The method initializes an internal state of a quad sprite

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

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

			/*!
				\brief The method sets up an identifier fo a material that will be used for the sprite

				\param[in] materialName A string that contains material's name

				\param[in] materialId An identifier of a material
			*/

			TDE2_API void SetMaterialName(const std::string& materialName) override;

			/*!
				\brief The method sets up a color for a sprite

				\param[in] color A color that will be specified for a sprite
			*/

			TDE2_API void SetColor(const TColor32F& color) override;

			/*!
					\brief The method returns an identifier of used material

					\return The method returns an identifier of used material
			*/

			TDE2_API const std::string& GetMaterialName() const override;

			/*!
				\brief The method returns a color of a sprite

				\return The method returns a color of a sprite
			*/

			TDE2_API const TColor32F& GetColor() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CQuadSprite)
		protected:
			std::string mMaterialName;

			TColor32F   mColor;
			/*!
				\todo a sprite should contains
				- ref to atlas 
				- curr image id
				- curr material
				- flip X Y?
				- graphics layer id ?
			*/
	};

	
	/*!
		struct TQuadSpriteParameters

		\brief The structure contains parameters for creation of CQuadSprite
	*/

	typedef struct TQuadSpriteParameters : public TBaseComponentParameters
	{
	} TQuadSpriteParameters;


	/*!
		\brief A factory function for creation objects of CQuadSpriteFactory's type.

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CQuadSpriteFactory's implementation
	*/

	TDE2_API IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result);


	/*!
		class CQuadSpriteFactory

		\brief The class is factory facility to create a new objects of CQuadSprite type
	*/

	class CQuadSpriteFactory : public ISpriteFactory, public CBaseObject
	{
		public:
			friend TDE2_API IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an internal state of a factory

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init() override;

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* Create(const TBaseComponentParameters* pParams) const override;

			/*!
				\brief The method creates a new instance of a component based on passed parameters

				\param[in] pParams An object that contains parameters that are needed for the component's creation

				\return A pointer to a new instance of IComponent type
			*/

			TDE2_API IComponent* CreateDefault(const TBaseComponentParameters& params) const override;

			/*!
				\brief The method returns an identifier of a component's type, which
				the factory serves

				\return The method returns an identifier of a component's type, which
				the factory serves
			*/

			TDE2_API TypeId GetComponentTypeId() const override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CQuadSpriteFactory)
	};
}