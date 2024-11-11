// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"
#include "MyLight.h"
#include "LightManager.h"

using namespace std;
using namespace DirectX;

#define POINT_LIGHT_COUNT 0
#define DIR_LIGHT_COUNT 3

class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightViews[DIR_LIGHT_COUNT];
		XMMATRIX lightProjections[DIR_LIGHT_COUNT];
	};

	//struct DirectionalLight
	//{
	//	XMFLOAT4 lightDir;
	//	XMFLOAT4 ambient;
	//	XMFLOAT4 diffuse;
	//	XMFLOAT4 specular; //(color.rgb, power)
	//};
	struct DirLightBufferType
	{
		DirectionalLight::DirLightInfo dirLights[DIR_LIGHT_COUNT];
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, LightManager* lightManager);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);



private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* dirLightBuffer;
};

#endif