//////////////////////////////////////////////////////////////////////////////////////////
//	Main.cpp
//	Direct3D Shadow Mapping
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
#include "Timer/TIMER.h"
#include "Fps Counter/FPS_COUNTER.h"
#include "Window/WINDOW.h"
#include "Bitmap Font/BITMAP_FONT.h"
#include "INTERACTOR.h"
#include "SHADOW_MAP_LIGHT.h"
#include "scene.h"
#include "Main.h"

//link to libraries
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")

TIMER timer;
FPS_COUNTER fpsCounter;
BITMAP_FONT font;

//Main D3D interface
LPDIRECT3D8 d3d=NULL;

//D3D device
IDirect3DDevice8 * d3dDevice=NULL;

//Camera
INTERACTOR camera;

//Light
SHADOW_MAP_LIGHT light;
D3DLIGHT8 dimLight, brightLight;	//for shadowed, unshadowed respectively

//shadow map size
int shadowMapPower=10;		//size=2**power
const int minShadowMapPower=7, maxShadowMapPower=10;

//Current interactor
INTERACTOR * currentInteractor=&camera;

//Window surfaces
IDirect3DSurface8 * windowRenderTarget=NULL;
IDirect3DSurface8 * windowDepthSurface=NULL;

//Light sphere
D3DMATERIAL8 lightMaterial;
ID3DXMesh * lightSphere=NULL;

//Scale/bias matrix for shadow map
D3DXMATRIX biasMat;

//Pixel shader ID
DWORD psID;

//If true, draw small tori, otherwise small spheres
bool drawTori=false;

//Set up D3D
bool D3DInit()
{
	//Init window
	if(!WINDOW::Instance()->Init("Shadow Mapping", 640, 480, D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8,
								 D3DFMT_D24S8, false, &d3d, &d3dDevice))
	{
		return false;
	}
	
	//Check for caps
	D3DCAPS8 caps;
	d3dDevice->GetDeviceCaps(&caps);

	//Check for "max" blending support
	if(!(caps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP))
	{
		LOG::Instance()->OutputError("MAX blending unsupported");
		return false;
	}

	//Check for ps.1.1 support
	if(caps.PixelShaderVersion<D3DPS_VERSION(1, 1))
	{
		LOG::Instance()->OutputError("Pixel shader 1.1 unsupported");
		return false;
	}

	//Init font
	if(!font.Init(d3dDevice))
		return false;

	//Enable z buffer
	d3dDevice->SetRenderState(D3DRS_ZENABLE, true);

	//Cull CW since using a RH coordinate system
	d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	//Set up Lighting
	d3dDevice->SetRenderState(D3DRS_LIGHTING, true);
	d3dDevice->SetRenderState(D3DRS_AMBIENT, 0xFF404040);
	d3dDevice->SetRenderState(D3DRS_SPECULARENABLE, true);

	return true;
}

//Set up variables
bool DemoInit()
{
	//Init scene
	if(!InitScene(d3dDevice))
		return false;

	//Init camera
	camera.Init(D3DXVECTOR3(-2.5f, 3.5f, -2.5f), 3.2f);

	//Init light
	light.Init(D3DXVECTOR3(2.0f, 3.0f,-2.0f), 3.5f, 7.0f);
	light.SetClipDistances(2.0f, 8.0f);
	light.shadowMapSize=1<<shadowMapPower;

	//Set the light material
	memset(&lightMaterial, 0, sizeof(D3DMATERIAL8));
	lightMaterial.Emissive=D3DXCOLOR(1.0f, 1.0f, 0.0f, 0.0f);

	//Create light sphere
	HRESULT hr;
	hr=D3DXCreateSphere(d3dDevice, 0.1f, 12, 12, &lightSphere, NULL);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create light sphere");
		return false;
	}

	//Fill in the d3d light structures
	memset(&brightLight, 0, sizeof(D3DLIGHT8));
	brightLight.Type=D3DLIGHT_POINT;
	brightLight.Position=light.position;
	brightLight.Ambient=D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.0f);
	brightLight.Diffuse=D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
	brightLight.Specular=D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
	brightLight.Attenuation0=1.0f;
	brightLight.Range=1000.0f;

	memset(&dimLight, 0, sizeof(D3DLIGHT8));
	dimLight.Type=D3DLIGHT_POINT;
	dimLight.Position=light.position;
	dimLight.Ambient=D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.0f);
	dimLight.Diffuse=D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.0f);
	dimLight.Specular=D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
	dimLight.Attenuation0=1.0f;
	dimLight.Range=1000.0f;

	//Create light textures
	if(!light.CreateTextures(d3d, d3dDevice))
		return false;

	//Save pointers to the current render target and depth-stencil surfaces
	//so they can be restored
	hr=d3dDevice->GetRenderTarget(&windowRenderTarget);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to save window render target");
		return false;
	}
	hr=d3dDevice->GetDepthStencilSurface(&windowDepthSurface);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to save window depth-stencil surface");
		return false;
	}


	//Load the pixel shader
	LPD3DXBUFFER psCode;
	hr=D3DXAssembleShaderFromFile("shadowMapPS.txt", 0, NULL, &psCode, NULL);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to open \"shadowMapPS.txt\"");
		return false;
	}
	hr=d3dDevice->CreatePixelShader((DWORD *)psCode->GetBufferPointer(), &psID);
	if(FAILED(hr))
	{
		LOG::Instance()->OutputError("Unable to create pixel shader");
		return false;
	}

	//Set the scale/bias shadow map matrix
	//See nVidia "Hardware Shadow Mapping" paper
	float offset=0.5f+(0.5f/light.shadowMapSize);
	biasMat._11=0.5f;	biasMat._12=0.0f;	biasMat._13=0.0f;			biasMat._14=0.0f;
	biasMat._21=0.0f;	biasMat._22=-0.5f;	biasMat._23=0.0f;			biasMat._24=0.0f;
	biasMat._31=0.0f;	biasMat._32=0.0f;	biasMat._33=float(1<<24);	biasMat._34=0.0f;
	biasMat._41=offset;	biasMat._42=offset;	biasMat._43=-400000.0f;		biasMat._44=1.0f;

	//reset timer
	timer.Reset();

	return true;
}

//Perform per-frame updates
void UpdateFrame()
{
	//set currentTime and timePassed
	static double lastTime=timer.GetTime();
	double currentTime=timer.GetTime();
	double timePassed=currentTime-lastTime;
	lastTime=currentTime;

	//Update window
	WINDOW::Instance()->Update();

	//Update camera/light due to mouse movement
	currentInteractor->Update();

	//If we are moving the light, update the positions in the light structures
	if(currentInteractor==&light)
	{
		dimLight.Position=light.position;
		brightLight.Position=light.position;
	}

	//Update the shadow map size
	if(WINDOW::Instance()->IsKeyPressed(VK_UP) && shadowMapPower<maxShadowMapPower)
	{
		++shadowMapPower;
		light.shadowMapSize=1<<shadowMapPower;

		//Recreate the shadow map textures
		light.CreateTextures(d3d, d3dDevice);

		//Update the "offset" in the scale & bias matrix
		float offset=0.5f+(0.5f/light.shadowMapSize);
		biasMat._41=offset;
		biasMat._42=offset;

		WINDOW::Instance()->SetKeyReleased(VK_UP);
	}

	if(WINDOW::Instance()->IsKeyPressed(VK_DOWN) && shadowMapPower>minShadowMapPower)
	{
		--shadowMapPower;
		light.shadowMapSize=1<<shadowMapPower;

		//Recreate the shadow map textures
		light.CreateTextures(d3d, d3dDevice);

		//Update the "offset" in the scale & bias matrix
		float offset=0.5f+(0.5f/light.shadowMapSize);
		biasMat._41=offset;
		biasMat._42=offset;

		WINDOW::Instance()->SetKeyReleased(VK_DOWN);
	}

	//Change the current interactor
	if(WINDOW::Instance()->IsKeyPressed('C'))
		currentInteractor=&camera;

	if(WINDOW::Instance()->IsKeyPressed('L'))
		currentInteractor=&light;

	//Pause/unpause
	if(WINDOW::Instance()->IsKeyPressed('P'))
		timer.Pause();

	if(WINDOW::Instance()->IsKeyPressed('U'))
		timer.Unpause();

	//Select between drawing tori & spheres
	if(WINDOW::Instance()->IsKeyPressed('S'))
		drawTori=false;

	if(WINDOW::Instance()->IsKeyPressed('T'))
		drawTori=true;
	
	//Render frame
	RenderFrame(currentTime, timePassed);
}

//Render a frame
void RenderFrame(double currentTime, double timePassed)
{
	//Set shadow map as render target
	d3dDevice->SetRenderTarget(	light.shadowMapRenderTargetSurface,
								light.shadowMapDepthSurface);

	//Clear buffers
	d3dDevice->Clear(	0,
						NULL,
						D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,	//which buffers to clear
						D3DCOLOR_XRGB(0, 0, 0),				//color to clear to
						1.0f, 0);							//depth & stencil values

	//Draw from light's point of view
	light.UpdateMatrices();
	d3dDevice->SetTransform(D3DTS_PROJECTION, &light.projectionMatrix);
	d3dDevice->SetTransform(D3DTS_VIEW, &light.viewMatrix);

	//Turn off lighting
	d3dDevice->SetRenderState(D3DRS_LIGHTING, false);

	//Use flat shading
	d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	//Disable color writes
	d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

	//Draw scene
	if(SUCCEEDED(d3dDevice->BeginScene()))
	{
		DrawScene(d3dDevice, (float)currentTime/1000, drawTori);
	
		//End Drawing
		d3dDevice->EndScene();
	}



	//Set window as render target
	d3dDevice->SetRenderTarget(	windowRenderTarget, windowDepthSurface);

	//Clear buffers
	d3dDevice->Clear(	0,
						NULL,
						D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,	//which buffers to clear
						D3DCOLOR_XRGB(0, 0, 0),				//color to clear to
						1.0f, 0);							//depth & stencil values

	//Draw from camera's point of view
	D3DXMATRIX projectionMatrix;
	D3DXMatrixPerspectiveFovRH(	&projectionMatrix,
								D3DX_PI/4,
								(float)WINDOW::Instance()->width/WINDOW::Instance()->height,
								1.0f, 100.0f);
	d3dDevice->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

	camera.SetupViewMatrix();
	d3dDevice->SetTransform(D3DTS_VIEW, &camera.viewMatrix);

	//turn on lighting, Gouraud shading and color writes
	d3dDevice->SetRenderState(D3DRS_LIGHTING, true);
	d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	d3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE,	D3DCOLORWRITEENABLE_RED		|
														D3DCOLORWRITEENABLE_GREEN	|
														D3DCOLORWRITEENABLE_BLUE	|
														D3DCOLORWRITEENABLE_ALPHA);

	//Draw everything shadowed
	if(SUCCEEDED(d3dDevice->BeginScene()))
	{
		d3dDevice->SetLight(0, &dimLight);
		d3dDevice->LightEnable(0, true);

		DrawScene(d3dDevice, (float)currentTime/1000, drawTori);
	
		//End Drawing
		d3dDevice->EndScene();
	}




	//Now draw unshadowed parts
	
	//use MAX blending
	d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX);
	d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

	
	//Bind shadow map depth texture to unit 0
	d3dDevice->SetTexture(0, light.shadowMapDepthTexture);

	//use linear filtering
	d3dDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	d3dDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);

	//Set up texture coordinate generation
	d3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,
									D3DTTFF_COUNT4 | D3DTTFF_PROJECTED);

	//Set up texture matrix
	D3DXMATRIX inverseView;
	D3DXMatrixInverse(&inverseView, NULL, &camera.viewMatrix);
	D3DXMATRIX textureMatrix=	inverseView * light.viewMatrix *
								light.projectionMatrix * biasMat;
	d3dDevice->SetTransform(D3DTS_TEXTURE0, &textureMatrix);

	//Draw unshadowed
	if(SUCCEEDED(d3dDevice->BeginScene()))
	{
		d3dDevice->SetLight(0, &brightLight);
		d3dDevice->LightEnable(0, true);

		//Use pixel shader
		d3dDevice->SetPixelShader(psID);

		DrawScene(d3dDevice, (float)currentTime/1000, drawTori);

		d3dDevice->SetPixelShader(NULL);
	
		//End Drawing
		d3dDevice->EndScene();
	}

	//Disable blending
	d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	//Disable the texture
	d3dDevice->SetTexture(0, NULL);

	//Disable texture coord generation
	d3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);

	
	
	//Set light's world matrix
	D3DXMATRIX lightWorldMatrix;
	D3DXMatrixTranslation(	&lightWorldMatrix,
							light.position.x, light.position.y, light.position.z);
	d3dDevice->SetTransform(D3DTS_WORLD, &lightWorldMatrix);

	//Set light material
	d3dDevice->SetMaterial(&lightMaterial);

	//Draw light & print details
	if(SUCCEEDED(d3dDevice->BeginScene()))
	{
		lightSphere->DrawSubset(0);

		font.StartTextMode();

		fpsCounter.Update();
		font.Print(0, 8, 0xFFFFFF00, "FPS: %.2f", fpsCounter.GetFps());

		font.Print(	0, WINDOW::Instance()->height-70, 0xFFFFFF00,
					"Shadow Map Size: %d", light.shadowMapSize);

		font.Print(	0, WINDOW::Instance()->height-50, 0xFF20CF20,
					"Shadow Map Precision: 24 bit");

		font.EndTextMode();

		//End Drawing
		d3dDevice->EndScene();
	}


	//Swap buffers
	d3dDevice->Present(NULL, NULL, NULL, NULL);

	//Save a screenshot
	if(WINDOW::Instance()->IsKeyPressed(VK_F1))
	{
		WINDOW::Instance()->SaveScreenshot(d3dDevice);
		WINDOW::Instance()->SetKeyReleased(VK_F1);
	}

	//quit if necessary
	if(WINDOW::Instance()->IsKeyPressed(VK_ESCAPE))
		PostQuitMessage(0);
}

//Shut down demo
void DemoShutdown()
{
	light.ReleaseTextures();

	if(lightSphere)
		lightSphere->Release();
	lightSphere=NULL;

	//Shut down D3D objects
	if(d3dDevice)
		d3dDevice->Release();
	d3dDevice=NULL;

	if(d3d)
		d3d->Release();
	d3d=NULL;

	font.Shutdown();
	WINDOW::Instance()->Shutdown();
}

//WinMain
int WINAPI WinMain(	HINSTANCE	hInstance,			//Instance
					HINSTANCE	hPrevInstance,		//Previous Instance
					LPSTR		lpCmdLine,			//Command line params
					int			nShowCmd)			//Window show state
{
	//Save hInstance
	WINDOW::Instance()->hInstance=hInstance;

	//Init D3D and variables
	if(!D3DInit())
	{
		LOG::Instance()->OutputError("Direct3D Initiation Failed");
		return false;
	}
	else
		LOG::Instance()->OutputSuccess("Direct3D Initiation Successful");

	if(!DemoInit())
	{
		LOG::Instance()->OutputError("Demo Initiation Failed");
		return false;
	}
	else
		LOG::Instance()->OutputSuccess("Demo Initiation Successful");

	//Main Loop
	for(;;)
	{
		if(!(WINDOW::Instance()->HandleMessages()))	//quit if HandleMessages returns false
			break;

		UpdateFrame();
	}

	//Shutdown
	DemoShutdown();

	//Exit program
	LOG::Instance()->OutputSuccess("Exiting...");
	return 0;
}
