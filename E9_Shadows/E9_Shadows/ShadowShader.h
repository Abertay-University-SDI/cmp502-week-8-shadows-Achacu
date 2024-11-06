// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"

using namespace std;
using namespace DirectX;

#define POINT_LIGHT_COUNT 0
#define DIR_LIGHT_COUNT 1

class ShadowShader : public BaseShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	struct DirLightBufferType
	{
		XMFLOAT4 ambient[DIR_LIGHT_COUNT];
		XMFLOAT4 diffuse[DIR_LIGHT_COUNT];
		XMFLOAT4 direction[DIR_LIGHT_COUNT];
		XMFLOAT4 specular[DIR_LIGHT_COUNT]; //(color.rgb, power)
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, ID3D11ShaderResourceView* depthMap, std::vector<Light*> dirLights);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);


private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* dirLightBuffer;
};

#endif