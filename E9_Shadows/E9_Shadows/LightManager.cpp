#include "LightManager.h"
#include <sstream>

LightManager::LightManager()
{
	dirLights = map<string, DirectionalLight>();
}

DirectionalLight* LightManager::GetDirectionalLight(string id)
{
	return &dirLights.at(id);
}
std::map<string, DirectionalLight>::iterator LightManager::GetDirLightsBegin() {
	return dirLights.begin();
}
std::map<string, DirectionalLight>::iterator LightManager::GetDirLightsEnd() {
	return dirLights.end();
}
void LightManager::FillLightInfo(DirectionalLight* light, float ambient[4], float diffuse[4], float specular[4]) {
	memcpy(light->guiInfo.ambient, ambient, sizeof(float[4]));
	memcpy(light->guiInfo.diffuse, diffuse, sizeof(float[4]));
	memcpy(light->guiInfo.specular, specular, sizeof(float[4]));
}
void LightManager::AddDirectionalLight(string id, float ambient[4], float diffuse[4], float specular[4], float pivot[4], float direction[4])
{
	DirectionalLight light = DirectionalLight();
	FillLightInfo(&light, ambient, diffuse, specular);
	memcpy(light.guiInfo.pivot, pivot, sizeof(float[4]));
	memcpy(light.guiInfo.direction, direction, sizeof(float[4]));

	light.UpdateLightWithGUIInfo();
	dirLights[id] = light;
}
void StrToFloat4(string s, float a[4]) {
	std::stringstream ss(s);
	string coord;
	for (int i = 0; i < 4; i++)
	{
		getline(ss, coord, ',');
		a[i] = atof(coord.c_str());
	}
}
void LightManager::ReadDirectionalLight(string id, string lightStr) 
{
	std::stringstream ss(lightStr);
	float ambient[4], diffuse[4], specular[4], pivot[4], direction[4];
	string ambientStr, diffuseStr, specularStr, pivotStr, directionStr;

	getline(ss, ambientStr, '}');
	StrToFloat4(ambientStr.substr(1), ambient);
	getline(ss, diffuseStr, '}');
	StrToFloat4(diffuseStr.substr(2), diffuse);
	getline(ss, specularStr, '}');
	StrToFloat4(specularStr.substr(2), specular);
	getline(ss, pivotStr, '}');
	StrToFloat4(pivotStr.substr(2), pivot);
	getline(ss, directionStr, '}');
	StrToFloat4(directionStr.substr(2), direction);

	AddDirectionalLight(id, ambient, diffuse, specular, pivot, direction);
}
void LightManager::ReadLightDataFromFile(string filePath)
{
	ifstream myReadFile(filePath);

	string line;
	while (getline(myReadFile, line))
	{
		char type = line[0];
		if (type == '#') continue; //comment
		string id = line.substr(2, line.find_first_of(':')-2);
		string lightStr = line.substr(line.find_first_of(':')+1);
		ReadDirectionalLight(id, lightStr);
	}
	myReadFile.close();
}

//string Float3ToStr(const float a[3]) {
//	return "{" + std::to_string(a[0]) + "," + std::to_string(a[1]) + "," + std::to_string(a[2]) + "}";
//}
void LightManager::WriteLightDataToFile(string filePath)
{
	ofstream myWriteFile(filePath);
	myWriteFile.clear();

	myWriteFile << "#DirLights:{ambient},{diffuse},{specular},{pivot},{direction}\n";
	for (auto it = dirLights.begin(); it != dirLights.end(); it++)
	{
		string id = it->first;
		DirectionalLight* l = &(it->second);
		myWriteFile << "D/" << id << ":" << l->ToString() << endl;
	}
	myWriteFile.close();
}

