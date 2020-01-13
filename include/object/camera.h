#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include "object/sObject.h"
#define CAMERA_POSITION_FACTOR 0.6



class Camera :public SObject
{
#pragma region Field

private:
	float _fov;
	float _znear, _zfar;
	const float _step = 0.05f;
	const float tolerance = 0.001f;
	
public:

	bool updated = false;

	struct
	{
		glm::mat4 perspective;
		glm::mat4 view;
	} matrices;
	float rotationSpeed = 1.0f;
	float movementSpeed = 0.10f;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

#pragma region Function
public:
	Camera(float fov, float aspect, float znear, float zfar);
	Camera();
	~Camera();
	void lookat(glm::vec3 target);
	
	float getNearClip();

	float getFarClip();

	void updateAspectRatio(float aspect);

	void setPosition(glm::vec3 position);

	void setPerspective(float fov, float aspect, float znear, float zfar);

	void rotate(glm::vec2 delta);

	void translate(glm::vec3 delta);

	void scale(glm::vec3 delta);

	void rotateZ(glm::vec3 delta);

	float getFov()const;
	float getAspect()const;
	float getZnear()const;
	float getzFar()const;


private:
	void updateViewMatrix();


};