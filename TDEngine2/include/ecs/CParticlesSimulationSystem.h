/*!
	\file CParticlesSimulationSystem.h
	\date 21.03.2021
	\authors Kasimov Ildar
*/

#pragma once


#include "CBaseSystem.h"


namespace TDEngine2
{
	class IResourceManager;
	class IRenderer;
	class IGraphicsObjectManager;	


	/*!
		\brief A factory function for creation objects of CParticlesSimulationSystem's type.

		\param[in, out] pRenderer A pointer to IRenderer implementation
		\param[in, out] pGraphicsObjectManager A pointer to IGraphicsObjectManager implementation
		\param[out] result Contains RC_OK if everything went ok, or some other code, which describes an error

		\return A pointer to CParticlesSimulationSystem's implementation
	*/

	TDE2_API ISystem* CreateParticlesSimulationSystem(IRenderer* pRenderer, IGraphicsObjectManager* pGraphicsObjectManager, E_RESULT_CODE& result);
}