#pragma once

#include <directxmath.h>
using namespace DirectX;

class MyLight
{
public:
	struct LightInfo {
		XMFLOAT4 ambient = XMFLOAT4(0,0,0,0);
		XMFLOAT4 diffuse = XMFLOAT4(0, 0, 0, 0);
		//XMFLOAT3 direction;
		XMFLOAT4 specular = XMFLOAT4(0, 0, 0, 0);
		XMFLOAT4 position = XMFLOAT4(0, 0, 0, 0);
	};
	struct ImGuiLightInfo {
		float ambient[4] = { 0,0,0,0 };
		float diffuse[4] = { 1,0,0,0 };
		float specular[4] = { 2,0,0,0 };
		float position[4] = { 3,0,0,0 };
	};

	//virtual void generateViewMatrix();		///< Generates and upto date view matrix, based on current rotation
	void generateProjectionMatrix(float screenNear, float screenFar);			///< Generate project matrix based on current rotation and provided near & far plane
	void generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far);		///< Generates orthographic matrix based on supplied screen dimensions and near & far plane.

	struct MatrixInfo {
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
		//XMMATRIX orthoMatrix;
	};


	LightInfo info;
	MatrixInfo matrixInfo;
	ImGuiLightInfo guiInfo;
	virtual void UpdateLightWithGUIInfo() {
		UpdateLightWithGUIInfo(info, guiInfo);
	}
	void UpdateLightWithGUIInfo(LightInfo& info, ImGuiLightInfo& guiInfo) {
		info.ambient = XMFLOAT4(guiInfo.ambient[0], guiInfo.ambient[1], guiInfo.ambient[2],1);
		info.diffuse = XMFLOAT4(guiInfo.diffuse[0], guiInfo.diffuse[1], guiInfo.diffuse[2],1);
		info.specular = XMFLOAT4(guiInfo.specular[0], guiInfo.specular[1], guiInfo.specular[2], guiInfo.specular[3]);
		info.position = XMFLOAT4(guiInfo.position[0], guiInfo.position[1], guiInfo.position[2],0);
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
	virtual XMMATRIX getViewMatrix();			///< Get light view matrix for shadow mapping, returns XMMATRIX
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

class DirectionalLight : public MyLight
{
public:
	DirectionalLight() {
		info = DirLightInfo();
		guiInfo = ImGuiDirLightInfo();
	}
	void generateViewMatrix();		///< Generates and upto date view matrix, based on current rotation

	struct DirLightInfo : public LightInfo
	{
		XMFLOAT4 direction = XMFLOAT4(0, 0, 0, 0);
	};

	struct ImGuiDirLightInfo : public ImGuiLightInfo {
		float direction[4] = { 0,-1,0,0 };
	};

	virtual void UpdateLightWithGUIInfo() {
		UpdateLightWithGUIInfo(info, guiInfo);
	}
	void UpdateLightWithGUIInfo(DirLightInfo& info, ImGuiDirLightInfo& guiInfo) {
		MyLight::UpdateLightWithGUIInfo(info, guiInfo);
		info.direction = XMFLOAT4(guiInfo.direction[0], guiInfo.direction[1], guiInfo.direction[2], 0);
	}

	ImGuiDirLightInfo guiInfo;
	DirLightInfo info;
};
