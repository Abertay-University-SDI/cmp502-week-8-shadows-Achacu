// texture shader.cpp
#include "shadowshader.h"


ShadowShader::ShadowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"shadow_vs.cso", L"shadow_ps.cso");
}

ShadowShader::~ShadowShader()
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
	if (dirLightBuffer)
	{	
		dirLightBuffer->Release();
		dirLightBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void ShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
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

	// Sampler for shadow map sampling.
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
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup light buffer
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(DirLightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &dirLightBuffer);

	////create Texture2DArray for shadow maps
	//D3D11_TEXTURE2D_DESC dirShadowMapsDesc;
	//dirShadowMapsDesc.Width = 4096; //TODO: read value from LigthManager
	//dirShadowMapsDesc.Height = 4096; //TODO: read value from LigthManager
	//dirShadowMapsDesc.MipLevels = 1;
	////dirShadowMapsDesc.ArraySize = DIR_LIGHT_COUNT;
	//dirShadowMapsDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; //same as in ShadowMap.cpp
	//dirShadowMapsDesc.SampleDesc.Count = 1;
	//dirShadowMapsDesc.Usage = D3D11_USAGE_DEFAULT;
	//dirShadowMapsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; //same as in ShadowMap.cpp
	//dirShadowMapsDesc.CPUAccessFlags = 0;
	//dirShadowMapsDesc.MiscFlags = 0;	
	//renderer->CreateTexture2D(&dirShadowMapsDesc, NULL, &dirShadowMaps);


	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 4096;
	texDesc.Height = 4096;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = DIR_LIGHT_COUNT;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	//ID3D11Texture2D* depthMap = 0;
	renderer->CreateTexture2D(&texDesc, 0, &dirShadowMaps);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;
	
	for (int i = 0; i < texDesc.ArraySize; i++)
	{
		dirShadowMapsDSVs.push_back(NULL);
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		renderer->CreateDepthStencilView(dirShadowMaps, &dsvDesc, &dirShadowMapsDSVs[i]);
	}
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2DArray.ArraySize = texDesc.ArraySize;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	renderer->CreateShaderResourceView(dirShadowMaps, &srvDesc, &dirShadowMapsSRV);

	
	////Create view to access the shadow map Texture2DArray
	//D3D11_DEPTH_STENCIL_VIEW_DESC dirShadowMapsDSVDesc;
	//dirShadowMapsDSVDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//dirShadowMapsDesc.Format;
	//dirShadowMapsDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;//ARRAY;
	////dirShadowMapsDSVDesc.Texture2DArray.MipSlice = 0; //no mipmaps; just get the 1st one
	////dirShadowMapsDSVDesc.Texture2DArray.ArraySize = 1;
	////dirShadowMapsDSVDesc.Texture2DArray.FirstArraySlice = 0;
	//renderer->CreateDepthStencilView(dirShadowMaps, &dirShadowMapsDSVDesc, &dirShadowMapsDSV);	

	//D3D11_SHADER_RESOURCE_VIEW_DESC dirShadowMapsSRVDesc = {};
	//dirShadowMapsSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	//dirShadowMapsSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;//ARRAY;
	////dirShadowMapsSRVDesc.Texture2DArray.MostDetailedMip = 0;
	////dirShadowMapsSRVDesc.Texture2DArray.MipLevels = dirShadowMapsDesc.MipLevels;
	////dirShadowMapsSRVDesc.Texture2DArray.ArraySize = dirShadowMapsDesc.ArraySize;

	//renderer->CreateShaderResourceView(dirShadowMaps, &dirShadowMapsSRVDesc, &dirShadowMapsSRV);
}

	//RenderTarget = new ID3D11RenderTargetView * [targets];
	//for (USHORT i = 0; i < targets; i++) 
	//{ 
	//	srtDesc.Format = sTexDesc.Format; 
	//	srtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY; 
	//	srtDesc.Texture2DArray.MipSlice = 0; 
	//	srtDesc.Texture2DArray.ArraySize = 1; 
	//	srtDesc.Texture2DArray.FirstArraySlice = i; 
	//	hr = m_pd3dDevice->CreateRenderTargetView(m_pInputView, &srtDesc, &RenderTarget[i]); 
	//}

void ShadowShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, LightManager* lightManager, Camera* cam)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	DirLightBufferType* lightPtr;
	
	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

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
	deviceContext->Map(dirLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	DirLightBufferType* dirLightPtr;
	dirLightPtr = (DirLightBufferType*)mappedResource.pData;
	DirectionalLight* dirLight;

	//ID3D11ShaderResourceView* shadowMaps[DIR_LIGHT_COUNT] = {};
	XMMATRIX lightViews[DIR_LIGHT_COUNT] = {};
	XMMATRIX lightProjections[DIR_LIGHT_COUNT] = {};

	int i = 0;
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++, i++)
	{
		DirectionalLight* dirLight = &(it->second);
	
		//shadowMaps[i] = dirLight->shadowMap->getDepthMapSRV();
		lightViews[i] = XMMatrixTranspose(dirLight->getViewMatrix());
		lightProjections[i] = XMMatrixTranspose(dirLight->getProjectionMatrix());
		dirLightPtr->dirLights[i] = dirLight->info;
	}
	deviceContext->Unmap(dirLightBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &dirLightBuffer);

	// Lock the constant buffer so it can be written to.
	deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	memcpy(dataPtr->lightViews, lightViews, sizeof(XMMATRIX) * DIR_LIGHT_COUNT);
	memcpy(dataPtr->lightProjections, lightProjections, sizeof(XMMATRIX) * DIR_LIGHT_COUNT);
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);
	deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);
	deviceContext->PSSetShaderResources(1, 1, &dirShadowMapsSRV); //i+1 since 0 is reserved for texture
}

