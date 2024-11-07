#pragma once

#include <directxmath.h>
using namespace DirectX;

class MyLight
{
public:
	MyLight() {
		info = LightInfo();
	}
	~MyLight() {	
	}

	void generateViewMatrix() {}		///< Generates and upto date view matrix, based on current rotation
	void generateProjectionMatrix(float screenNear, float screenFar);			///< Generate project matrix based on current rotation and provided near & far plane
	void generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far);		///< Generates orthographic matrix based on supplied screen dimensions and near & far plane.

	struct LightInfo {
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		//XMFLOAT3 direction;
		XMFLOAT4 specular;
		XMFLOAT4 position;
	};
	struct MatrixInfo {
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		//XMMATRIX orthoMatrix;
	};

	struct ImGuiLightInfo {
		float ambient[4];
		float diffuse[4];
		float direction[4];
		float specular[4];
		float position[4];
	};

	LightInfo info;
	MatrixInfo matrixInfo;
	ImGuiLightInfo guiInfo;
	void UpdateLightWithGUIInfo() {
		info.ambient = XMFLOAT4(guiInfo.ambient[0], guiInfo.ambient[1], guiInfo.ambient[2],0);
	}
	// Setters
	//void setAmbientColour(float red, float green, float blue, float alpha);		///< Set ambient colour RGBA
	//void setDiffuseColour(float red, float green, float blue, float alpha);		///< Set diffuse colour RGBA
	//void setDirection(float x, float y, float z);								///< Set light direction (for directional lights)
	//void setSpecularColour(float red, float green, float blue, float alpha);	///< set specular colour RGBA
	//void setSpecularPower(float power);											///< Set specular power
	//void setPosition(float x, float y, float z);								///< Set light position (for point lights)
	//void setLookAt(float x, float y, float z);									///< Set light lookAt (near deprecation)

	// Getters
	//XMFLOAT4 getAmbientColour();		///< Get ambient colour, returns float4
	//XMFLOAT4 getDiffuseColour();		///< Get diffuse colour, returns float4
	//XMFLOAT3 getDirection();			///< Get light direction, returns float3
	//XMFLOAT4 getSpecularColour();		///< Get specular colour, returns float4
	//float getSpecularPower();			///< Get specular power, returns float
	//XMFLOAT3 getPosition();				///< Get light position, returns float3
	XMMATRIX getViewMatrix();			///< Get light view matrix for shadow mapping, returns XMMATRIX
	XMMATRIX getProjectionMatrix();		///< Get light projection matrix for shadow mapping, returns XMMATRIX
	XMMATRIX getOrthoMatrix();			///< Get light orthographic matrix for shadow mapping, returns XMMATRIX


protected:
	//XMFLOAT4 ambientColour;
	//XMFLOAT4 diffuseColour;
	//XMFLOAT3 direction;
	//XMFLOAT4 specularColour;
	//float specularPower;
	//XMVECTOR position;
	//XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX orthoMatrix;
};

