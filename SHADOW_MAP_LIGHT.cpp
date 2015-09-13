//////////////////////////////////////////////////////////////////////////////////////////
//	SHADOW_MAP_LIGHT.cpp
//	Functions for light with shadow map
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//	Modified:	10th September 2002	-	Converted to D3D
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#include <windows.h>
#include <D3DX8.h>
#include "Window/WINDOW.h"
#include "Log/LOG.h"
#include "INTERACTOR.h"
#include "SHADOW_MAP_LIGHT.h"

//This is overrided to reverse the direction of movement when the left button is held
void SHADOW_MAP_LIGHT::Update()
{
	//update angles
	if(WINDOW::Instance()->IsLeftButtonPressed())
	{
		leftRightRotation-=(WINDOW::Instance()->GetMouseXMovement())*rotationSensitivity;
		upDownRotation-=(WINDOW::Instance()->GetMouseYMovement())*rotationSensitivity;
	}

	//update radius
	if(WINDOW::Instance()->IsRightButtonPressed())
	{
		//move out
		if(WINDOW::Instance()->GetMouseYMovement()>0)
		{
			//move out no further than maxR
			if(r+(WINDOW::Instance()->GetMouseYMovement())*(float)translationSensitivity>maxR)
				r=maxR;
			else
				r+=(WINDOW::Instance()->GetMouseYMovement())*(float)translationSensitivity;
		}

		//move in
		if(WINDOW::Instance()->GetMouseYMovement()<0)
		{
			//move in no further than minR
			if(r+(WINDOW::Instance()->GetMouseYMovement())*(float)translationSensitivity<minR)
				r=minR;
			else
				r+=(WINDOW::Instance()->GetMouseYMovement())*(float)translationSensitivity;
		}
	}

	//update position if there has been a change
	if(WINDOW::Instance()->IsLeftButtonPressed() || WINDOW::Instance()->IsRightButtonPressed())
	{
		D3DXMATRIX rTranslation, udRotation, lrRotation, lookAtTranslation, tempMatrix;
		D3DXMatrixTranslation(&lookAtTranslation, lookAt.x, lookAt.y, lookAt.z);
		D3DXMatrixRotationY(&lrRotation, -(float)leftRightRotation);
		D3DXMatrixRotationX(&udRotation, -(float)upDownRotation);
		D3DXMatrixTranslation(&rTranslation, 0.0f, 0.0f, r);
		
		tempMatrix=rTranslation*udRotation*lrRotation*lookAtTranslation;
		float rhw=1.0f/tempMatrix._44;
		position.x=tempMatrix._41*rhw;
		position.y=tempMatrix._42*rhw;
		position.z=tempMatrix._43*rhw;
	}
}

void SHADOW_MAP_LIGHT::UpdateMatrices(void)
{
	//Set up view matrix
	SetupViewMatrix();
	
	//Set up projection matrix
	D3DXMatrixPerspectiveFovRH(&projectionMatrix, D3DX_PI/3, 1.0f, n, f);
}

//Set up shadow map textures
bool SHADOW_MAP_LIGHT::CreateTextures(LPDIRECT3D8 d3d, IDirect3DDevice8 * d3dDevice)
{
	//release any old textures
	ReleaseTextures();

	HRESULT hr;

	//Check for render target texture support
	hr=d3d->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
								D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Render target texture unsupported");
		return false;
	}
	
	//Create render target texture
	hr=d3dDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, D3DUSAGE_RENDERTARGET,
								D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &shadowMapRenderTargetTexture);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create render target texture");
		return false;
	}

	//Get the top level of the shadow map render target texture
	shadowMapRenderTargetTexture->GetSurfaceLevel(0, &shadowMapRenderTargetSurface);




	//Check for depth texture support
	hr=d3d->CheckDeviceFormat(	D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
								D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_TEXTURE, D3DFMT_D24S8);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Depth texture unsupported");
		return false;
	}
	//Create depth texture
	hr=d3dDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, D3DUSAGE_DEPTHSTENCIL,
								D3DFMT_D24S8, D3DPOOL_DEFAULT, &shadowMapDepthTexture);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create depth texture");
		return false;
	}

		
	//Get the top level of the shadow map depth texture
	shadowMapDepthTexture->GetSurfaceLevel(0, &shadowMapDepthSurface);

	return true;
}

void SHADOW_MAP_LIGHT::ReleaseTextures()
{
	if(shadowMapRenderTargetSurface)
		shadowMapRenderTargetSurface->Release();
	shadowMapRenderTargetSurface=NULL;

	if(shadowMapDepthSurface)
		shadowMapDepthSurface->Release();
	shadowMapDepthSurface=NULL;

	if(shadowMapRenderTargetTexture)
		shadowMapRenderTargetTexture->Release();
	shadowMapRenderTargetTexture=NULL;
	
	if(shadowMapDepthTexture)
		shadowMapDepthTexture->Release();
	shadowMapDepthTexture=NULL;
}