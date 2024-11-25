// Colour shader.h
// Simple shader example.
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{

public:
	//struct LightBufferType
	//{
	//	XMFLOAT3 lightPos;
	//	float range;
	//};

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();


	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix/*, XMFLOAT3 lightPos, float lightRange*/);
private:
	void initShader(const wchar_t* vs, const wchar_t* ps);


private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightBuffer;
};
