#pragma once

#include <directxmath.h>
#include <DXF.h>
using namespace DirectX;

class MyLight
{
public:
	struct LightInfo {
		XMFLOAT4 ambient = XMFLOAT4(0,0,0,0);
		XMFLOAT4 diffuse = XMFLOAT4(0, 0, 0, 0);
		XMFLOAT4 specular = XMFLOAT4(0, 0, 0, 0);
	};
	struct ImGuiLightInfo {
		float ambient[4] = { 0,0,0,0 };
		float diffuse[4] = { 1,0,0,1 };
		float specular[4] = { 0,0,0,0 };
	};
	LightInfo info;
	ImGuiLightInfo guiInfo;

	virtual string ToString();
	string ToString(ImGuiLightInfo guiInfo);

	virtual void UpdateLightWithGUIInfo() {
		UpdateLightWithGUIInfo(info, guiInfo);
	}
	void UpdateLightWithGUIInfo(LightInfo& info, ImGuiLightInfo& guiInfo) {
		info.ambient = XMFLOAT4(guiInfo.ambient[0], guiInfo.ambient[1], guiInfo.ambient[2],1);
		info.diffuse = XMFLOAT4(guiInfo.diffuse[0], guiInfo.diffuse[1], guiInfo.diffuse[2],1);
		info.specular = XMFLOAT4(guiInfo.specular[0], guiInfo.specular[1], guiInfo.specular[2], guiInfo.specular[3]);
	}

	void generatePerspectiveMatrix(float screenNear, float screenFar);			///< Generate project matrix based on current rotation and provided near & far plane
	void generateOrthoMatrix(float screenWidth, float screenHeight, float nearD, float farD);		///< Generates orthographic matrix based on supplied screen dimensions and near & far plane.
	virtual XMMATRIX getViewMatrix();			///< Get light view matrix for shadow mapping, returns XMMATRIX
	XMMATRIX getProjectionMatrix();		///< Get light projection matrix for shadow mapping, returns XMMATRIX
	XMMATRIX getPerspectiveMatrix();
	XMMATRIX getOrthoMatrix();			///< Get light orthographic matrix for shadow mapping, returns XMMATRIX

protected:
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMMATRIX perspectiveMatrix;
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

	XMMATRIX GetWorldMatrix();

	string ToString();

	struct DirLightInfo : public LightInfo
	{
		XMFLOAT4 position = XMFLOAT4(2, 3, 4, 0);
		XMFLOAT4 direction = XMFLOAT4(0, -1, 0, 0);
	};

	struct ImGuiDirLightInfo : public ImGuiLightInfo {
		float direction[4] = { 0,-1,0,0 };
		float pivot[4] = { 0,0,0, 10 }; //w component is dst from pivot along -direction
	};

	virtual void UpdateLightWithGUIInfo() {
		UpdateLightWithGUIInfo(info, guiInfo);
	}
	void UpdateLightWithGUIInfo(DirLightInfo& info, ImGuiDirLightInfo& guiInfo) {
		MyLight::UpdateLightWithGUIInfo(info, guiInfo);
		info.direction = XMFLOAT4(guiInfo.direction[0], guiInfo.direction[1], guiInfo.direction[2], 0);
		info.position = XMFLOAT4(guiInfo.pivot[0] - guiInfo.direction[0] * guiInfo.pivot[3], guiInfo.pivot[1] - guiInfo.direction[1] * guiInfo.pivot[3], guiInfo.pivot[2] - guiInfo.direction[2] * guiInfo.pivot[3], 0);
	}

	ImGuiDirLightInfo guiInfo;
	DirLightInfo info;
	ShadowMap* shadowMap;
};

static string Float4ToStr(const float a[4]);
