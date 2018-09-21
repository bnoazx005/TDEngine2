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

///graphics
#include "graphics\d3d11\CD3D11GraphicsContext.h"

///platform
#include "platform\win32\CWin32WindowSystem.h"

///utils
#include "utils\Types.h"
#include "utils\Config.h"