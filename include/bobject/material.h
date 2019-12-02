#pragma once
#include<string>
#include<map>
#include<vector>
#include <glm/glm.hpp>
struct Material
{

	Material() {};
	
	std::string uuid;
	float opacity = 1.0;
	bool visible = true;
	std::map<std::string, std::string> userData;
	std::string type = "Material";
	glm::vec3 color;
	glm::vec3 albedo =glm::vec3(0.45);
	float metallic = 0.5f;
	float roughness = 0.8f;
	float ao = 0.8f;
	bool wireframe;
	bool update = true;
	// control mask
	bool flag = true;
	bool transparent = false;
    double shiness = 0.0;
    std::string name;
};

struct MaterialPbr {
	// Parameter block used as push constant block
	struct PushBlock {
		float roughness;
		float metallic;
		float r, g, b;
	} params;
	std::string name;
	MaterialPbr() {};
	MaterialPbr(std::string n, glm::vec3 c, float r, float m) : name(n) {
		params.roughness = r;
		params.metallic = m;
		params.r = c.r;
		params.g = c.g;
		params.b = c.b;
	};
};
class Materials
{
public:
	Materials();

	~Materials();
	void setMateiral();
	std::vector<MaterialPbr> materials;
	int32_t materialIndex = 0;

	std::vector<std::string> materialNames;
private:

};


