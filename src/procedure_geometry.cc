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
void create_bones(struct Mesh& mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces)
{
	Skeleton& skeleton = mesh.skeleton;
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
	Joint& root = skeleton.joints[skeleton.rootJoint];
	
	recurse_joint_t(mesh, glm::mat4(1.0f), root, root.offset, bone_vertices, bone_faces);

	// for (int i = 0; i < skeleton.bones.size(); i++) {
	// 	std::cout << "root joint id: " << skeleton.bones[i].jointStart << " child joint id: " << skeleton.bones[i].jointEnd << std::endl;
	// 	std::cout << glm::to_string(skeleton.bones[i].translation) << std::endl;
	// 	std::cout << glm::to_string(skeleton.bones[i].rotation) << std::endl;

	// }
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

void recurse_joint_t(struct Mesh& mesh, glm::mat4 transformationSoFar, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces) {
	for (int i = 0; i < parent.children.size(); i++) {
		Joint currJoint = mesh.skeleton.joints[parent.children[i]];
		glm::vec3 jointStartCoords = offsetSoFar;
		glm::vec3 jointEndCoords = currJoint.offset + offsetSoFar;

		Bone& currBone = mesh.skeleton.bones[parent.outBones[i]];
		glm::mat4 oldTranslation = transformationSoFar;
		glm::vec4 translationCoords = glm::inverse(transformationSoFar) * glm::vec4(offsetSoFar, 1.0f);
		currBone.translation = glm::transpose(glm::mat4(1.0f, 0.0f, 0.0f, translationCoords[0],
														0.0f, 1.0f, 0.0f, translationCoords[1],
														0.0f, 0.0f, 1.0f, translationCoords[2],
														0.0f, 0.0f, 0.0f, 1.0f));
		
		transformationSoFar = transformationSoFar * currBone.translation;
		glm::vec4 rotationCoords = (glm::inverse(transformationSoFar) * glm::vec4(jointEndCoords, 1.0f)) / currBone.length;

		glm::vec3 t = glm::normalize(glm::vec3(rotationCoords));
		glm::vec3 v;
		float min = std::min(std::min(std::abs(t[0]),std::abs(t[1])),std::abs(t[2]));
		if (std::abs(t[0]) == min) {
			v = glm::vec3(1.0f, 0.0f, 0.0f);
		} else if (std::abs(t[1]) == min) {
			v = glm::vec3(0.0f, 1.0f, 0.0f);
		} else if (std::abs(t[2]) == min) {
			v = glm::vec3(0.0f, 0.0f, 1.0f);
		}
		glm::vec3 n = glm::cross(t, v) / glm::length(glm::cross(t, v)); // normalizing t x v is slightly different
		glm::vec3 b = glm::cross(t, n);

		currBone.rotation = glm::mat4(n.x, n.y, n.z, 0.0f,
									  b.x, b.y, b.z, 0.0f,
									  t.x, t.y, t.z, 0.0f,
									  0.0f, 0.0f, 0.0f, 1.0f);

		glm::vec4 vertStart = transformationSoFar * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		transformationSoFar = transformationSoFar * currBone.rotation;
		glm::vec4 vertEnd = transformationSoFar * glm::vec4(0.0f, 0.0f, currBone.length, 1.0f);

		bone_vertices.push_back(vertStart);
		bone_vertices.push_back(vertEnd);

		bone_faces.push_back(glm::uvec2(bone_vertices.size()-2, bone_vertices.size()-1));
		
		recurse_joint_t(mesh, transformationSoFar, currJoint, jointEndCoords, bone_vertices, bone_faces);
	}
}