// Light shader.h
// Basic single light shader setup
#ifndef _SHADOWSHADER_H_
#define _SHADOWSHADER_H_

#include "DXF.h"
#include "LightingUtilsC++.h"

using namespace std;
using namespace DirectX;

class ShadowShader : public BaseShader
{
public:	
	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, ID3D11ShaderResourceView* texture, LightManager* lightManager, Camera* cam);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightMatrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateShadow;

	ID3D11Buffer* lightBuffer;

	ID3D11Buffer* cameraBuffer;

};

#endif