cmake_minimum_required (VERSION 2.8)

#
#	\todo options' checking was removed to simplify building. SHOULD BE returned later
#

#if (BUILD_D3D11_GCTX_PLUGIN)
	add_subdirectory(D3D11GraphicsContext)
#endif (BUILD_D3D11_GCTX_PLUGIN)

#if (BUILD_GL3_0_GCTX_PLUGIN)
	add_subdirectory(OGLGraphicsContext)
#endif (BUILD_GL3_0_GCTX_PLUGIN)