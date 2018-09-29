/*!
	\file TDEngine.h
	\date 14.09.2018
	\authors Kasimov Ildar
*/

#pragma once

///core
#include "core\IEngineCore.h"
#include "core\IEngineSubsystem.h"
#include "core\IWindowSystem.h"
#include "core\IGraphicsContext.h"
#include "core\CEngineCore.h"
#include "core\IEngineCoreBuilder.h"
#include "core\CDefaultEngineCoreBuilder.h"
#include "core\IEngineListener.h"
#include "core\IBaseObject.h"
#include "core\CBaseObject.h"

///ecs
#include "ecs\CEntity.h"
#include "ecs\CEntityManager.h"
#include "ecs\CComponentManager.h"
#include "ecs\IComponent.h"
#include "ecs\CBaseComponent.h"
#include "ecs\ISystem.h"
#include "ecs\ISystemManager.h"
#include "ecs\CSystemManager.h"
#include "ecs\IWorld.h"
#include "ecs\CWorld.h"

///graphics
#include "graphics\d3d11\CD3D11GraphicsContext.h"
#include "graphics\ogl\COGLGraphicsContext.h"
#include "graphics\ogl\IOGLContextFactory.h"
#include "graphics\ogl\win32\CWin32GLContextFactory.h"

///platform
#include "platform\win32\CWin32WindowSystem.h"
#include "platform\win32\CWin32Timer.h"

///utils
#include "utils\Types.h"
#include "utils\Config.h"
#include "utils\Color.h"
#include "utils\ILogger.h"
#include "utils\CFileLogger.h"
#include "utils\ITimer.h"