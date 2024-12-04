#include "PortalShader.h"

PortalShader::PortalShader(ID3D11Device* device, HWND hwnd) : TextureShader(device, hwnd)
{
	initShader(L"portal_vs.cso", L"portal_ps.cso");
}