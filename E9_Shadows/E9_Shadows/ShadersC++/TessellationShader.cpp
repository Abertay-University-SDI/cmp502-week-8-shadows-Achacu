// tessellation shader.cpp
#include "tessellationshader.h"


TessellationShader::TessellationShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"tessellation_vs.cso", L"tessellation_hs.cso", L"tessellation_ds.cso", L"tessellation_ps.cso");
}


TessellationShader::~TessellationShader()
{
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}
	if (layout)
	{
		layout->Release();
		layout = 0;
	}
	
	//Release base shader components
	BaseShader::~BaseShader();
}

void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	D3D11_BUFFER_DESC tessellationBufferDesc;
	tessellationBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	tessellationBufferDesc.ByteWidth = sizeof(TessellationBufferType);
	tessellationBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	tessellationBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tessellationBufferDesc.MiscFlags = 0;
	tessellationBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&tessellationBufferDesc, NULL, &tessellationBuffer);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC heightSamplerDesc;
	heightSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	heightSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	heightSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	heightSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	heightSamplerDesc.MipLODBias = 0.0f;
	heightSamplerDesc.MaxAnisotropy = 1;
	heightSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	heightSamplerDesc.MinLOD = 0;
	heightSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&heightSamplerDesc, &heightSampleState);

	InitLightingAndShadowInfo(renderer, matrixBuffer, lightMatrixBuffer, sampleStateShadow, cameraBuffer, lightBuffer);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	//heightmap texture info needed in pixel shader
	D3D11_BUFFER_DESC heightmapPixelBufferDesc;
	heightmapPixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	heightmapPixelBufferDesc.ByteWidth = sizeof(HeightmapPixelBufferType);
	heightmapPixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	heightmapPixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	heightmapPixelBufferDesc.MiscFlags = 0;
	heightmapPixelBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&heightmapPixelBufferDesc, NULL, &heightmapPixelBuffer);
}

void TessellationShader::initShader(const wchar_t* vsFilename, const wchar_t* hsFilename, const wchar_t* dsFilename, const wchar_t* psFilename)
{
	// InitShader must be overwritten and it will load both vertex and pixel shaders + setup buffers
	initShader(vsFilename, psFilename);

	// Load other required shaders.
	loadHullShader(hsFilename);
	loadDomainShader(dsFilename);
}


void TessellationShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, 
	LightManager* lightManager, Camera* cam, float tesDstRange[2], float tesHeightRange[2], float maxTessellation, 
	ID3D11ShaderResourceView* heightTex, ID3D11ShaderResourceView* diffuseTextures[4], float diffuseTexScales[4], int samplesPerTexel)
{	
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	SetLightingAndShadowParameters(deviceContext, mappedResource, cam, lightManager, tworld, tview, tproj,
		matrixBuffer, lightMatrixBuffer, sampleStateShadow, lightBuffer, cameraBuffer, true);

	deviceContext->HSSetConstantBuffers(1, 1, &matrixBuffer);

	result = deviceContext->Map(tessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TessellationBufferType* tesPtr = (TessellationBufferType*)mappedResource.pData;
	tesPtr->tesDstRange = XMFLOAT2(tesDstRange);
	tesPtr->tesHeightRange = XMFLOAT2(tesHeightRange);
	tesPtr->maxTessellation = maxTessellation;
	tesPtr->camWorldPos = cam->getPosition();
	deviceContext->Unmap(tessellationBuffer, 0);
	deviceContext->HSSetConstantBuffers(0, 1, &tessellationBuffer);

	deviceContext->HSSetShaderResources(3, 1, &heightTex);
	deviceContext->HSSetSamplers(1, 1, &heightSampleState);

	// Set shader height texture resource in the domain shader.
	deviceContext->DSSetShaderResources(3, 1, &heightTex);
	deviceContext->DSSetSamplers(1, 1, &heightSampleState);

	deviceContext->PSSetShaderResources(3, 1, &heightTex);
	deviceContext->PSSetSamplers(1, 1, &heightSampleState);

	//Diffuse textures
	result = deviceContext->Map(heightmapPixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	HeightmapPixelBufferType* heightPixelPtr = (HeightmapPixelBufferType*)mappedResource.pData;
	heightPixelPtr->diffuseTexScales = XMFLOAT4(diffuseTexScales);
	heightPixelPtr->samplesPerTexel = samplesPerTexel;
	heightPixelPtr->padding3 = XMFLOAT3(0,0,0);
	deviceContext->Unmap(heightmapPixelBuffer, 0);

	deviceContext->PSSetConstantBuffers(2, 1, &heightmapPixelBuffer);
	deviceContext->PSSetShaderResources(4, 4, diffuseTextures);
	deviceContext->PSSetSamplers(2, 1, &sampleState);
}


