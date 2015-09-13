//////////////////////////////////////////////////////////////////////////////////////////
//	Scene.cpp
//	Scene for Direct3D Shadow Mapping
//	Downloaded from: www.paulsprojects.net
//	Created:	28th November 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <D3DX8.h>
#include "Log/LOG.h"
#include "scene.h"

//Torus
D3DMATERIAL8 torusMaterial;
ID3DXMesh * torus=NULL;

//Sphere
D3DMATERIAL8 sphereMaterial;
ID3DXMesh * sphere=NULL;

//Small torus
//Uses sphere material
ID3DXMesh * smallTorus=NULL;

//Floor
D3DMATERIAL8 floorMaterial;
IDirect3DVertexBuffer8 * floorVB=NULL;

bool InitScene(IDirect3DDevice8 * d3dDevice)
{
	HRESULT hr;

	//Create torus
	hr=D3DXCreateTorus(d3dDevice, 0.2f, 0.5f, 24, 36, &torus, NULL);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create torus");
		return false;
	}

	//Set up torus material
	memset(&torusMaterial, 0, sizeof(D3DMATERIAL8));
	torusMaterial.Diffuse.r=1.0f;
	torusMaterial.Ambient.r=1.0f;
	torusMaterial.Specular=D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
	torusMaterial.Power=16.0f;

	//Create sphere
	hr=D3DXCreateSphere(d3dDevice, 0.2f, 24, 24, &sphere, NULL);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create sphere");
		return false;
	}

	//Create small torus
	hr=D3DXCreateTorus(d3dDevice, 0.08f, 0.2f, 24, 24, &smallTorus, NULL);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create torus");
		return false;
	}

	//Set up sphere material
	memset(&sphereMaterial, 0, sizeof(D3DMATERIAL8));
	sphereMaterial.Diffuse.g=1.0f;
	sphereMaterial.Ambient.g=1.0f;
	sphereMaterial.Specular=D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
	sphereMaterial.Power=16.0f;

	//Create a triangle strip to build the floor out of
	FLOOR_VERTEX floorVertices[32];

	for(int i=0; i<16; ++i)
	{
		floorVertices[2*i].position=D3DXVECTOR3(float(i)/5-1.5f, 0.0f, 0.0f);
		floorVertices[2*i].normal=D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		floorVertices[2*i+1].position=D3DXVECTOR3(float(i)/5-1.5f, 0.0f, 0.2f);
		floorVertices[2*i+1].normal=D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	}

	//Put these vertices into a vertex buffer
	hr=d3dDevice->CreateVertexBuffer(	32*sizeof(FLOOR_VERTEX), D3DUSAGE_WRITEONLY,
										FLOOR_VERTEX_FVF, D3DPOOL_MANAGED, &floorVB);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create floor vertex buffer");
		return false;
	}

	unsigned char * bytePtr;
	hr=floorVB->Lock(0, 0, &bytePtr, 0);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to lock floor vertex buffer");
		return false;
	}

	memcpy(bytePtr, floorVertices, 32*sizeof(FLOOR_VERTEX));

	floorVB->Unlock();

	//Set up floor material
	memset(&floorMaterial, 0, sizeof(D3DMATERIAL8));
	floorMaterial.Diffuse.b=1.0f;
	floorMaterial.Ambient.b=1.0f;
	floorMaterial.Specular=D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
	floorMaterial.Power=32.0f;
	
	return true;
}

void DrawScene(IDirect3DDevice8 * d3dDevice, float angle, bool drawTori)
{
	D3DXMATRIX translationMatrix, rotationMatrix;
	D3DXMATRIX worldMatrix;

	//Set torus world matrix
	D3DXMatrixTranslation(&translationMatrix, 0.0f, 0.5f, 0.0f);
	D3DXMatrixRotationX(&rotationMatrix, D3DX_PI/2);
	worldMatrix=rotationMatrix*translationMatrix;
	d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);

	//Draw torus
	d3dDevice->SetMaterial(&torusMaterial);
	torus->DrawSubset(0);


	//Set sphere material
	d3dDevice->SetMaterial(&sphereMaterial);

	//Set small object rotation
	D3DXMatrixRotationY(&rotationMatrix, angle);
	D3DXMATRIX rotationMatrix2;

	//Loop through 4 spheres/tori, set translation and draw
	for(int i=0; i<4; ++i)
	{
		float x= (i==0 || i==3) ? 0.45f : -0.45f;
		float z= (i==2 || i==3) ? 0.45f : -0.45f;

		D3DXMatrixTranslation(&translationMatrix, x, 1.0f, z);
		D3DXMatrixRotationY(&rotationMatrix2, i*D3DX_PI/2-D3DX_PI/4);
		
		worldMatrix=rotationMatrix2*translationMatrix*rotationMatrix;
		d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);
		
		if(drawTori)
			smallTorus->DrawSubset(0);
		else
			sphere->DrawSubset(0);
	}

	//Set up to draw floor
	d3dDevice->SetMaterial(&floorMaterial);
	d3dDevice->SetVertexShader(FLOOR_VERTEX_FVF);
	d3dDevice->SetStreamSource(0, floorVB, sizeof(FLOOR_VERTEX));

	//draw top
	for(int i=0; i<15; ++i)
	{
		D3DXMatrixTranslation(&worldMatrix, 0.0f, 0.1f, 0.2f*i-1.5f);
		d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 30);
	}

	//draw bottom
	D3DXMatrixRotationZ(&rotationMatrix, D3DX_PI);

	for(int i=0; i<15; ++i)
	{
		D3DXMatrixTranslation(&translationMatrix, 0.0f,-0.1f, 0.2f*i-1.5f);
		worldMatrix=rotationMatrix*translationMatrix;
		d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 30);
	}

	//draw sides
	D3DXMatrixRotationX(&rotationMatrix, D3DX_PI/2);
	D3DXMatrixTranslation(&translationMatrix, 0.0f, 0.1f, 1.5f);

	for(int i=0; i<4; ++i)
	{
		D3DXMatrixRotationY(&rotationMatrix2, i*D3DX_PI/2);
		worldMatrix=rotationMatrix*translationMatrix*rotationMatrix2;
		d3dDevice->SetTransform(D3DTS_WORLD, &worldMatrix);
		d3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 30);
	}	
}

void ShutdownScene()
{
	if(torus)
		torus->Release();
	torus=NULL;

	if(sphere)
		sphere->Release();
	sphere=NULL;

	if(smallTorus)
		smallTorus->Release();
	smallTorus=NULL;

	if(floorVB)
		floorVB->Release();
	floorVB=NULL;
}