#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"

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
	Joint root =  skeleton.rootJoint;
	std::cout << "root children size: " << root.children.size() << std::endl;
	std::cout << "root id: " << root.id << std::endl;
	std::cout << "root parent id: " << root.parentID << std::endl;
	std::cout << "joints first " << skeleton.joints[0].parentID << std::endl;
	recurse_joint(mesh, root, root.offset, bone_vertices, bone_faces);
}


void recurse_joint(struct Mesh mesh, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces) {
	if (parent.children.size() > 0) {
		for (int i = 0; i < parent.children.size(); i++) {
			int currJointIndex = parent.children[i];
			Joint currJoint = mesh.skeleton.joints[currJointIndex];
			glm::vec3 jointStartCoords = offsetSoFar;
			glm::vec4 startVertices = glm::vec4(jointStartCoords[0], jointStartCoords[1], jointStartCoords[2], 0.0f);

			glm::vec3 jointEndCoords = currJoint.offset + offsetSoFar;
			glm::vec4 endVertices = glm::vec4(jointEndCoords[0], jointEndCoords[1], jointEndCoords[2], 0.0f);
			
			bone_vertices.push_back(startVertices);
			bone_vertices.push_back(endVertices);

			bone_faces.push_back(glm::uvec2(bone_vertices.size()-2, bone_vertices.size()-1));
			//glm::vec3 world_coords = root.offset + recurse_joint(mesh, mesh.skeleton.joints[root.parentID]);

			recurse_joint(mesh, currJoint, jointEndCoords, bone_vertices, bone_faces);
		}
		
	}
}
// glm::vec3 recurse_joint(struct Mesh mesh, struct Joint root) {
// 	if (root.parentID == -1) {
// 		std::cout << "base case" << std::endl;
// 		return root.offset;
// 	}
// 	std::cout << "appended offset: " << root.offset.x << " " << root.offset.y << " " << root.offset.z << std::endl;
// 	return root.offset + recurse_joint(mesh, mesh.skeleton.joints[root.parentID]);
// }



// parent joint plus child joint to get  to world coordinates, but child of child joints needs to be set too


// start at root joint
// for every child of the root joint, call recurse joint

