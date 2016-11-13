#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"
#include <glm/ext.hpp>
#include <iostream>

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.	
void create_bones(struct Mesh mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces)
{
	Skeleton skeleton = mesh.skeleton;
	int startIndex = 0;
	int endIndex = 1;	
	Bone bone;
	Joint start;
	Joint end;
	glm::vec3 jointStartCoords;
	glm::vec3 jointEndCoords;
	glm::vec4 startVertices;
	glm::vec4 endVertices;
	// std::cout << "numbones: " << skeleton.bones.size() << std::endl;
	// std::cout << "numjoints: " << skeleton.joints.size() << std::endl;
	// std::cout << "first joint: " << skeleton.joints[0].id << std::endl;
	Joint root = skeleton.joints[skeleton.rootJoint];
	std::cout << "skeleton bone: " << skeleton.bones.size() << " skeleton joints: " << skeleton.joints.size() << std::endl;

	recurse_joint_t(mesh, glm::mat4(1.0f), root, root.offset, bone_vertices, bone_faces);
}


void recurse_joint(struct Mesh mesh, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces) {
	for (int i = 0; i < parent.children.size(); i++) {
		int currJointIndex = parent.children[i];
		Joint currJoint = mesh.skeleton.joints[currJointIndex];
		glm::vec3 jointStartCoords = offsetSoFar;
		glm::vec4 startVertices = glm::vec4(jointStartCoords[0], jointStartCoords[1], jointStartCoords[2], 1.0f);

		glm::vec3 jointEndCoords = currJoint.offset + offsetSoFar;
		glm::vec4 endVertices = glm::vec4(jointEndCoords[0], jointEndCoords[1], jointEndCoords[2], 1.0f);
		
		bone_vertices.push_back(startVertices);
		bone_vertices.push_back(endVertices);

		bone_faces.push_back(glm::uvec2(bone_vertices.size()-2, bone_vertices.size()-1));

		recurse_joint(mesh, currJoint, jointEndCoords, bone_vertices, bone_faces);
	}
}

void recurse_joint_t(struct Mesh mesh, glm::mat4 transformationSoFar, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces) {
	for (int i = 0; i < parent.children.size(); i++) {
		std::cout << "i: " << i << " parent.children.size(): " << parent.children.size() << std::endl;
		Joint currJoint = mesh.skeleton.joints[parent.children[i]];
		glm::vec3 jointStartCoords = offsetSoFar;
		glm::vec3 jointEndCoords = currJoint.offset + offsetSoFar;

		Bone& currBone = mesh.skeleton.bones[parent.outBones[i]];
		glm::mat4 oldTranslation = transformationSoFar;
		glm::vec4 translationCoords = glm::inverse(transformationSoFar) * glm::vec4(offsetSoFar, 1.0f); // normalize?
		currBone.translation = glm::transpose(glm::mat4(1.0f, 0.0f, 0.0f, translationCoords[0],
														0.0f, 1.0f, 0.0f, translationCoords[1],
														0.0f, 0.0f, 1.0f, translationCoords[2],
														0.0f, 0.0f, 0.0f, 1.0f));
		
		transformationSoFar = transformationSoFar * currBone.translation;
		float boneLength = glm::length(currJoint.offset);
		// std::cout << "boneLength: " << boneLength << " currBone.length: " << currBone.length << std::endl;
		glm::vec4 rotationCoords = (glm::inverse(transformationSoFar) * glm::vec4(jointEndCoords, 1.0f)) / currBone.length;
		printf("rotationCoords: %f, %f, %f\n", rotationCoords.x, rotationCoords.y, rotationCoords.z);

		glm::vec3 t = glm::vec3(rotationCoords);
		glm::vec3 v;
		float min = std::min(std::min(t[0],t[1]),t[2]);
		printf("min: %d t[0]: %f t[1]: %f t[2]: %f\n", min, t[0], t[1], t[2]);
		std::cout << "min of t0 t1 t2 " << std::min(std::min(t[0],t[1]),t[2]) << std::endl;
		if (t[0] == min) {
			v = glm::vec3(1.0f, 0.0f, 0.0f);
		} else if (t[1] == min) {
			v = glm::vec3(0.0f, 1.0f, 0.0f);
		} else if (t[2] == min) {
			v = glm::vec3(0.0f, 0.0f, 1.0f);
		}
		// v = glm::normalize(v);
		glm::vec3 n = glm::cross(t, v) / glm::length(glm::cross(t, v)); // normalizing t x v is slightly different
		glm::vec3 b = glm::cross(t, n); // normalize?
		//n = glm::cross(t, v) / glm::length(glm::cross(t, v)); 
		//n = glm::normalize(n);
		//b = glm::normalize(b);
		printf("v: %f, %f, %f\n", v.x, v.y, v.z);
		printf("n: %f, %f, %f\n", n.x, n.y, n.z);
		printf("b: %f, %f, %f\n", b.x, b.y, b.z);
		printf("t: %f, %f, %f\n", t.x, t.y, t.z);

		currBone.rotation = glm::mat4(n.x, n.y, n.z, 0.0f,
									  b.x, b.y, b.z, 0.0f,
									  t.x, t.y, t.z, 0.0f,
									  0.0f, 0.0f, 0.0f, 1.0f);

		glm::vec4 vertStart = transformationSoFar * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		printf("%f, %f, %f\n", vertStart.x, vertStart.y, vertStart.z);
		//std::cout << "A1: " << glm::to_string(transformationSoFar) << std::endl;

		transformationSoFar = transformationSoFar * currBone.rotation;
		glm::vec4 vertEnd = transformationSoFar * glm::vec4(0.0f, 0.0f, currBone.length, 1.0f);
		printf("%f, %f, %f\n", vertEnd.x, vertEnd.y, vertEnd.z);
		bone_vertices.push_back(vertStart);
		bone_vertices.push_back(vertEnd);

		bone_faces.push_back(glm::uvec2(bone_vertices.size()-2, bone_vertices.size()-1));
		//std::cout << "A2: " << glm::to_string(transformationSoFar) << std::endl;
		recurse_joint_t(mesh, transformationSoFar, currJoint, jointEndCoords, bone_vertices, bone_faces);
	}
}