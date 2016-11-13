#include "config.h"
#include "bone_geometry.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

/*
 * For debugging purpose.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	size_t count = std::min(v.size(), static_cast<size_t>(10));
	for (size_t i = 0; i < count; ++i) os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}



// FIXME: Implement bone animation.


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::loadpmd(const std::string& fn)
{
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);

	int id = 0;
	int parent = 0;
	glm::vec3 offset = glm::vec3(0.0f,0.0f,0.0f);
	while(mr.getJoint(id, offset, parent)) {
		Joint joint;
		joint.id = id;
		joint.offset = offset;
		joint.parentID = parent;

		if (parent == -1) {
			skeleton.rootJoint = id;
			Bone bone;
			skeleton.bones.push_back(bone);
		} else {
			Bone bone;
			
			bone.jointStart = parent;
			bone.jointEnd = id;
			
			bone.t = joint.offset;			
			bone.length = glm::length(bone.t);
			// bone.t = glm::normalize(bone.t);
			// glm::vec3 v = bone.t;
			// int min = std::min(std::min(v[0],v[1]),v[2]);
			// if (v[0] == min) {
			// 	v[0] = 1;
			// 	v[1] = 0;
			// 	v[2] = 0;
			// } else if (v[1] == min) {
			// 	v[1] = 1;
			// 	v[0] = 0;
			// 	v[2] = 0;
			// } else if (v[2] == min) {
			// 	v[2] = 1;
			// 	v[0] = 0;
			// 	v[1] = 0;
			// }
			// bone.n = glm::cross(bone.t, v) / glm::length(glm::cross(bone.t, v));
			// bone.b = glm::cross(bone.t, bone.n);
			

			// bone.translation = glm::mat4(1.0f, 0.0f, 0.0f, bone.t[0],
			// 							 0.0f, 1.0f, 0.0f, bone.t[1],
			// 							 0.0f, 0.0f, 1.0f, bone.t[2],
			// 						     0.0f, 0.0f, 0.0f, 1.0f);

			// bone.rotation = glm::mat4(bone.t.x, bone.t.y, bone.t.z, 0.0f,
			// 						  bone.b.x, bone.b.y, bone.b.z, 0.0f,
			// 						  bone.n.x, bone.n.y, bone.n.z, 0.0f,
			// 						  0.0f, 0.0f, 0.0f, 1.0f);
			
			skeleton.bones.push_back(bone);
			int boneIndex = skeleton.bones.size()-1;
	
			skeleton.joints[parent].outBones.push_back(id);
			joint.inBone = id;	

			// if (parent == 0) { 
			// 	skeleton.rootJoint.children.push_back(id);
			// 	skeleton.rootJoint.outBones.push_back(boneIndex);
			// }
			//else
			skeleton.joints[parent].children.push_back(id);
		}
		skeleton.joints.push_back(joint);
		id++;
	}

	// FIXME: load skeleton and blend weights from PMD file
	//        also initialize the skeleton as needed
}

void Mesh::updateAnimation()
{
	animated_vertices = vertices;
	// FIXME: blend the vertices to animated_vertices, rather than copy
	//        the data directly.
}


void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto& vert : vertices) {
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

