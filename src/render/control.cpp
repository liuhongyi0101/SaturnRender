#include "renderer/control.h"
#include "camera.hpp"
Control::Control()
{
	
}

Control::~Control()
{
}
void Control::setupCamera(Camera &camera,int width,int height) {

	camera.type = Camera::CameraType::firstperson;
	camera.setPosition(glm::vec3(10.0f, 13.0f, 1.8f));
	camera.setRotation(glm::vec3(-62.5f, 90.0f, 0.0f));
	camera.movementSpeed = 4.0f;
	camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
	camera.rotationSpeed = 0.25f;

}