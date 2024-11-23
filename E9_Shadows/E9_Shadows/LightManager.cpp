#include "LightManager.h"
#include <sstream>

LightManager::LightManager()
{
	dirLights = map<string, DirectionalLight>();
}

std::map<string, DirectionalLight>::iterator LightManager::GetDirLightsBegin() { return dirLights.begin(); }
std::map<string, DirectionalLight>::iterator LightManager::GetDirLightsEnd() { return dirLights.end(); }
std::map<string, PointLight>::iterator LightManager::GetPointLightsBegin() { return pLights.begin(); }
std::map<string, PointLight>::iterator LightManager::GetPointLightsEnd() { return pLights.end(); }

void StrToFloat4(string s, float a[4]) {
	std::stringstream ss(s);
	string coord;
	for (int i = 0; i < 4; i++)
	{
		getline(ss, coord, ',');
		a[i] = atof(coord.c_str());
	}
}
void LightManager::ReadLightDataFromFile(string filePath) //public
{
	ifstream myReadFile(filePath);

	string line;
	while (getline(myReadFile, line))
	{
		char type = line[0];
		if (type == '#') continue; //it's a comment
		string id = line.substr(2, line.find_first_of(':')-2);
		string lightStr = line.substr(line.find_first_of(':')+1);
		if(type == 'D') ReadDirectionalLight(id, lightStr);
		else if(type == 'P') ReadPointLight(id, lightStr);
		//else if pointlights, spotlights
	}
	myReadFile.close();
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
void LightManager::ReadPointLight(string id, string lightStr)
{
	std::stringstream ss(lightStr);
	float ambient[4], diffuse[4], specular[4], pos[4], att[4];
	string ambientStr, diffuseStr, specularStr, posStr, attStr;

	getline(ss, ambientStr, '}');
	StrToFloat4(ambientStr.substr(1), ambient);
	getline(ss, diffuseStr, '}');
	StrToFloat4(diffuseStr.substr(2), diffuse);
	getline(ss, specularStr, '}');
	StrToFloat4(specularStr.substr(2), specular);
	getline(ss, posStr, '}');
	StrToFloat4(posStr.substr(2), pos);
	getline(ss, attStr, '}');
	StrToFloat4(attStr.substr(2), att);

	AddPointLight(id, ambient, diffuse, specular, pos, att);
}

void LightManager::AddDirectionalLight(string id, float ambient[4], float diffuse[4], float specular[4], float pivot[4], float direction[4])
{
	DirectionalLight light = DirectionalLight();
	memcpy(light.guiInfo.ambient, ambient, sizeof(float[4]));
	memcpy(light.guiInfo.diffuse, diffuse, sizeof(float[4]));
	memcpy(light.guiInfo.specular, specular, sizeof(float[4]));
	memcpy(light.guiInfo.pivot, pivot, sizeof(float[4]));
	memcpy(light.guiInfo.direction, direction, sizeof(float[4]));

	light.UpdateLightWithGUIInfo();
	dirLights[id] = light;
}
void LightManager::AddPointLight(string id, float ambient[4], float diffuse[4], float specular[4], float position[4], float attenuation[4])
{
	PointLight light = PointLight();
	memcpy(light.guiInfo.ambient, ambient, sizeof(float[4]));
	memcpy(light.guiInfo.diffuse, diffuse, sizeof(float[4]));
	memcpy(light.guiInfo.specular, specular, sizeof(float[4]));
	memcpy(light.guiInfo.position, position, sizeof(float[4]));
	memcpy(light.guiInfo.attenuation, attenuation, sizeof(float[4]));

	light.UpdateLightWithGUIInfo();
	pLights[id] = light;
}

void LightManager::WriteLightDataToFile(string filePath) //public
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
	myWriteFile << "#PointLights:{ambient},{diffuse},{specular},{position},{attFactors.xyz, range}\n";
	for (auto it = pLights.begin(); it != pLights.end(); it++)
	{
		string id = it->first;
		PointLight* l = &(it->second);
		myWriteFile << "P/" << id << ":" << l->ToString() << endl;
	}
	//add support for pointlights, spot lights
	myWriteFile.close();
}
void LightManager::InitializeLights(ID3D11Device* renderer)
{
	InitializeDirLights(renderer);		
}
void LightManager::InitializeDirLights(ID3D11Device* renderer)
{
	//Create Texture2DArray for directional shadow maps
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 4096;
	texDesc.Height = 4096;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = DIR_LIGHT_COUNT;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; //24-bit (0->1) red channel, 8-bit typeless green channel
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //needed for shadow maps
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	renderer->CreateTexture2D(&texDesc, 0, &dirShadowMaps);

	//Create view to access the shadow map Texture2DArray
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; //24-bit (0->1) red channel, 8-bit unused and typeless alpha channel  
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	renderer->CreateShaderResourceView(dirShadowMaps, &srvDesc, &dirShadowMapsSRV);

	//Initialize shadow map and projection matrix
	DirectionalLight* dirLight;
	int i = 0;
	for (auto it = dirLights.begin(); it != dirLights.end(); it++, i++)
	{
		string id = it->first;
		dirLight = &(it->second);

		dirLight->shadowMap = new ShadowMap(renderer, shadowmapWidth, shadowmapHeight, dirShadowMaps, i);
		dirLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
	}
}

