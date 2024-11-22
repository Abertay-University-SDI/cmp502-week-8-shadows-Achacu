#pragma once
#include <string>
#include "DXF.h"	// include dxframework
#include <iostream>
#include <fstream>
#include "MyLight.h"

#define DIR_LIGHT_COUNT 3
#define POINT_LIGHT_COUNT 0

class LightManager
{
public:
	ID3D11ShaderResourceView* dirShadowMapsSRV;
	
	LightManager();

	DirectionalLight* GetDirectionalLight(string id);

	std::map<string, DirectionalLight>::iterator GetDirLightsBegin();
	std::map<string, DirectionalLight>::iterator GetDirLightsEnd();



	void ReadLightDataFromFile(string filePath);
	void InitializeLights(ID3D11Device* renderer);
	void WriteLightDataToFile(string filePath);
private:
	void ReadDirectionalLight(string id, string lightStr);
	void FillLightInfo(DirectionalLight* light, float ambient[4], float diffuse[4], float specular[4]);
	void AddDirectionalLight(string id, float ambient[4], float diffuse[4], float specular[4], float pivot[4], float direction[4]);
	void InitializeDirLights(ID3D11Device* renderer);

	int shadowmapWidth = 4096;
	int shadowmapHeight = 4096;
	int sceneWidth = 100;
	int sceneHeight = 100;

	std::map<string, DirectionalLight> dirLights;
	ID3D11Texture2D* dirShadowMaps;
};



