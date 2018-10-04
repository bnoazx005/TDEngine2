/*!
	/file TDEngine.h
	/date 14.09.2018
	/authors Kasimov Ildar
*/

#pragma once

///core
#include "core/IEngineCore.h"
#include "core/IEngineSubsystem.h"
#include "core/IWindowSystem.h"
#include "core/IGraphicsContext.h"
#include "core/CEngineCore.h"
#include "core/IEngineCoreBuilder.h"
#include "core/CDefaultEngineCoreBuilder.h"
#include "core/IEngineListener.h"
#include "core/IBaseObject.h"
#include "core/CBaseObject.h"
#include "core/IPlugin.h"
#include "core/IDLLManager.h"

///ecs
#include "ecs/CEntity.h"
#include "ecs/CEntityManager.h"
#include "ecs/CComponentManager.h"
#include "ecs/IComponent.h"
#include "ecs/CBaseComponent.h"
#include "ecs/ISystem.h"
#include "ecs/ISystemManager.h"
#include "ecs/CSystemManager.h"
#include "ecs/IWorld.h"
#include "ecs/CWorld.h"

///graphics
#include "graphics/IBuffer.h"
#include "graphics/IVertexBuffer.h"
#include "graphics/IIndexBuffer.h"

///math
#include "math/TVector3.h"

///platform
#include "platform/win32/CWin32WindowSystem.h"
#include "platform/win32/CWin32Timer.h"
#include "platform/win32/CWin32DLLManager.h"

///utils
#include "utils/Types.h"
#include "utils/Config.h"
#include "utils/Color.h"
#include "utils/ILogger.h"
#include "utils/CFileLogger.h"
#include "utils/ITimer.h"
#include "utils/Utils.h"