#include "MyLight.h"
#include <sstream>
#include <iomanip>

static string Float4ToStr(const float a[4]) {
	std::stringstream ss = stringstream();
	ss << std::defaultfloat << a[0] << ",";
	ss << std::defaultfloat << a[1] << ",";
	ss << std::defaultfloat << a[2] << ",";
	ss << std::defaultfloat << a[3];
	return "{" + ss.str() + "}";
}
string MyLight::ToString() {
	return ToString(guiInfo);
}
string MyLight::ToString(ImGuiLightInfo guiInfo) {
	string s = "";
	s += Float4ToStr(guiInfo.ambient) + ",";
	s += Float4ToStr(guiInfo.diffuse) + ",";
	s += Float4ToStr(guiInfo.specular);
	return s;
}

// Create a projection matrix for the (point) light source. Used in shadow mapping.
void MyLight::generatePerspectiveMatrix(float screenNear, float screenFar, float fovAngleY, float aspectRatio)
{
	// Create the projection matrix for the light.
	perspectiveMatrix = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, screenNear, screenFar);
	projectionMatrix = perspectiveMatrix;
}
// Create orthomatrix for (directional) light source. Used in shadow mapping.
void MyLight::generateOrthoMatrix(float screenWidth, float screenHeight, float nearD, float farD)
{
	orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, nearD, farD);
	projectionMatrix = orthoMatrix;
}
XMMATRIX MyLight::getViewMatrix()
{
	return viewMatrix;
}

XMMATRIX MyLight::getProjectionMatrix()
{
	return projectionMatrix;
}
XMMATRIX MyLight::getPerspectiveMatrix()
{
	return perspectiveMatrix;
}
XMMATRIX MyLight::getOrthoMatrix()
{
	return orthoMatrix;
}


//create view matrix, based on light position and lookat. Used for shadow mapping.
void DirectionalLight::generateViewMatrix()
{
	XMFLOAT4& direction = info.direction;
	// default up vector
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	if (direction.y == 1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
	}
	else if (direction.y == -1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
	}

	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
	XMVECTOR right = XMVector3Cross(dir, up);
	up = XMVector3Cross(right, dir);
	// Create the view matrix from the three vectors.
	XMVECTOR position = XMLoadFloat3(&XMFLOAT3(info.position.x, info.position.y, info.position.z));
	viewMatrix = XMMatrixLookAtLH(position, position + dir, up);
}

XMMATRIX DirectionalLight::GetWorldMatrix() { return XMMatrixTranslation(info.position.x, info.position.y, info.position.z); }
string DirectionalLight::ToString() {
	string s = MyLight::ToString(guiInfo);	
	s += "," + Float4ToStr(guiInfo.pivot) + ",";
	s += Float4ToStr(guiInfo.direction);
	return s;
}



XMMATRIX PointLight::GetWorldMatrix() { return XMMatrixTranslation(info.position.x, info.position.y, info.position.z); }
string PointLight::ToString()
{
	string s = MyLight::ToString(guiInfo);
	s += "," + Float4ToStr(guiInfo.position) + ",";
	s += Float4ToStr(guiInfo.attenuation);
	return s;
}
void PointLight::generateViewMatrix(int shadowFaceIndex)
{
	XMVECTOR dir;
	switch (shadowFaceIndex)
	{
		case 0:
			// right
			dir = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0);
			break;
		case 1:
			// left
			dir = XMVectorSet(-1.0f, 0.0f, 0.0f, 1.0);
			break;
		case 2:
			// up
			dir = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0);
			break;
		case 3:
			// down
			dir = XMVectorSet(0.0f, -1.0f, 0.0f, 1.0);
			break;
		case 4:
			// front
			dir = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
			break;
		case 5:
			// back 
			dir = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
			break;
	}
	XMVECTOR up;
	switch (shadowFaceIndex)
	{
		case 0:
		case 1:
		case 4:
		case 5:
			up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 2:
			up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
			break;
		case 3:
			up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
			break;
	}

	// Create the view matrix from the three vectors.
	XMVECTOR position = XMLoadFloat3(&XMFLOAT3(info.position.x, info.position.y, info.position.z));
	viewMatrix = XMMatrixLookToLH(position, dir, up);
}

void PointLight::generatePerspectiveMatrix()
{
	//Increasing near plane improves precision from afar. Far plane is adjusted to fit pointlight's range. FOV is set at 90º deg.
	MyLight::generatePerspectiveMatrix(.5f, guiInfo.attenuation[3], 1.570796f, 1);
}


void SpotLight::generateViewMatrix()
{
	XMFLOAT4& direction = info.direction;
	// default up vector
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	if (direction.y == 1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
	}
	else if (direction.y == -1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
	}

	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
	XMVECTOR right = XMVector3Cross(dir, up);
	up = XMVector3Cross(right, dir);
	// Create the view matrix from the three vectors.
	XMVECTOR position = XMLoadFloat3(&XMFLOAT3(info.position.x, info.position.y, info.position.z));
	viewMatrix = XMMatrixLookAtLH(position, position + dir, up);
}

void SpotLight::generatePerspectiveMatrix()
{
	//Increasing near plane improves precision from afar. Far plane is adjusted to fit spotlight's range. FOV is adjusted to match twice the cutoff angle.
	MyLight::generatePerspectiveMatrix(.5f, guiInfo.attenuation[3]*1.1f, guiInfo.angleFalloff[0]*2.1f, 1);
}

XMMATRIX SpotLight::GetWorldMatrix() { return XMMatrixTranslation(info.position.x, info.position.y, info.position.z); }
string SpotLight::ToString()
{
	string s = MyLight::ToString(guiInfo);
	s += "," + Float4ToStr(guiInfo.position) + ",";
	s += Float4ToStr(guiInfo.direction) + ",";
	s += Float4ToStr(guiInfo.attenuation) + ",";
	s += Float4ToStr(guiInfo.angleFalloff);
	return s;
}
