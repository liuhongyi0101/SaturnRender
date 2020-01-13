#include "object/material.h"

Materials::Materials()
{
	materialIndex = 0;
}

Materials::~Materials()
{
}
void Materials::setMateiral() {

	// Setup some default materials (source: https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/)
	materials.push_back(MaterialPbr("Gold", glm::vec3(1.0f, 0.765557f, 0.336057f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Copper", glm::vec3(0.955008f, 0.637427f, 0.538163f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Chromium", glm::vec3(0.549585f, 0.556114f, 0.554256f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Nickel", glm::vec3(0.659777f, 0.608679f, 0.525649f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Titanium", glm::vec3(0.541931f, 0.496791f, 0.449419f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Cobalt", glm::vec3(0.662124f, 0.654864f, 0.633732f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Platinum", glm::vec3(0.672411f, 0.637331f, 0.585456f), 0.1f, 1.0f));
	// Testing materials
	materials.push_back(MaterialPbr("White", glm::vec3(1.0f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Red", glm::vec3(1.0f, 0.0f, 0.0f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Blue", glm::vec3(0.0f, 0.0f, 1.0f), 0.1f, 1.0f));
	materials.push_back(MaterialPbr("Black", glm::vec3(0.0f), 0.1f, 1.0f));

	for (auto material : materials) {
		materialNames.push_back(material.name);
	}
}