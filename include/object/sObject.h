#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class SObject
{
public:
	SObject();
	~SObject();
protected:
	std::string type;
	glm::vec3 position;

private:
	// 私有属性不可继承
};