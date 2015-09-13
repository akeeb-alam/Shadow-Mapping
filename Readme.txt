**************************************************************************
**	Shadow Mapping
**
**	www.paulsprojects.net
**
**	paul@paulsprojects.net
**************************************************************************

Description:

This is a Direct3D conversion of my OpenGL shadow mapping project. It displays a simple scene, lit by a single light.

The scene is first rendered from the light's point of view, using one texture as render target, and another as depth-stencil surface. The depth-stencil texture is then projected onto the scene as drawn from the eye's point of view. A comparison of distance stored in the texture to distance from the light is automatically made, which determines shadowed regions.

The main differences between this and my OpenGL version are:

In this program, the shadow map can be arbitrarily large, since it is a separate render target. In the OpenGL version, the depth buffer of the window is used and read back, so the shadow map can be no larger than the window.

This program only supports 24 bit shadow mapping. An 8 bit version, which would work on far more systems, is harder in D3D since there is no easy way to retrieve the contents of the depth buffer and save this into a 2D luminance texture.

I had to use a pixel shader to evaluate the lighting equation on this project. This is because if you enable specular calculations with D3D using the fixed function pipeline, the specular color is automatically added to the pixel color after texture application. Using a pixel shader avoids this problem as it gives the application control of the color sum.


Requirements:

"Max" blending support
Pixel shader 1.1
Render-to-depth-texture support


References:

GDC 2000 "Shadow Mapping with Today's OpenGL Hardware", Mark Kilgard. From developer.nvidia.com

"Projective Texture Mapping" Cass Everitt. From developer.nvidia.com

"Hardware Shadow Mapping", Cass Everitt, Ashu Rege and Cem Cebenoyan. From developer.nvidia.com

"Shadowcast" demo. From developer.nvidia.com


Keys:

F1	-	Take a screenshot
Escape	-	Quit

Up Arrow-	Increase Shadow Map resolution
Down Arrow-	Decrease Shadow Map resolution

C	-	Use mouse to move camera
L	-	Use mouse to move light

T	-	Draw Tori (Donuts)
B	-	Draw Spheres
