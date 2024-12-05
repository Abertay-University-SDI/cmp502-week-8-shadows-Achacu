// Light shader.h
// Basic single light shader setup
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class TessellationShader : public BaseShader
{

public:

	TessellationShader(ID3D11Device* device, HWND hwnd);
	~TessellationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, float tessellationFactors[4], Camera* cam, float tessellationRange[2], ID3D11ShaderResourceView* heightTex);
private:
	struct TessellationBufferType
	{
		XMFLOAT4 tessellationFactors;
		XMFLOAT3 camWorldPos;
		float maxTessellationDistance;
	};
	struct WaveBufferType
	{
		float time;
		float A;
		float freq;
		float speed;
	};
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename);
private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* tessellationBuffer;
	ID3D11Buffer* waveBuffer;
	ID3D11SamplerState* heightSampleState;
};
