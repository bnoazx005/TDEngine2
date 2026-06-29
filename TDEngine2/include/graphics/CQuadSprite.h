/*!
	\file CQuadSprite.h
	\date 18.11.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "../utils/Utils.h"
#include "../utils/Color.h"
#include "../ecs/CBaseComponent.h"


namespace TDEngine2
{
	CLASS_META(SECTION = ecs, flags = SERIALIZE_MARKED_ONLY_FIELDS)
	struct TQuadSpriteComponentData
	{
		FIELD_META(name= material) std::string mMaterialName;

		FIELD_META(name = color) TColor32F     mColor;

		/*!
			\todo a sprite should contains
			- ref to atlas
			- curr image id
			- curr material
			- flip X Y?
			- graphics layer id ?
		*/

		TDE2_DECLARE_COMPONENT_META(TQuadSpriteComponentData);
	};


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

	class CQuadSprite: public CBaseComponentT<CQuadSprite, TQuadSpriteComponentData>
	{
		public:
			friend TDE2_API IComponent* CreateQuadSprite(E_RESULT_CODE& result);
		public:
			TDE2_REGISTER_COMPONENT_TYPE(CQuadSprite)
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CQuadSprite)
	};

	
	/*!
		struct TQuadSpriteParameters

		\brief The structure contains parameters for creation of CQuadSprite
	*/

	typedef struct TQuadSpriteParameters : public TBaseComponentParameters
	{
	} TQuadSpriteParameters;


	TDE2_DECLARE_COMPONENT_FACTORY(QuadSprite, TQuadSpriteParameters);
}