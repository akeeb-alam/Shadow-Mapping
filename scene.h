//////////////////////////////////////////////////////////////////////////////////////////
//	Scene.h
//	Scene for Direct3D Shadow Mapping
//	Downloaded from: www.paulsprojects.net
//	Created:	28th November 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef SCENE_H
#define SCENE_H

bool InitScene(IDirect3DDevice8 * d3dDevice);
void DrawScene(IDirect3DDevice8 * d3dDevice, float angle, bool drawTori);
void ShutdownScene();

//Class for floor vertices
class FLOOR_VERTEX
{
public:
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
};

const DWORD FLOOR_VERTEX_FVF=D3DFVF_XYZ | D3DFVF_NORMAL;

#endif	//SCENE_H