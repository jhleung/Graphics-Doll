#include "config.h"
#include "bone_geometry.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

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
	std::vector<SparseTuple> tups;
	while(mr.getJoint(id, offset, parent)) {
		Joint joint;
		joint.id = id;
		joint.offset = offset;
		joint.parentID = parent;

		if (parent == -1) {
			skeleton.rootJoint = id;
			Bone bone;
			bone.transformation = glm::mat4(1);
			skeleton.bones.push_back(bone);
			joint.inBone = 0;	


		} else {
			Bone bone;
			
			bone.jointStart = parent;
			bone.jointEnd = id;
			
			bone.t = joint.offset;			
			bone.length = glm::length(bone.t);
			
			skeleton.bones.push_back(bone);
	
			skeleton.joints[parent].outBones.push_back(id);
			joint.inBone = id;	
			skeleton.joints[parent].children.push_back(id);

			// iterate through mesh vertices and build up sparsetuples = (i,j,0)

			// iterate through vector of tuples and update joints new vector of weights 
		}
		skeleton.joints.push_back(joint);
		id++;
	}

	mr.getJointWeights(tups);
	for (int i = 0; i < tups.size(); i++) {
		SparseTuple tup = tups[i];
		int jid = tup.jid;
		int vid = tup.vid;
		Joint joint = skeleton.joints[jid];
		float weight = tup.weight;

		for (int b = 0; b < joint.outBones.size(); b++) {
			int bid = joint.outBones[b];
			dok[vid][bid] = weight;
		}
	}

	// FIXME: load skeleton and blend weights from PMD file
	//        also initialize the skeleton as needed
}


void Mesh::updateAnimation()
{
	animated_vertices = vertices;
	for (int i = 0; i < vertices.size(); i++) {
		glm::mat4 translations = glm::mat4(0.0f);
		for(auto const& entry : dok[i]) {
			int bid = entry.first;
			// float weight = entry.second;
			Bone currBone = skeleton.bones[bid];
			glm::mat4 x = currBone.transformation * glm::inverse(currBone.oldTransformation);
			x = dok[i][bid] * x;
			translations = translations + x;
		}
		animated_vertices[i] = translations * vertices[i];
	}
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

