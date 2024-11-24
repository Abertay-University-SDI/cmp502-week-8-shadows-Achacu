#pragma once
#include "d3d.h"

using namespace DirectX;

class ShadowCubeMap
{
public:
	ShadowCubeMap(ID3D11Device* device, int mWidth, int mHeight);
	ShadowCubeMap(ID3D11Device* device, int mWidth, int mHeight, ID3D11Texture2D* texArray, int arrayIndex);
	~ShadowCubeMap();

	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc);
	ID3D11ShaderResourceView* getDepthMapSRV() { return mDepthMapSRV; };

private:
	ID3D11DepthStencilView* mDepthMapDSV;
	ID3D11ShaderResourceView* mDepthMapSRV;
	D3D11_VIEWPORT viewport;
	ID3D11RenderTargetView* renderTargets[1];
	ID3D11Texture2D* depthMap;
};

