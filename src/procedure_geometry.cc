#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"
#include <glm/ext.hpp>
#include <iostream>
#include <math.h>

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces) {
	for (float i = 0; i < 10; i++) {
		float y = i/10.0f;
		//float theta = 2*M_PI*y;
		glm::vec4 start = glm::vec4(0.0f, y, 0.0f, 1.0f);
		glm::vec4 end = glm::vec4(0.0f, y, 1.0f, 1.0f);
		// glm::vec4 start = glm::vec4(std::cos(theta), std::sin(theta), 0.0f, 1.0f);
		// glm::vec4 end = glm::vec4(std::cos(theta), std::sin(theta), endTemp[2], 1.0f);

		cylinder_vertices.push_back(start);
		cylinder_vertices.push_back(end);
		cylinder_faces.push_back(glm::uvec2(cylinder_vertices.size()-2, cylinder_vertices.size()-1));
	}
}

int intersects(const struct Mesh &mesh, const glm::vec3& origin, const glm::vec3& direction) {

	for (int i = 0; i < mesh.skeleton.bones.size(); i++) {
		float t = 0.0;
		const Bone& currBone = mesh.skeleton.bones[i];
		if (RayIntersectCylinder(mesh, i, origin, direction, kCylinderRadius, currBone.length, &t)) {
			return i;
		}
	}
	return -1; // found no intersections
}

// infinite number of circles -> cylinder
// see if ray intersects, translate world_coords to bone coords
// solve equation of circle for t1,t2 using quadratic equation
// see if any t1, t2 satisfies the range -> return true
// if both satisfy, pick the smaller of the two
bool RayIntersectCylinder(const struct Mesh& mesh, const int boneIndex, const glm::vec3& origin, const glm::vec3& direction, float radius, float height, float* t) {	
	const Bone currBone = mesh.skeleton.bones[boneIndex];
	glm::vec3 alignedOrigin = glm::vec3(glm::inverse(currBone.transformation) * glm::vec4(origin, 1.0f)); // along z axis
	glm::vec3 alignedDirection = glm::vec3(glm::inverse(currBone.transformation) * glm::vec4(direction, 0.0f));

	float x_p = alignedOrigin.x;
	float x_d = alignedDirection.x;
	float y_p = alignedOrigin.y;
	float y_d = alignedDirection.y;

	float a = y_d*y_d + x_d*x_d;
	float b = 2 * y_d * y_p + 2* x_d * x_p;
	float c = x_p*x_p + y_p*y_p - radius*radius;

	if ((b*b - 4*a*c) < 0) // determinant
		return false;

	float t1 = (-b + sqrt(b*b - 4*a*c)) / (2*a);
	float t2 = (-b - sqrt(b*b - 4*a*c)) / (2*a);

	float z1 = alignedOrigin.z + t1*alignedDirection.z;
	float z2 = alignedOrigin.z + t2*alignedDirection.z;
	bool t1InRange = z1 >= 0 && z1 <= height;
	bool t2InRange = z2 >= 0 && z2 <= height;

	// if (t1InRange && t2InRange) {

	// 	// t = std::min(t1,t2);
	// 	if (t1 > t2)
	// 		t = &t2;
	// 	else t = &t1;
	// }

	return t1InRange || t2InRange;
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
	// for (int i = ; i < 2; i++) {
	// std::cout << "bones size: " << skeleton.bones.size() << std::endl;
		// std::cout << "translation: " << glm::to_string(mesh.skeleton.bones[2].translation) << std::endl;
		// std::cout << "rotation: " << glm::to_string(mesh.skeleton.bones[2].rotation) << std::endl;
	// }
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

		currBone.transformation = transformationSoFar;

		bone_vertices.push_back(vertStart);
		bone_vertices.push_back(vertEnd);

		bone_faces.push_back(glm::uvec2(bone_vertices.size()-2, bone_vertices.size()-1));

		recurse_joint_t(mesh, transformationSoFar, currJoint, jointEndCoords, bone_vertices, bone_faces);
	}
}