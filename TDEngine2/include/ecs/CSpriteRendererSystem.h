/*!
	\file CSpriteRendererSystem.h
	\date 04.12.2018
	\authors Kasimov Ildar
*/

#pragma once


#include "ISystem.h"
#include "./../core/CBaseObject.h"
#include <vector>


namespace TDEngine2
{
	class CTransform;
	class CQuadSprite;
	class CRenderQueue;
	class IGraphicsObjectManager;
	class IVertexBuffer;
	class IIndexBuffer;
	class IVertexDeclaration;
	class IGraphicsLayersInfo;


	/*!
		\brief A factory function for creation objects of CSpriteRendererSystem's type.

		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

		\param[in, out] pRenderQueue A pointer to CRenderQueue which represents a buffer of commands for a rendering pipeline

		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CSpriteRendererSystem's implementation
	*/

	TDE2_API ISystem* CreateSpriteRendererSystem(IGraphicsObjectManager* pGraphicsObjectManager, CRenderQueue* pRenderQueue, E_RESULT_CODE& result);


	/*!
		class CSpriteRendererSystem

		\brief The class is a system that processes ISprite components
	*/

	class CSpriteRendererSystem : public ISystem, public CBaseObject
	{
		public:
			friend TDE2_API ISystem* CreateSpriteRendererSystem(IGraphicsObjectManager* pGraphicsObjectManager, CRenderQueue* pRenderQueue, E_RESULT_CODE& result);
		public:
			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\param[in, out] pRenderQueue A pointer to CRenderQueue which represents a buffer of commands for a rendering pipeline

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IGraphicsObjectManager* pGraphicsObjectManager, CRenderQueue* pRenderQueue);

			/*!
				\brief The method frees all memory occupied by the object

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Free() override;

			/*!
				\brief The method inject components array into a system

				\param[in] pWorld A pointer to a main scene's object
			*/

			TDE2_API void InjectBindings(IWorld* pWorld) override;

			/*!
				\brief The main method that should be implemented in all derived classes.
				It contains all the logic that the system will execute during engine's work.

				\param[in] pWorld A pointer to a main scene's object

				\param[in] dt A delta time's value
			*/

			TDE2_API void Update(IWorld* pWorld, F32 dt) override;
		protected:
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CSpriteRendererSystem)

			TDE2_API U32 _computeSpriteCommandKey();
		protected:
			std::vector<CTransform*>    mTransforms;

			std::vector<CQuadSprite*>   mSprites;

			CRenderQueue*               mpRenderQueue;

			IGraphicsObjectManager*     mpGraphicsObjectManager;

			std::vector<IVertexBuffer*> mSpritesPerInstanceData;

			IVertexBuffer*              mpSpriteVertexBuffer;

			IIndexBuffer*               mpSpriteIndexBuffer;

			IVertexDeclaration*         mpSpriteVertexDeclaration;

			U16                         mSpriteFaces[6];

			IGraphicsLayersInfo*        mpGraphicsLayers;
	};
}