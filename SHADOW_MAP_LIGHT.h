//////////////////////////////////////////////////////////////////////////////////////////
//	SHADOW_MAP_LIGHT.h
//	Class declaration for light with shadow map. Derives from INTERACTOR, so it can be moved
//	using the mouse
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//	Modified:	10th September 2002	-	Converted to D3D
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef SHADOW_MAP_LIGHT_H
#define SHADOW_MAP_LIGHT_H

class SHADOW_MAP_LIGHT : public INTERACTOR
{
public:
	float n, f;
	D3DXMATRIX projectionMatrix;

	void SetClipDistances(float newNear, float newFar)
	{
		n=newNear;
		f=newFar;

		UpdateMatrices();
	}

	virtual void Update();
	void UpdateMatrices(void);

	int shadowMapSize;

	//Textures
	LPDIRECT3DTEXTURE8 shadowMapRenderTargetTexture;
	LPDIRECT3DTEXTURE8 shadowMapDepthTexture;
	
	//Pointers to top levels of textures
	IDirect3DSurface8 * shadowMapRenderTargetSurface;
	IDirect3DSurface8 * shadowMapDepthSurface;

	bool CreateTextures(LPDIRECT3D8 d3d, IDirect3DDevice8 * d3dDevice);
	void ReleaseTextures();

	SHADOW_MAP_LIGHT() :	shadowMapRenderTargetTexture(NULL), shadowMapDepthTexture(NULL),
							shadowMapRenderTargetSurface(NULL), shadowMapDepthSurface(NULL)
	{}
	~SHADOW_MAP_LIGHT()
	{}
};

#endif	//SHADOW_MAP_LIGHT_H