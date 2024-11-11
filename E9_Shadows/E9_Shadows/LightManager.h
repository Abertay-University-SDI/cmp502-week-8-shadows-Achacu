#pragma once
#include <string>
#include "DXF.h"	// include dxframework
#include <iostream>
#include <fstream>
#include "MyLight.h"

class LightManager
{
public:
	LightManager();

	DirectionalLight* GetDirectionalLight(string id);

	std::map<string, DirectionalLight>::iterator GetBegin();
	std::map<string, DirectionalLight>::iterator GetEnd();

	void FillLightInfo(DirectionalLight* light, float ambient[4], float diffuse[4], float specular[4]);
	void AddDirectionalLight(string id, float ambient[4], float diffuse[4], float specular[4], float pivot[4], float direction[4]);

	void ReadDirectionalLight(string id, string lightStr);

	void ReadLightDataFromFile(string filePath);
	void WriteLightDataToFile(string filePath);
private:
	std::map<string, DirectionalLight> dirLights;
};



