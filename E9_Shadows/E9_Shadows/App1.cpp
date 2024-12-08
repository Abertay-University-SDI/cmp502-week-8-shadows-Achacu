// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "Utility/MyLight.h"

App1::App1()
{

}

float spherePos[3] = { 15.f, 2.f, -5.f };
float teapotPos[3] = { 5.f, 2.f, -5.f };
float teapotRot[3] = { 0,0,0 };
float teapotScale[3] = { 0.3,0.3,0.3 };

float lightPos[3] = { 0.f, 0.f, -20.f };
float lightDir[3] = { 0.0f, -0.7f, 0.7f };
float sceneCenter[3] = { 0.0f, 0.0f, 0.0f };
float lightDstFromCenter = 10.0;

int pointLightShadowMapIndex = 0;

float screenAspect;

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	
	//DirectionalLight::DirectionalLightInfo* info = &direLight.info;
	screenAspect = (float)screenWidth / screenHeight;
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	quad = new QuadMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/4, screenHeight/4, -screenWidth/2.7, screenHeight/2.7);

	// Create Mesh object and shader object
	tesPlane = new TessellationPlane(renderer->getDevice(), renderer->getDeviceContext(), 100);

	//textureMgr->loadTexture(L"height", L"res/height.png");
	textureMgr->loadTexture(L"height", L"res/heightmap.png");
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	textureMgr->loadTexture(L"grass", L"res/grass.jpg");
	textureMgr->loadTexture(L"rock", L"res/rock_diffuse.tif");

	diffuseTextures[0] = textureMgr->getTexture(L"grass");
	diffuseTextures[1] = textureMgr->getTexture(L"rock");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);
	portalShader = new PortalShader(renderer->getDevice(), hwnd);
	heightMapShader = new TessellationShader(renderer->getDevice(), hwnd);
	heightmapDepthShader = new HeightmapDepthShader(renderer->getDevice(), hwnd);

	portalARenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	portalBRenderTexture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);


	//Reads light info from file and creates lights
	lightManager = new LightManager();
	lightManager->ReadLightDataFromFile("Utility/lightInfo.txt");
	lightManager->InitializeLights(renderer->getDevice());

	//Reads transform info from file and creates transforms
	tManager = new TransformManager();
	tManager->ReadTransformDataFromFile("Utility/transformData.txt");
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Perform depth pass
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++) 
	{
		depthPass(&(it->second));
	}
	for (auto it = lightManager->GetSpotLightsBegin(); it != lightManager->GetSpotLightsEnd(); it++)
	{
		depthPass(&(it->second));
	}
	for (auto it = lightManager->GetPointLightsBegin(); it != lightManager->GetPointLightsEnd(); it++)
	{
		depthPass(&(it->second));
	}
	portalPass();
	// Render scene
	finalPass();

	return true;
}
void App1::depthPass(DirectionalLight* dirLight)
{
	dirLight->shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	dirLight->generateViewMatrix();
	depthPass(dirLight->getViewMatrix(), dirLight->getOrthoMatrix());
}
void App1::depthPass(SpotLight* sLight)
{
	sLight->shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
	sLight->generateViewMatrix();
	sLight->generatePerspectiveMatrix();
	depthPass(sLight->getViewMatrix(), sLight->getPerspectiveMatrix());
}
void App1::depthPass(PointLight* pLight)
{
	pLight->generatePerspectiveMatrix();

	for (int i = 0; i < 6; i++)
	{
		pLight->shadowMaps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());
		pLight->generateViewMatrix(i);
		depthPass(/*XMFLOAT3(pLight->guiInfo.position), pLight->guiInfo.attenuation[3],*/ pLight->getViewMatrix(), pLight->getPerspectiveMatrix());
	}

}
void App1::depthPass(XMMATRIX lightViewMatrix, XMMATRIX lightProjMatrix)
{	
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjMatrix/*, lightPos, range*/);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	//Render cube
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), tManager->GetTransformMatrix("cube"), lightViewMatrix, lightProjMatrix/*, lightPos, range*/);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	//Render sphere
	worldMatrix = XMMatrixTranslation(spherePos[0], spherePos[1], spherePos[2]);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjMatrix/*, lightPos, range*/);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixScaling(teapotScale[0], teapotScale[1], teapotScale[2]) * XMMatrixRotationRollPitchYaw(teapotRot[0], teapotRot[1], teapotRot[2]) * XMMatrixTranslation(teapotPos[0], teapotPos[1], teapotPos[2]);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjMatrix/*, lightPos, range*/);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	//Tessellated height map mesh
	tesPlane->sendData(renderer->getDeviceContext());
	heightmapDepthShader->setShaderParameters(renderer->getDeviceContext(), tManager->GetTransformMatrix("terrain"), lightViewMatrix, lightProjMatrix,
		camera, tesDstRange, tesHeightRange, maxTessellation, textureMgr->getTexture(L"height"));
	heightmapDepthShader->render(renderer->getDeviceContext(), tesPlane->getIndexCount());


	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	//PointLight* pLight = &lightManager->GetPointLightsBegin()->second;
	//pLight->generateViewMatrix(2);
	//pLight->generatePerspectiveMatrix();
	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = /*pLight->getViewMatrix();*/camera->getViewMatrix();
	XMMATRIX projectionMatrix = /*pLight->getProjectionMatrix();*/renderer->getProjectionMatrix();
	
	RenderSceneObjs(worldMatrix, viewMatrix, projectionMatrix);

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix(), 
		/*lightManager->pShadowMapsSRV*/lightManager->GetPointLightsBegin()->second.shadowMaps[pointLightShadowMapIndex]->getDepthMapSRV());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
}
void App1::RenderSceneObjs(XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix,
		textureMgr->getTexture(L"wood"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixScaling(teapotScale[0], teapotScale[1], teapotScale[2]) * XMMatrixRotationRollPitchYaw(teapotRot[0], teapotRot[1], teapotRot[2]) * XMMatrixTranslation(teapotPos[0], teapotPos[1], teapotPos[2]);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	//Render cube
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), tManager->GetTransformMatrix("cube"), viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	//Render sphere
	worldMatrix = XMMatrixTranslation(spherePos[0], spherePos[1], spherePos[2]);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	//Render light debug cubes
	cube->sendData(renderer->getDeviceContext());
	XMMATRIX debugScale = XMMatrixScaling(0.1, 0.1, 0.1);
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++)
	{
		worldMatrix = (it->second).GetWorldMatrix();
		textureShader->setShaderParameters(renderer->getDeviceContext(), debugScale * worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}
	for (auto it = lightManager->GetPointLightsBegin(); it != lightManager->GetPointLightsEnd(); it++)
	{
		worldMatrix = (it->second).GetWorldMatrix();
		textureShader->setShaderParameters(renderer->getDeviceContext(), debugScale * worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}
	for (auto it = lightManager->GetSpotLightsBegin(); it != lightManager->GetSpotLightsEnd(); it++)
	{
		worldMatrix = (it->second).GetWorldMatrix();
		textureShader->setShaderParameters(renderer->getDeviceContext(), debugScale * worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}

	//portal A
	quad->sendData(renderer->getDeviceContext());
	portalShader->setShaderParameters(renderer->getDeviceContext(), tManager->GetTransformMatrix("portalA"), viewMatrix, projectionMatrix, portalARenderTexture->getShaderResourceView());
	portalShader->render(renderer->getDeviceContext(), quad->getIndexCount());
	//portal B
	portalShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixScaling(-1, 1, 1) * tManager->GetTransformMatrix("portalB"), viewMatrix, projectionMatrix, portalBRenderTexture->getShaderResourceView());
	portalShader->render(renderer->getDeviceContext(), quad->getIndexCount());

	//Tessellated height map mesh
	tesPlane->sendData(renderer->getDeviceContext());
	heightMapShader->setShaderParameters(renderer->getDeviceContext(), tManager->GetTransformMatrix("terrain"), viewMatrix, projectionMatrix,
		lightManager, camera, tesDstRange, tesHeightRange, maxTessellation, textureMgr->getTexture(L"height"), diffuseTextures, diffuseTexScales, samplesPerTexel);
	heightMapShader->render(renderer->getDeviceContext(), tesPlane->getIndexCount());

}

void App1::portalPass()
{
	// Set the render target to be the render to texture and clear it (portalA render texture)
	portalARenderTexture->setRenderTarget(renderer->getDeviceContext());
	portalARenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	//gets view matrix from portal A to portal B
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix, projMatrix;
	CalculatePortalViewProjMatrix(tManager->GetTransformMatrix("portalA"), tManager->GetTransformMatrix("portalB"), viewMatrix, projMatrix);

	RenderSceneObjs(worldMatrix, viewMatrix, projMatrix);

	// Set the render target to be the render to texture and clear it (portalB render texture)
	portalBRenderTexture->setRenderTarget(renderer->getDeviceContext());
	portalBRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	//gets view matrix from portal A to portal B
	CalculatePortalViewProjMatrix(tManager->GetUnscaledTransformMatrix("portalB"), tManager->GetUnscaledTransformMatrix("portalA"), viewMatrix, projMatrix);
	RenderSceneObjs(worldMatrix, viewMatrix, projMatrix);

	renderer->setBackBufferRenderTarget();
}
#define DEG2PI 0.0174532925f
void App1::CalculatePortalViewProjMatrix(const DirectX::XMMATRIX& portal1WorldMatrix, const DirectX::XMMATRIX& portal2WorldMatrix, DirectX::XMMATRIX& viewMatrix, DirectX::XMMATRIX& projMatrix)
{	
	XMMATRIX camWorldMatrix = XMMatrixRotationRollPitchYaw(camera->getRotation().x * DEG2PI, camera->getRotation().y * DEG2PI, camera->getRotation().z * DEG2PI) * XMMatrixTranslation(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
	XMMATRIX portal1WorldToLocal = XMMatrixInverse(NULL, portal1WorldMatrix);

	//These two points are needed to calculate forward and up vectors that can be used to create the view matrix
	XMVECTOR forwardVector = XMVectorSet(0,0,1 ,1);
	XMVECTOR upVector = XMVectorSet(0,1,0 ,1);

	//(0,0,0),(0,0,1) and (0,1,0) are transformed from local camera space to world space. 
	//Their positions relative to portal1 (local portal1 space) are then converted to world space as if relative to portal2
	XMMATRIX m = camWorldMatrix * portal1WorldToLocal * portal2WorldMatrix;
	forwardVector = XMVector4Transform(forwardVector, m);
	upVector = XMVector4Transform(upVector, m);

	XMVECTOR transformedCamPosVector = XMVECTOR(m.r[3]); //the last row of m holds the transformed camera position (transforming (0,0,0) in local camera space)
	//we calculate the forward and up vectors as forwardPoint - camPos and upPoint - camPos
	forwardVector -= transformedCamPosVector;
	upVector -= transformedCamPosVector;

	viewMatrix = XMMatrixLookToLH(transformedCamPosVector, XMVector3Normalize(forwardVector), XMVector3Normalize(upVector));

	//The near plane of the projection needs to match the distance from the transformed camera position to portal2.
	//Otherwise, since the virtual camera is behind portal2 if anything was between the two it would show up in the render texture
	XMVECTOR portal2Pos = portal2WorldMatrix.r[3];
	XMVECTOR dst = XMVector4Length(transformedCamPosVector - portal2Pos);
	projMatrix = XMMatrixPerspectiveFovLH(45 * DEG2PI, screenAspect, dst.m128_f32[0], SCREEN_DEPTH);
}

DirectionalLight myLight = DirectionalLight();
void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	ImGui::SetWindowSize(ImVec2::ImVec2(300, 600));

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::Text("PointShadowmapIndex");
	ImGui::DragInt("PointShadowmapIndex", &pointLightShadowMapIndex, 0.1, 0, 5);
	ImGui::Text("Sphere");
	ImGui::DragFloat3("PositionSphere", spherePos, 0.1f, -50, 50);
	ImGui::Text("Teapot");
	ImGui::DragFloat3("PositionTeapot", teapotPos, 0.1f, -50, 50);
	ImGui::SliderFloat3("RotationTeapot", teapotRot, -3.14, 3.14);
	ImGui::SliderFloat3("ScaleTeapot", teapotScale, 0, 3);

	ImGui::Text("Tessellation");
	ImGui::DragFloat2("TesDstRange", tesDstRange, 0.1f, 0, 200);
	ImGui::DragFloat2("TesHeightRange", tesHeightRange, 0.05f, 0, 50);
	ImGui::SliderFloat("MaxTessellation", &maxTessellation, 1, 64);
	ImGui::Text("Heightmap");
	ImGui::DragFloat4("DiffuseTexScales", diffuseTexScales, 0.05f, 0, 100);
	ImGui::DragInt("SamplesPerTexel", &samplesPerTexel, 0.1f, 1, 30);

	LightGUI();
	
	TransformsGUI();


	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::TransformsGUI()
{
	//display all transform controls
	for (auto it = tManager->GetBegin(); it != tManager->GetEnd(); it++)
	{
		string id = it->first;
		TransformManager::Transform* t = &(it->second);
		string posName = id + "Position";
		string rotName = id + "Rotation";
		string scaleName = id + "Scale";


		if (ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
		{
			ImGui::DragFloat3(posName.c_str(), t->position, 0.1f, -50, 50);
			ImGui::SliderFloat3(rotName.c_str(), t->rotation, -3.14, 3.14);
			ImGui::DragFloat3(scaleName.c_str(), t->scale, 0.1f, -1, 50);
		}
	}
	if (ImGui::Button("Save transforms"))
		tManager->WriteTransformDataToFile("Utility/transformData.txt");
}

void App1::LightGUI()
{
	string ambientStr, diffuseStr, specColStr, specPowStr, dirStr, pivotStr, dstFromPivotStr;
	DirectionalLight* dirLight;
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++)
	{
		string id = it->first;
		dirLight = &(it->second);

		ambientStr = "AmbientD" + id;
		diffuseStr = "DiffuseD" + id;
		specColStr = "SpecColD" + id;
		specPowStr = "SpecPowD" + id;
		dirStr = "DirD" + id;
		pivotStr = "PivotD" + id;
		dstFromPivotStr = "DstFromPivotD" + id;

		if (ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
		{

			if (ImGui::ColorEdit3(ambientStr.c_str(), dirLight->guiInfo.ambient, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(diffuseStr.c_str(), dirLight->guiInfo.diffuse, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(specColStr.c_str(), dirLight->guiInfo.specular, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::DragFloat(specPowStr.c_str(), &dirLight->guiInfo.specular[3], 1, 1, 64) ||
				ImGui::DragFloat3(pivotStr.c_str(), dirLight->guiInfo.pivot, 0.1, -100, 100) ||
				ImGui::DragFloat(dstFromPivotStr.c_str(), &dirLight->guiInfo.pivot[3], 0.1, 0, 30) ||
				ImGui::DragFloat3(dirStr.c_str(), dirLight->guiInfo.direction, 0.1, -1, 1))
				dirLight->UpdateLightWithGUIInfo();
		}
	}
	PointLight* pLight;
	string posStr, attStr, rangeStr;
	for (auto it = lightManager->GetPointLightsBegin(); it != lightManager->GetPointLightsEnd(); it++)
	{
		string id = it->first;
		pLight = &(it->second);

		ambientStr = "AmbientP" + id;
		diffuseStr = "DiffuseP" + id;
		specColStr = "SpecColP" + id;
		specPowStr = "SpecPowP" + id;
		posStr = "PosP" + id;
		attStr = "AttP" + id;
		rangeStr = "RangeP" + id;

		if (ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
		{

			if (ImGui::ColorEdit3(ambientStr.c_str(), pLight->guiInfo.ambient, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(diffuseStr.c_str(), pLight->guiInfo.diffuse, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(specColStr.c_str(), pLight->guiInfo.specular, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::DragFloat(specPowStr.c_str(), &pLight->guiInfo.specular[3], 1, 1, 64) ||
				ImGui::DragFloat3(posStr.c_str(), pLight->guiInfo.position, 0.1, -100, 100) ||
				ImGui::DragFloat3(attStr.c_str(), pLight->guiInfo.attenuation, 0.01, 0, 1) ||
				ImGui::DragFloat(rangeStr.c_str(), &pLight->guiInfo.attenuation[3], 0.1, 0, 30))
				pLight->UpdateLightWithGUIInfo();
		}
	}
	SpotLight* sLight;
	string halfAngleStr, fallOffExpStr;
	for (auto it = lightManager->GetSpotLightsBegin(); it != lightManager->GetSpotLightsEnd(); it++)
	{
		string id = it->first;
		sLight = &(it->second);

		ambientStr = "AmbientS" + id;
		diffuseStr = "DiffuseS" + id;
		specColStr = "SpecColS" + id;
		specPowStr = "SpecPowS" + id;
		posStr = "PosS" + id;
		dirStr = "DirS" + id;
		attStr = "AttS" + id;
		rangeStr = "RangeS" + id;
		halfAngleStr = "HalfAngleS" + id;
		fallOffExpStr = "FalloffExpS" + id;

		if (ImGui::CollapsingHeader(id.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
		{

			if (ImGui::ColorEdit3(ambientStr.c_str(), sLight->guiInfo.ambient, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(diffuseStr.c_str(), sLight->guiInfo.diffuse, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(specColStr.c_str(), sLight->guiInfo.specular, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::DragFloat(specPowStr.c_str(), &sLight->guiInfo.specular[3], 1, 1, 64) ||
				ImGui::DragFloat3(posStr.c_str(), sLight->guiInfo.position, 0.1, -100, 100) ||
				ImGui::DragFloat3(dirStr.c_str(), sLight->guiInfo.direction, 0.1, -1, 1) ||
				ImGui::DragFloat3(attStr.c_str(), sLight->guiInfo.attenuation, 0.01, 0, 1) ||
				ImGui::DragFloat(rangeStr.c_str(), &sLight->guiInfo.attenuation[3], 0.1, 0, 50) ||
				ImGui::SliderAngle(halfAngleStr.c_str(), &sLight->guiInfo.angleFalloff[0], 0, 90) ||
				ImGui::DragFloat(fallOffExpStr.c_str(), &sLight->guiInfo.angleFalloff[1], 0.1, 1, 10))
				sLight->UpdateLightWithGUIInfo();
		}
	}
	if (ImGui::Button("Save light info"))
	{
		lightManager->WriteLightDataToFile("Utility/lightInfo.txt");
	}
}

