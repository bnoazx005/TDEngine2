/*!
	\file CUIElementsRenderSystem.h
	\date 28.04.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"
#include "../utils/Utils.h"
#include "../graphics/UI/CUIElementMeshDataComponent.h"
#include <vector>
#include <unordered_map>


namespace TDEngine2
{
	class IRenderer;
	class IGraphicsObjectManager;
	class CRenderQueue;
	class IVertexDeclaration;
	class IResourceManager;
	class IGraphicsLayersInfo;
	class IVertexBuffer;
	class IIndexBuffer;

	enum class TResourceId : U32;
	enum class TMaterialInstanceId : U32;


	/*!
		\brief A factory function for creation objects of CUIElementsRenderSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CUIElementsRenderSystem's implementation
	*/

	TDE2_API ISystem* CreateUIElementsRenderSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);


	/*!
		class CUIElementsRenderSystem

		\brief The class is a system that processes UI elements such as canvases and LayoutElement components
	*/

	class CUIElementsRenderSystem : public CBaseSystem
	{
		public:
			friend TDE2_API ISystem* CreateUIElementsRenderSystem(IRenderer*, IGraphicsObjectManager*, E_RESULT_CODE& result);
		public:
			typedef std::unordered_map<TResourceId, TMaterialInstanceId> TMaterialsMap;
		public:
			TDE2_SYSTEM(CUIElementsRenderSystem);

			/*!
				\brief The method initializes an inner state of a system

				\param[in, out] pRenderer A pointer to IRenderer implementation

				\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation

				\return RC_OK if everything went ok, or some other code, which describes an error
			*/

			TDE2_API E_RESULT_CODE Init(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager);

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
			DECLARE_INTERFACE_IMPL_PROTECTED_MEMBERS(CUIElementsRenderSystem)

			TDE2_API E_RESULT_CODE _initDefaultResources();

			TDE2_API E_RESULT_CODE _updateGPUBuffers();
		protected:
			static constexpr U32 mMaxVerticesCount = 1 << 14;

			IGraphicsObjectManager*         mpGraphicsObjectManager;

			CRenderQueue*                   mpUIElementsRenderGroup;

			IResourceManager*               mpResourceManager;

			IVertexDeclaration*             mpDefaultUIVertexDecl;
			IVertexDeclaration*             mpDefaultFontVertexDecl;

			TResourceId                     mDefaultUIMaterialId;
			TResourceId                     mDefaultFontMaterialId;
			
			CScopedPtr<IGraphicsLayersInfo> mpGraphicsLayers;

			std::vector<TEntityId>          mUIElementsEntities;

			std::vector<TUIElementsVertex> mVertices;
			std::vector<U16>               mIndices;

			IVertexBuffer*                 mpVertexBuffer;
			IIndexBuffer*                  mpIndexBuffer;

			TMaterialsMap                  mUsingMaterials;
	};
}