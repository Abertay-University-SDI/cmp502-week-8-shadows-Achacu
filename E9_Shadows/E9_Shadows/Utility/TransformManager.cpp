#include "TransformManager.h"
#include <sstream>

TransformManager::TransformManager()
{
	transforms = map<string, Transform>();
}

XMMATRIX TransformManager::GetTransformMatrix(string id)
{
	Transform* t = &transforms.at(id);
	return XMMatrixScaling(t->scale[0], t->scale[1], t->scale[2]) * XMMatrixRotationRollPitchYaw(t->rotation[0], t->rotation[1], t->rotation[2])
		* XMMatrixTranslation(t->position[0], t->position[1], t->position[2]);
}
XMMATRIX TransformManager::GetUnscaledTransformMatrix(string id)
{
	Transform* t = &transforms.at(id);
	return XMMatrixRotationRollPitchYaw(t->rotation[0], t->rotation[1], t->rotation[2])
		* XMMatrixTranslation(t->position[0], t->position[1], t->position[2]);
}

TransformManager::Transform* TransformManager::GetTransform(string id)
{
	return &transforms.at(id);
}
std::map<string, TransformManager::Transform>::iterator TransformManager::GetBegin() {
	return transforms.begin();
}
std::map<string, TransformManager::Transform>::iterator TransformManager::GetEnd() {
	return transforms.end();
}
void TransformManager::AddTransform(string id, float pos[3], float rot[3], float scale[3])
{
	Transform t = Transform();
	t.position[0] = pos[0];
	t.position[1] = pos[1];
	t.position[2] = pos[2];
	t.rotation[0] = rot[0];
	t.rotation[1] = rot[1];
	t.rotation[2] = rot[2];
	t.scale[0] = scale[0];
	t.scale[1] = scale[1];
	t.scale[2] = scale[2];
	transforms.emplace(id, t);
}
void TransformManager::AddTransform(string id, float pos[3], float rot[3])
{
	float scale[] = { 1,1,1 };
	AddTransform(id, pos, rot, scale);
}

void TransformManager::AddTransform(string id, float pos[3])
{
	float rot[] = { 0,0,0 };
	float scale[] = { 1,1,1 };
	AddTransform(id, pos, rot, scale);
}
void StrToFloat3(string s, float a[3]) {
	std::stringstream ss(s);
	string coord;
	for (int i = 0; i < 3; i++) 
	{
		getline(ss, coord, ',');
		a[i] = atof(coord.c_str());
	}
}
void TransformManager::ReadTransformDataFromFile(string filePath)
{
	ifstream myReadFile(filePath);
	
	string line;
	while (getline(myReadFile, line))
	{
		string id = line.substr(0, line.find_first_of(':'));
		string transformStr = line.substr(id.size()+1);

		ReadTransform(id, transformStr);
	}
	myReadFile.close();
}
void TransformManager::ReadTransform(string id, string transformStr)
{
	std::stringstream ss(transformStr);
	float pos[3], rot[3], scale[3];
	string posStr, rotStr, scaleStr;

	getline(ss, posStr, '}');
	StrToFloat3(posStr.substr(1), pos);
	getline(ss, rotStr, '}');
	StrToFloat3(rotStr.substr(2), rot);
	getline(ss, scaleStr, '}');
	StrToFloat3(scaleStr.substr(2), scale);

	AddTransform(id, pos, rot, scale);
}


string Float3ToStr(const float a[3]) {
	return "{" + std::to_string(a[0]) + "," + std::to_string(a[1]) + "," + std::to_string(a[2]) + "}";
}
void TransformManager::WriteTransformDataToFile(string filePath)
{
	ofstream myWriteFile(filePath);

	for (auto it = transforms.begin(); it != transforms.end(); it++)
	{
		string id = it->first;
		TransformManager::Transform* t = &(it->second);
		myWriteFile << id << ":" << Float3ToStr(t->position) << "," << Float3ToStr(t->rotation) << "," << Float3ToStr(t->scale) << endl;
	}
	myWriteFile.close();
}
