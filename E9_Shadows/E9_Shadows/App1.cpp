// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "MyLight.h"

App1::App1()
{

}

float cubePos[3] = { -5.f, 2.f, -5.f };
float spherePos[3] = { 15.f, 2.f, -5.f };
float teapotPos[3] = { 5.f, 2.f, -5.f };
float teapotRot[3] = { 0,0,0 };
float teapotScale[3] = { 0.3,0.3,0.3 };

float lightPos[3] = { 0.f, 0.f, -20.f };
float lightDir[3] = { 0.0f, -0.7f, 0.7f };
float sceneCenter[3] = { 0.0f, 0.0f, 0.0f };
float lightDstFromCenter = 10.0;

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	
	//DirectionalLight::DirectionalLightInfo* info = &direLight.info;

	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");
	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth/2, screenHeight/2, -screenWidth/2.7, screenHeight/2.7);

	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");

	// initial shaders
	textureShader = new TextureShader(renderer->getDevice(), hwnd);
	depthShader = new DepthShader(renderer->getDevice(), hwnd);
	shadowShader = new ShadowShader(renderer->getDevice(), hwnd);



	//Reads light info from file and creates lights
	lightManager = new LightManager();
	lightManager->ReadLightDataFromFile("lightInfo.txt");
	lightManager->InitializeLights(renderer->getDevice());


	//lightManager->AddDirectionalLight("light1", new float[4] {0, 0, 0, 1}, new float[4] {1, 1, 0, 1}, new float[4] {0, 0, 0, 0},new float[4] {2, 5, 5, 10}, new float[4] {-0.7, -0.7, 0, 0});
	//lightManager->AddDirectionalLight("light2", new float[4] {0, 0.234, 0, 1}, new float[4] {1, 1, 1, 1}, new float[4] {0, 0, 1, 0.2},new float[4] {2, 3, 4, 20}, new float[4] {-0.7, 0.7, 0.7, 0});
	//string s = lightManager->GetDirectionalLight("light1")->ToString();
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
	// Render scene
	finalPass();

	return true;
}

void App1::depthPass(DirectionalLight* dirLight)
{
	// Set the render target to be the render to texture.
	dirLight->shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get the world, view, and projection matrices from the camera and d3d objects.
	dirLight->generateViewMatrix();
	XMMATRIX lightViewMatrix = dirLight->getViewMatrix();
	XMMATRIX lightProjectionMatrix = dirLight->getOrthoMatrix();
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	// Render floor
	mesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	//Render cube
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]);
	cube->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	//Render sphere
	worldMatrix = XMMatrixTranslation(spherePos[0], spherePos[1], spherePos[2]);
	sphere->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixScaling(teapotScale[0], teapotScale[1], teapotScale[2]) * XMMatrixRotationRollPitchYaw(teapotRot[0], teapotRot[1], teapotRot[2]) * XMMatrixTranslation(teapotPos[0], teapotPos[1], teapotPos[2]);
	// Render model
	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	camera->update();

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	

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
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]);
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	//Render sphere
	worldMatrix = XMMatrixTranslation(spherePos[0], spherePos[1], spherePos[2]);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), lightManager, camera);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	//Render light debug cubes
	cube->sendData(renderer->getDeviceContext());
	for (auto it = lightManager->GetDirLightsBegin(); it != lightManager->GetDirLightsEnd(); it++)
	{
		worldMatrix = (it->second).GetWorldMatrix();
		textureShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixScaling(0.1,0.1,0.1)*worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), cube->getIndexCount());
	}


	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix(), lightManager->dirShadowMapsSRV/*lightManager->GetDirLightsBegin()->second.shadowMap->getDepthMapSRV()*/);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	gui();
	renderer->endScene();
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

	ImGui::Text("Cube");
	ImGui::DragFloat3("PositionCube", cubePos, 0.1f, -50, 50);
	ImGui::Text("Sphere");
	ImGui::DragFloat3("PositionSphere", spherePos, 0.1f, -50, 50);
	ImGui::Text("Teapot");
	ImGui::DragFloat3("PositionTeapot", teapotPos, 0.1f, -50, 50);
	ImGui::SliderFloat3("RotationTeapot", teapotRot, -3.14, 3.14);
	ImGui::SliderFloat3("ScaleTeapot", teapotScale, 0, 3);

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
	if (ImGui::Button("Save light info")) {
		lightManager->WriteLightDataToFile("lightInfo.txt");
	}
	

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

