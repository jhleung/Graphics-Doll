#ifndef BONE_GEOMETRY_H
#define BONE_GEOMETRY_H

#include <ostream>
#include <vector>
#include <map>
#include <limits>
#include <glm/glm.hpp>
#include <mmdadapter.h>
#include "mmd/mmd.hxx"
struct BoundingBox {
	BoundingBox()
		: min(glm::vec3(-std::numeric_limits<float>::max())),
		max(glm::vec3(std::numeric_limits<float>::max())) {}
	glm::vec3 min;
	glm::vec3 max;
};

struct Joint {
	// FIXME: Implement your Joint data structure.
	// Note: PMD represents weights on joints, but you need weights on
	//       bones to calculate the actual animation.
	int id;
	int parentID;
	glm::vec3 offset;

	int inBone;
	std::vector<int> outBones;

	std::vector<int> children;
	// start at root and recurse on children, accumulating the offsets
	// from and to endpoints
};

struct Bone {
	int jointStart; // should these be pointers?
	int jointEnd;
	float length;
	//length

	glm::vec3 t; // direction
	glm::vec3 n; // arbitrary unit vector perpindicular to t
	glm::vec3 b; // binormal direction
	glm::mat4 translation;
	glm::mat4 rotation;
	glm::mat4 transformation;
	glm::mat4 oldTransformation;
	glm::mat4 blending;
	// orientation matrix is just tnb
	// transformation matrices (translate/rotation)

};

struct Skeleton {
	// FIXME: create skeleton and bone data structures
	int rootJoint;
	std::vector<Bone> bones;
	std::vector<Joint> joints;
	bool dirty = false;
};


struct Mesh {
	Mesh();
	~Mesh();
	std::vector<glm::vec4> vertices;
	std::vector<glm::vec4> animated_vertices;
	std::vector<glm::uvec3> faces;
	std::vector<glm::vec4> vertex_normals;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<Material> materials;
	BoundingBox bounds;
	Skeleton skeleton;

	void loadpmd(const std::string& fn);
	void updateAnimation();
	int getNumberOfBones() const 
	{ 
		return 0;
		// FIXME: return number of bones in skeleton
	}
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(bounds.min + bounds.max); }
private:
	void computeBounds();
	void computeNormals();
};

#endif
