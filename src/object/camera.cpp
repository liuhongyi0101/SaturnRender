#include"object/camera.h"

float MINIMUM_ZOOM_VALUE = 10.0f;
float MINUMUM_DELTA_VALUE = 0.1f;

Camera::Camera(float fov, float aspect, float znear, float zfar) {
	this->type = "camera";
	this->up = glm::vec3(0.0f, 0.0f, 1.0f);
	setPerspective(fov, aspect, znear, zfar);
}
Camera::Camera() {}
Camera::~Camera() {

}
void Camera::updateViewMatrix()
{
	updated = true;
}
void Camera::lookat(glm::vec3 target) {

	matrices.view = glm::lookAt(position, target, up);
}

float Camera::getNearClip() {
	return _znear;
}

float Camera::getFarClip() {
	return _zfar;
}

void Camera::setPerspective(float fov, float aspect, float znear, float zfar)
{
	this->_fov = fov;
	this->_znear = znear;
	this->_zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
};

void Camera::updateAspectRatio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(_fov), aspect, _znear, _zfar);
}

void Camera::setPosition(glm::vec3 position)
{
	this->position = position;
	lookat(target);
}


void Camera::rotate(glm::vec2 delta)
{
	glm::vec3 rotateAround = target;
	glm::vec3 relativeTarget = rotateAround - target;
	glm::vec3 relativePosition = rotateAround - position;
	glm::vec3 modelUp(0.0f, 1.0f, 0.0f);
	glm::vec3 dir = target - position;
	dir = glm::normalize(dir);

	glm::vec3 right = glm::cross(dir, this->up);
	right = glm::normalize(right);
	float d = -0.5f * rotationSpeed;
	glm::quat q1 = glm::angleAxis(glm::radians(d * delta.x), modelUp);
	glm::quat q2 = glm::angleAxis(glm::radians(d * delta.y), right);
	glm::quat q = q1 * q2;

	glm::vec3 newRelativeTarget = q * relativeTarget;
	glm::vec3 newRelativePosition = q * relativePosition;
	glm::vec3 newTarget = rotateAround - newRelativeTarget;
	glm::vec3 newPosition = rotateAround - newRelativePosition;

	position = newPosition;
	target = newTarget;
	up = q * up;
	matrices.view = glm::lookAt(position, target, up);
}

void Camera::rotateZ(glm::vec3 delta)
{
	float angle = glm::length(delta);
	if (angle < tolerance)
	{
		return;
	}
	glm::vec3 axis(0.0f, 0.0f, 1.0f);
	if (delta.x >= 0)
	{
		axis.z = 1.0f;
	}
	else axis.z = -1.0f;

	angle *= rotationSpeed * 0.01;
	glm::quat quaternion = glm::angleAxis(angle, axis);
	glm::vec3 eye_ = position - target;

	eye_ = quaternion * eye_;
	up = quaternion * up;

	position = eye_ + target;
	matrices.view = glm::lookAt(position, target, up);
}

float Camera::getFov() const
{
	return _fov;
}

float Camera::getAspect() const
{
	return 0;
}

float Camera::getZnear() const
{
	return _znear;
}

float Camera::getzFar() const
{
	return _zfar;
}

void Camera::translate(glm::vec3 delta)
{
	float angle = glm::length(delta);
	if (angle < tolerance)
	{
		return;
	}

	glm::vec3 eye = this->position - this->target;
	delta *= glm::length(eye) * movementSpeed;
	// if both delta x and delta is quite small
	if (abs(delta.x) < MINUMUM_DELTA_VALUE && abs(delta.y) < MINUMUM_DELTA_VALUE)
	{
		if (abs(delta.x) > abs(delta.y))
			delta.x = delta.x > 0 ? MINUMUM_DELTA_VALUE : -MINUMUM_DELTA_VALUE;
		else
			delta.y = delta.y > 0 ? MINUMUM_DELTA_VALUE : -MINUMUM_DELTA_VALUE;
	}

	glm::vec3 pan = glm::cross(eye, up) * delta.x;
	pan += up * delta.y;
	this->position += pan;
	target += pan;
	matrices.view = glm::lookAt(this->position, target, up);
}

void Camera::scale(glm::vec3 delta) {
	float factor;
	float step = _step;
	float yy = target.y;
	float zoom = glm::length(this->position - target);
	// if zoom is too short, need to update the target
	if (zoom < MINIMUM_ZOOM_VALUE)
	{
		glm::vec3 dir = glm::normalize(target - position) * 0.5f;
		target = dir + target;
		zoom = glm::length(position - target);
		step *= 3;
		// in case the image is upside down
		if (target.y < position.y && up.y < 0)
		{
			up = glm::vec3(0, 1, 0);
		}
	}

	factor = zoom * step;
	factor = zoom + delta.z * factor;
	this->position = glm::normalize(this->position - target) * factor + target;
	matrices.view = glm::lookAt(this->position, target, up);
}


