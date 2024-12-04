// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "ShadersC++/TextureShader.h"
#include "ShadersC++/ShadowShader.h"
#include "ShadersC++/DepthShader.h"

#include "Utility/LightManager.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void depthPass(DirectionalLight* dirLight);
	void depthPass(SpotLight* sLight);
	void depthPass(PointLight* pLight);
	void depthPass(/*XMFLOAT3 lightPos, float range,*/ XMMATRIX lightViewMatrix, XMMATRIX lightProjMatrix);
	void finalPass();
	void gui();

private:
	TextureShader* textureShader;
	PlaneMesh* mesh;
	CubeMesh* cube;
	SphereMesh* sphere;
	OrthoMesh* orthoMesh;

	AModel* model;
	ShadowShader* shadowShader;
	DepthShader* depthShader;

	LightManager* lightManager;
};

#endif