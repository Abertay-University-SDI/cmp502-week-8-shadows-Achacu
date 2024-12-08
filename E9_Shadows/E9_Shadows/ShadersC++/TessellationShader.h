// Light shader.h
// Basic single light shader setup
#pragma once

#include "LightingUtilsC++.h"
#include "DXF.h"


using namespace std;
using namespace DirectX;


class TessellationShader : public BaseShader
{

public:

	TessellationShader(ID3D11Device* device, HWND hwnd);
	~TessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, LightManager* lightManager, Camera* cam, float tesDstRange[2], float tesHeightRange[2], float maxTessellation, ID3D11ShaderResourceView* heightTex, ID3D11ShaderResourceView* diffuseTextures[4]);
private:
	struct TessellationBufferType
	{
		XMFLOAT2 tesDstRange;
		XMFLOAT2 tesHeightRange;
		XMFLOAT3 camWorldPos;
		float maxTessellation;
	};
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tessellationBuffer;
	ID3D11SamplerState* heightSampleState;

	ID3D11Buffer* lightMatrixBuffer;

	ID3D11Buffer* lightBuffer;
	ID3D11SamplerState* sampleStateShadow;
	ID3D11Buffer* cameraBuffer;
};
