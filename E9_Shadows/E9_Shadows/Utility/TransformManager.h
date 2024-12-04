#pragma once
#include <string>
#include "DXF.h"	// include dxframework
#include <iostream>
#include <fstream>

class TransformManager
{
public:
	TransformManager();

	struct Transform {
		float position[3] = { 0,0,0 };
		float rotation[3] = { 0,0,0 };
		float scale[3] = { 1,1,1 };
	};

	XMMATRIX GetTransformMatrix(string id);
	Transform* GetTransform(string id);

	std::map<string, Transform>::iterator GetBegin();
	std::map<string, Transform>::iterator GetEnd();
	
	void AddTransform(string id, float pos[3], float rot[3], float scale[3]);
	void AddTransform(string id, float pos[3], float rot[3]);
	void AddTransform(string id, float pos[3]);

	void ReadTransformDataFromFile(string filePath);
	void WriteTransformDataToFile(string filePath);
private:
	std::map<string, Transform> transforms;
};

