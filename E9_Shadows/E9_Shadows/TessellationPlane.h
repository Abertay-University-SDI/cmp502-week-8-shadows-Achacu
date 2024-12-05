#pragma once
#include "BaseMesh.h"

class TessellationPlane : public BaseMesh
{
public:
	TessellationPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 100);
	~TessellationPlane();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST) override;
protected:
	void initBuffers(ID3D11Device* device);
	int resolution;
};

