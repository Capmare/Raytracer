
Movement and rotation is exactly like in Unreal engine

To move/rotate you have to hold the right mouse button while using the camera controls or movement

To enable/disable there are a couple of macros that you can modify

main.cpp USE_BUNNY
utils.h USE_SIMD_OP
rendered.h SOFT_SHADOWS PARALLEL_EXECUTION

This project uses only one external library for fast random number generation due to mt1997 being very slow.
In utils.h there are 2 methods to do the triangle intersection, both method are exactly the same and deliver same performance but due to the struggle i had while implementing the SIMD operations i left it there with a macro as a study case, the gain in fps is very minimal in both the bunny scene and the normal scene.
There is parallel execution implemented.
There are soft shadows implemented. 

Release	SOFT_SHADOWS PARALLEL_EXECUTION: 14fps
Release	SOFT_SHADOWS PARALLEL_EXECUTION USE_BUNNY: 0.58fps

USE_SIMD_OP will stay disable by default so it is sure it will compile on all computers
USE_BUNNY will be disabled by default so it renders the scene with the most fps in it for smooth movement and camera rotation


