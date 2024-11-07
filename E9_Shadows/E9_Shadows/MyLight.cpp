#include "MyLight.h"

// create view matrix, based on light position and lookat. Used for shadow mapping.
//void MyLight::generateViewMatrix()
//{
//	// default up vector
//	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
//	if (direction.y == 1 || (direction.x == 0 && direction.z == 0))
//	{
//		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
//	}
//	else if (direction.y == -1 || (direction.x == 0 && direction.z == 0))
//	{
//		up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
//	}
//	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
//	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
//	XMVECTOR right = XMVector3Cross(dir, up);
//	up = XMVector3Cross(right, dir);
//	// Create the view matrix from the three vectors.
//	viewMatrix = XMMatrixLookAtLH(position, position + dir, up);
//}

// Create a projection matrix for the (point) light source. Used in shadow mapping.
void MyLight::generateProjectionMatrix(float screenNear, float screenFar)
{
	float fieldOfView, screenAspect;

	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix for the light.
	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenFar);
	matrixInfo.projectionMatrix = projectionMatrix;
}

// Create orthomatrix for (directional) light source. Used in shadow mapping.
void MyLight::generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far)
{
	orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, near, far);
	matrixInfo.projectionMatrix = orthoMatrix;
}

//void MyLight::setAmbientColour(float red, float green, float blue, float alpha)
//{
//	ambientColour = XMFLOAT4(red, green, blue, alpha);
//}
//
//void MyLight::setDiffuseColour(float red, float green, float blue, float alpha)
//{
//	diffuseColour = XMFLOAT4(red, green, blue, alpha);
//}
//
//void MyLight::setDirection(float x, float y, float z)
//{
//	direction = XMFLOAT3(x, y, z);
//}
//
//void MyLight::setSpecularColour(float red, float green, float blue, float alpha)
//{
//	specularColour = XMFLOAT4(red, green, blue, alpha);
//}
//
//void MyLight::setSpecularPower(float power)
//{
//	specularPower = power;
//}
//
//void MyLight::setPosition(float x, float y, float z)
//{
//	position = XMVectorSet(x, y, z, 1.0f);
//}
//
//XMFLOAT4 MyLight::getAmbientColour()
//{
//	return ambientColour;
//}
//
//XMFLOAT4 MyLight::getDiffuseColour()
//{
//	return diffuseColour;
//}
//
//
//XMFLOAT3 MyLight::getDirection()
//{
//	return direction;
//}
//
//XMFLOAT4 MyLight::getSpecularColour()
//{
//	return specularColour;
//}
//
//
//float MyLight::getSpecularPower()
//{
//	return specularPower;
//}
//
//XMFLOAT3 MyLight::getPosition()
//{
//	XMFLOAT3 temp(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
//	return temp;
//}

XMMATRIX MyLight::getViewMatrix()
{
	return matrixInfo.viewMatrix;
}

XMMATRIX MyLight::getProjectionMatrix()
{
	return projectionMatrix;
}

XMMATRIX MyLight::getOrthoMatrix()
{
	return orthoMatrix;
}