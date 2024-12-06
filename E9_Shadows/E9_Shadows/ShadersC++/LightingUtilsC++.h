#pragma once
#ifndef _LIGHTINGUTILS_H_
#define _LIGHTINGUTILS_H_

#include "DXF.h"
#include "../Utility/MyLight.h"
#include "../Utility/LightManager.h"

using namespace std;
using namespace DirectX;

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};
struct LightMatrixBufferType
{
	XMMATRIX lightViews[SHADOW_MAP_COUNT];
	XMMATRIX lightProjections[SHADOW_MAP_COUNT];
};

struct CameraBufferType
{
	XMFLOAT3 worldPos;
	float padding;
};
struct LightBufferType
{
	DirectionalLight::DirLightInfo dirLights[DIR_LIGHT_COUNT];
	PointLight::PointLightInfo pLights[POINT_LIGHT_COUNT];
	SpotLight::SpotLightInfo sLights[SPOT_LIGHT_COUNT];
	XMMATRIX pLightViews[6 * POINT_LIGHT_COUNT];
	XMMATRIX pLightProjections[POINT_LIGHT_COUNT];
};

inline void InitLightingAndShadowInfo(ID3D11Device* renderer, ID3D11Buffer*& matrixBuffer, ID3D11Buffer*& lightMatrixBuffer, ID3D11SamplerState*& sampleStateShadow, ID3D11Buffer*& cameraBuffer, ID3D11Buffer*& lightBuffer)
{
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	D3D11_BUFFER_DESC lightMatrixBufferDesc;
	lightMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightMatrixBufferDesc.ByteWidth = sizeof(LightMatrixBufferType);
	lightMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightMatrixBufferDesc.MiscFlags = 0;
	lightMatrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightMatrixBufferDesc, NULL, &lightMatrixBuffer);


	// Sampler for shadow map sampling.
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&shadowSamplerDesc, &sampleStateShadow);

	//Setup camera buffer
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup light buffer
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
}

inline void SetLightingAndShadowParameters(ID3D11DeviceContext* deviceContext, D3D11_MAPPED_SUBRESOURCE& mappedResource, Camera* cam, LightManager* lightManager,
	const DirectX::XMMATRIX& tworld, const DirectX::XMMATRIX& tview, const DirectX::XMMATRIX& tproj, ID3D11Buffer* matrixBuffer,
	ID3D11Buffer* lightMatrixBuffer, ID3D11SamplerState* sampleStateShadow, ID3D11Buffer* lightBuffer, ID3D11Buffer* cameraBuffer, bool useDomainShader)
{
	//Send camera data to pixel shader
	CameraBufferType* camPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->worldPos = cam->getPosition();
	camPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);
	deviceContext->PSSetConstantBuffers(1, 1, &cameraBuffer);

	//Additional
	// Send light data to pixel shader
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	LightBufferType* lightsPtr;
	lightsPtr = (LightBufferType*)mappedResource.pData;

	XMMATRIX lightViews[SHADOW_MAP_COUNT] = {};
	XMMATRIX lightProjections[SHADOW_MAP_COUNT] = {};

	int i = 0;
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++, i++)
	{
		DirectionalLight* dirLight = &(it->second);

		lightViews[i] = XMMatrixTranspose(dirLight->getViewMatrix());
		lightProjections[i] = XMMatrixTranspose(dirLight->getProjectionMatrix());
		lightsPtr->dirLights[i] = dirLight->info;
	}
	int j = 0;
	for (auto it = lightManager->GetSpotLightsBegin(); it != lightManager->GetSpotLightsEnd(); it++, j++)
	{
		SpotLight* sLight = &(it->second);
		lightViews[i + j] = XMMatrixTranspose(sLight->getViewMatrix());
		lightProjections[i + j] = XMMatrixTranspose(sLight->getProjectionMatrix());
		lightsPtr->sLights[j] = sLight->info;
	}
	int k = 0;
	for (auto it = lightManager->GetPointLightsBegin(); it != lightManager->GetPointLightsEnd(); it++, k++)
	{
		PointLight* pLight = &(it->second);
		lightsPtr->pLights[k] = pLight->info;

		for (int v = 0; v < 6; v++)
		{
			pLight->generateViewMatrix(v);
			lightsPtr->pLightViews[6 * k + v] = XMMatrixTranspose(pLight->getViewMatrix());
		}

		lightsPtr->pLightProjections[k] = XMMatrixTranspose(pLight->getProjectionMatrix());
	}
	deviceContext->Unmap(lightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	MatrixBufferType* matrixPtr;
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matrixPtr = (MatrixBufferType*)mappedResource.pData;
	matrixPtr->world = tworld;// worldMatrix;
	matrixPtr->view = tview;
	matrixPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);

	LightMatrixBufferType* lightMatrixPtr;
	deviceContext->Map(lightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightMatrixPtr = (LightMatrixBufferType*)mappedResource.pData;
	memcpy(lightMatrixPtr->lightViews, lightViews, sizeof(lightViews));
	memcpy(lightMatrixPtr->lightProjections, lightProjections, sizeof(lightProjections));
	deviceContext->Unmap(lightMatrixBuffer, 0);


	if (!useDomainShader)
	{
		deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
		deviceContext->VSSetConstantBuffers(1, 1, &lightMatrixBuffer);
	}
	else
	{
		deviceContext->DSSetConstantBuffers(0, 1, &matrixBuffer);
		deviceContext->DSSetConstantBuffers(1, 1, &lightMatrixBuffer);
	}

	//shadow maps
	deviceContext->PSSetShaderResources(0, 1, &lightManager->dirShadowMapsSRV);
	deviceContext->PSSetShaderResources(1, 1, &lightManager->sShadowMapsSRV);
	deviceContext->PSSetShaderResources(2, 1, &lightManager->pShadowMapsSRV);
	deviceContext->PSSetSamplers(0, 1, &sampleStateShadow);
}

#endif
