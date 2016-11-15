#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
// FIXME: Add functions to generate the bone mesh.
void create_bones(struct Mesh& mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
//glm::vec3 recurse_joint(struct Mesh mesh, struct Joint root);
void recurse_joint_t(struct Mesh& mesh, glm::mat4 transformationSoFar, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
bool RayIntersectCylinder(const struct Mesh& mesh, const int boneIndex, const glm::vec3& origin, const glm::vec3& direction, float radius, float height, float* t);
int intersects(const struct Mesh& mesh, const glm::vec3& origin, const glm::vec3& direction);
void create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces);
#endif
