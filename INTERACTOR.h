//////////////////////////////////////////////////////////////////////////////////////////
//	INTERACTOR.h
//	Class declaration for mouse interactor class
//	Downloaded from: www.paulsprojects.net
//	Created:	8th September 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef INTERACTOR_H
#define INTERACTOR_H

class INTERACTOR
{
public:
	void Init(	D3DXVECTOR3 startPosition,
				float newMinR=0.0f, float newMaxR=10000000.0f,
				D3DXVECTOR3 newLookAt=D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	
	void SetSensitivity(double newRotationSensitivity=0.5*D3DX_PI/180, double newTranslationSensitivity=0.01)
	{
		rotationSensitivity=newRotationSensitivity;
		translationSensitivity=newTranslationSensitivity;
	}
	
	virtual void Update();
	void SetupViewMatrix();

	INTERACTOR()	:	rotationSensitivity(0.5*D3DX_PI/180), translationSensitivity(0.01)
	{}
	virtual ~INTERACTOR()
	{}

	D3DXVECTOR3 position;
	D3DXVECTOR3 lookAt;

	D3DXMATRIX viewMatrix;

	//spherical coordinates
	float r;
	double leftRightRotation, upDownRotation;
	
	double rotationSensitivity, translationSensitivity;
	float minR, maxR;
};

#endif	//INTERACTOR_H