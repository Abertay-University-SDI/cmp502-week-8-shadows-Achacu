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

//Initial light values
//float ambientColor[POINT_LIGHT_COUNT + DIR_LIGHT_COUNT][4] = { 0,0,0,1 };
//float diffuseColor[POINT_LIGHT_COUNT + DIR_LIGHT_COUNT][4] = { 1,1,1,1 };
//float specular[POINT_LIGHT_COUNT + DIR_LIGHT_COUNT][4] = { 0,0,0,0 }; //(color.rgb, specularPower)
//
//float direction[DIR_LIGHT_COUNT][3] = { 0,-1,0 };
//
//float position[POINT_LIGHT_COUNT + DIR_LIGHT_COUNT][3] = { 0,10,0 };
//float attenuation[POINT_LIGHT_COUNT][3] = { 0.5f,0.125f,0.0f, 0.5f,0.125f,0.0f };


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

	// Variables for defining shadow map
	int shadowmapWidth = 4096;
	int shadowmapHeight = 4096;
	int sceneWidth = 100;
	int sceneHeight = 100;

	// This is your shadow map

	DirectionalLight* dirLight;
	for (int i = 0, j = POINT_LIGHT_COUNT; i < DIR_LIGHT_COUNT; i++, j++)
	{
		dirLight = new DirectionalLight();
		dirLights.push_back(dirLight);

		//dirLight->setPosition(position[i][0], position[i][1], position[i][2]);
		//dirLight->setDirection(direction[i][0], direction[i][1], direction[i][2]);
		//dirLight->setDiffuseColour(diffuseColor[j][0], diffuseColor[j][1], diffuseColor[j][2], diffuseColor[j][3]);
		//dirLight->setAmbientColour(ambientColor[j][0], ambientColor[j][1], ambientColor[j][2], ambientColor[j][3]);
		//dirLight->setSpecularColour(specular[j][0], specular[j][1], specular[j][2], 1.0f);
		//dirLight->setSpecularPower(specular[j][3]);

		dirLight->shadowMap = new ShadowMap(renderer->getDevice(), shadowmapWidth, shadowmapHeight);
		dirLight->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
		dirLight->UpdateLightWithGUIInfo(); //PLACEHOLDER FOR FILE READING INITIALIZATION

		//DEBUGGING
		dirLight->info.diffuse = XMFLOAT4(0.9*(i == 0), 0.7*(i == 1),0, 1);
		dirLight->info.direction = XMFLOAT4((i == 0)? 1 : -1, -1, 0, 1);
		//
	}

	// Configure directional light
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(lightDir[0], lightDir[1], lightDir[2]);
	light->setPosition(lightPos[0], lightPos[1], lightPos[2]); //position and direction need to "match" since we are calculating the ortho matrix from the position
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);

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
	for (int i = 0; i < DIR_LIGHT_COUNT; i++) {
		depthPass(dirLights[i]);
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

	//Render light debug sphere
	worldMatrix = XMMatrixTranslation(light->getPosition().x, light->getPosition().y, light->getPosition().z);
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
		textureMgr->getTexture(L"wood"), dirLights);
	shadowShader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	// Render model
	worldMatrix = renderer->getWorldMatrix();
	worldMatrix = XMMatrixScaling(teapotScale[0], teapotScale[1], teapotScale[2]) * XMMatrixRotationRollPitchYaw(teapotRot[0], teapotRot[1], teapotRot[2]) * XMMatrixTranslation(teapotPos[0], teapotPos[1], teapotPos[2]);
	model->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), dirLights);
	shadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	//Render cube
	worldMatrix = XMMatrixTranslation(cubePos[0], cubePos[1], cubePos[2]);
	cube->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), dirLights);
	shadowShader->render(renderer->getDeviceContext(), cube->getIndexCount());

	//Render sphere
	worldMatrix = XMMatrixTranslation(spherePos[0], spherePos[1], spherePos[2]);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), dirLights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());

	//Render light debug sphere
	worldMatrix = XMMatrixTranslation(light->getPosition().x, light->getPosition().y, light->getPosition().z);
	sphere->sendData(renderer->getDeviceContext());
	shadowShader->setShaderParameters(renderer->getDeviceContext(), XMMatrixScaling(0.1,0.1,0.1)*worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), dirLights);
	shadowShader->render(renderer->getDeviceContext(), sphere->getIndexCount());


	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(renderer->getDeviceContext(), renderer->getWorldMatrix(), camera->getOrthoViewMatrix(), renderer->getOrthoMatrix(), dirLights[0]->shadowMap->getDepthMapSRV());
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

	ImGui::Text("Light");
	ImGui::DragFloat("LightDst2C", &lightDstFromCenter, 0.1, 0, 100);
	ImGui::DragFloat3("SceneCenter", sceneCenter, 0.1f, -100, 100);
	if(ImGui::DragFloat3("PositionLight", lightPos, 0.1f, -100, 100))
		light->setPosition(lightPos[0], lightPos[1], lightPos[2]); //position and direction need to "match" since we are calculating the ortho matrix from the position
	if (ImGui::SliderFloat3("DirLight", lightDir, -1, 1))
		light->setDirection(lightDir[0], lightDir[1], lightDir[2]);
	//position correction to match the light direction
	light->setPosition(sceneCenter[0] - lightDir[0] * lightDstFromCenter, sceneCenter[1] - lightDir[1]* lightDstFromCenter, sceneCenter[2] - lightDir[2]* lightDstFromCenter);

	ImGui::Text("Cube");
	ImGui::DragFloat3("PositionCube", cubePos, 0.1f, -50, 50);
	ImGui::Text("Sphere");
	ImGui::DragFloat3("PositionSphere", spherePos, 0.1f, -50, 50);
	ImGui::Text("Teapot");
	ImGui::DragFloat3("PositionTeapot", teapotPos, 0.1f, -50, 50);
	ImGui::SliderFloat3("RotationTeapot", teapotRot, -3.14, 3.14);
	ImGui::SliderFloat3("ScaleTeapot", teapotScale, 0, 3);

	string idStr, ambientStr, diffuseStr, specColStr, specPowStr, dirStr, pivotStr, dstFromPivotStr;
	int dirLightCount = dirLights.size();
	DirectionalLight* dirLight;
	for (int i = 0, j = POINT_LIGHT_COUNT; i < dirLightCount; i++, j++)
	{
		dirLight = dirLights[i];

		idStr = "Light" + to_string(i);
		ambientStr = "AmbientD" + to_string(i);
		diffuseStr = "DiffuseD" + to_string(i);
		specColStr = "SpecColD" + to_string(i);
		specPowStr = "SpecPowD" + to_string(i);
		dirStr = "DirD" + to_string(i);
		pivotStr = "PivotD" + to_string(i);
		dstFromPivotStr = "DstFromPivotD" + to_string(i);
		
		if (ImGui::CollapsingHeader(idStr.c_str(), ImGuiTreeNodeFlags_CollapsingHeader))
		{

			if (ImGui::ColorEdit3(ambientStr.c_str(), dirLight->guiInfo.ambient, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(diffuseStr.c_str(), dirLight->guiInfo.diffuse, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::ColorEdit3(specColStr.c_str(), dirLight->guiInfo.specular, ImGuiColorEditFlags_::ImGuiColorEditFlags_Float) ||
				ImGui::DragFloat(specPowStr.c_str(), &dirLight->guiInfo.specular[3], 1, 1, 32) ||
				ImGui::DragFloat3(pivotStr.c_str(), dirLight->guiInfo.sceneCenter, 0.1, -100, 100) ||
				ImGui::DragFloat(dstFromPivotStr.c_str(), &dirLight->guiInfo.lightDstFromCenter, 0.1, 0, 30) ||
				ImGui::DragFloat3(dirStr.c_str(), dirLight->guiInfo.direction, 0.1, -1, 1))
				dirLight->UpdateLightWithGUIInfo();
		}
		//dirLight = dirLights[i];
		//ImGui::Text("DirectionalLight %d", i);

		////ambient
		//ImGui::PushID(j);
		//if (ImGui::ColorEdit3("ambient", &ambientColor[j][0], ImGuiColorEditFlags_::ImGuiColorEditFlags_Float))
		//	dirLight->setAmbientColour(ambientColor[j][0], ambientColor[j][1], ambientColor[j][2], ambientColor[j][3]);
		//ImGui::PopID();

		////diffuse
		//ImGui::PushID(j);
		//if (ImGui::ColorEdit3("diffuse", &diffuseColor[j][0], ImGuiColorEditFlags_::ImGuiColorEditFlags_Float))
		//	dirLight->setDiffuseColour(diffuseColor[j][0], diffuseColor[j][1], diffuseColor[j][2], diffuseColor[j][3]);
		//ImGui::PopID();

		////specular
		//ImGui::PushID(j);
		//if (ImGui::ColorEdit3("specularColor", &specular[j][0], ImGuiColorEditFlags_::ImGuiColorEditFlags_Float))
		//	dirLight->setSpecularColour(specular[j][0], specular[j][1], specular[j][2], 1.0f);
		//ImGui::PopID();
		//ImGui::PushID(j); //ensures IDs are unique
		//if (ImGui::DragFloat("specularPower", &specular[j][3], 1, 1, 32))
		//	dirLight->setSpecularPower(specular[j][3]);
		//ImGui::PopID();

		////direction
		//ImGui::PushID(i);
		//if (ImGui::DragFloat3("direction", &direction[i][0], 0.1, -1, 1))
		//	dirLight->setDirection(direction[i][0], direction[i][1], direction[i][2]);
		//ImGui::PopID();
	}

	

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

