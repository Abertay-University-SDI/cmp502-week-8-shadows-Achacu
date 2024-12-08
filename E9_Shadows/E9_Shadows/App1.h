// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "ShadersC++/TextureShader.h"
#include "ShadersC++/ShadowShader.h"
#include "ShadersC++/DepthShader.h"

#include "Utility/LightManager.h"
#include "Utility/TransformManager.h"
#include "ShadersC++/PortalShader.h"
#include "TessellationPlane.h"
#include "ShadersC++/TessellationShader.h"
#include "ShadersC++/HeightmapDepthShader.h"


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
	void RenderSceneObjs(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
	void portalPass();
	void CalculatePortalViewProjMatrix(const DirectX::XMMATRIX& portal1WorldMatrix, const DirectX::XMMATRIX& portal2WorldMatrix, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projMatrix);
	void gui();

	void TransformsGUI();

	void LightGUI();

private:
	TransformManager* tManager;

	TextureShader* textureShader;
	PlaneMesh* mesh;
	CubeMesh* cube;
	SphereMesh* sphere;
	OrthoMesh* orthoMesh;
	AModel* model;

	ShadowShader* shadowShader;
	DepthShader* depthShader;
	LightManager* lightManager;

	QuadMesh* quad;
	PortalShader* portalShader;
	RenderTexture* portalARenderTexture;
	RenderTexture* portalBRenderTexture;

	TessellationPlane* tesPlane;
	TessellationShader* heightMapShader;
	HeightmapDepthShader* heightmapDepthShader;
	float tesDstRange[2] = { 5,100 };
	float tesHeightRange[2] = { 0.5f,5.f };
	float maxTessellation = 12.f;
	ID3D11ShaderResourceView* diffuseTextures[4];
	float diffuseTexScales[4] = {10,20,20,20};
	int samplesPerTexel = 6;
};

#endif