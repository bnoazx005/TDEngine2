#include "../../include/graphics/CQuadSprite.h"
#define META_EXPORT_ECS_SECTION
#include "../../include/metadata.h"


namespace TDEngine2
{
	TDE2_REGISTER_COMPONENT_FACTORY(CreateQuadSpriteFactory)
	TDE2_DEFINE_COMPONENT_META(TQuadSpriteComponentData)


	CQuadSprite::CQuadSprite() :
		CBaseComponentT()
	{
	}


	IComponent* CreateQuadSprite(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponent, CQuadSprite, result);
	}


	/*!
		\brief CQuadSpriteFactory's definition
	*/

	CQuadSpriteFactory::CQuadSpriteFactory() :
		CBaseComponentFactory()
	{
	}

	IComponent* CQuadSpriteFactory::CreateDefault() const
	{
		E_RESULT_CODE result = RC_OK;
		return CreateQuadSprite(result);
	}

	E_RESULT_CODE CQuadSpriteFactory::SetupComponent(CQuadSprite* pComponent, const TQuadSpriteParameters& params) const
	{
		if (!pComponent)
		{
			return RC_INVALID_ARGS;
		}

		return RC_OK;
	}


	IComponentFactory* CreateQuadSpriteFactory(E_RESULT_CODE& result)
	{
		return CREATE_IMPL(IComponentFactory, CQuadSpriteFactory, result);
	}
}