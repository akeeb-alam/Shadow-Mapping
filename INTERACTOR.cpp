//////////////////////////////////////////////////////////////////////////////////////////
//	INTERACTOR.cpp
//	Functions for mouse interactor class
//	Downloaded from: www.paulsprojects.net
//	Created:	8th September 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#include <windows.h>
#include <D3DX8.h>
#include "Log/LOG.h"
#include "Window/WINDOW.h"
#include "INTERACTOR.h"

void INTERACTOR::Init(	D3DXVECTOR3 startPosition, float newMinR, float newMaxR,
						D3DXVECTOR3 newLookAt)
{
	position=startPosition;
	lookAt=newLookAt;

	minR=newMinR;
	maxR=newMaxR;
	
	//calculate r and angles
	D3DXVECTOR3 lineOfSight=position-lookAt;
	r=(float)sqrt(double(	lineOfSight.x*lineOfSight.x+
							lineOfSight.y*lineOfSight.y+
							lineOfSight.z*lineOfSight.z));
	
	if(lineOfSight.z==0.0f)
	{
		if(lineOfSight.x>0.0f)
			leftRightRotation=-D3DX_PI/2;
		if(lineOfSight.x<0.0f)
			leftRightRotation=D3DX_PI/2;
	}
	else
	{
		if(lineOfSight.z<0.0f)
			leftRightRotation=D3DX_PI-atan(lineOfSight.x/lineOfSight.z);
		if(lineOfSight.z>0.0f)
			leftRightRotation=-atan(lineOfSight.x/lineOfSight.z);
	}


	
	if(lineOfSight.x==0.0f && lineOfSight.z==0.0f)
		upDownRotation=D3DX_PI/2;
	else
		upDownRotation=atan(lineOfSight.y/sqrt(	(lineOfSight.x*lineOfSight.x)+
												(lineOfSight.z*lineOfSight.z)));
}

void INTERACTOR::Update()
{
	//update angles
	if(WINDOW::Instance()->IsLeftButtonPressed())
	{
		leftRightRotation+=(WINDOW::Instance()->GetMouseXMovement())*rotationSensitivity;
		upDownRotation+=(WINDOW::Instance()->GetMouseYMovement())*rotationSensitivity;
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

void INTERACTOR::SetupViewMatrix()
{
	D3DXMATRIX rTranslation, udRotation, lrRotation, lookAtTranslation;
	D3DXMatrixTranslation(&rTranslation, 0.0f, 0.0f, -r);
	D3DXMatrixRotationX(&udRotation, (float)upDownRotation);
	D3DXMatrixRotationY(&lrRotation, (float)leftRightRotation);
	D3DXMatrixTranslation(&lookAtTranslation, -lookAt.x, -lookAt.y, -lookAt.z);
	
	viewMatrix=lookAtTranslation*lrRotation*udRotation*rTranslation;
}
