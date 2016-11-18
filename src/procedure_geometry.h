#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
// FIXME: Add functions to generate the bone mesh.
void create_bones(struct Mesh& mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
//glm::vec3 recurse_joint(struct Mesh mesh, struct Joint root);
void recurse_joint_t(struct Mesh& mesh, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
bool RayIntersectCylinder(const struct Mesh& mesh, const int boneIndex, const glm::vec3& origin, const glm::vec3& direction, float radius, float height, float* t);
int intersects(const struct Mesh& mesh, const glm::vec3& origin, const glm::vec3& direction);
void create_cylinder(std::vector<glm::vec4>& cylinder_vertices, std::vector<glm::uvec2>& cylinder_faces,
					 std::vector<glm::vec4>& norm_vertices, std::vector<glm::uvec2>& norm_faces,
					 std::vector<glm::vec4>& binorm_vertices, std::vector<glm::uvec2>& binorm_faces);
void redraw_skeleton(struct Mesh& mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);

void redraw_skel(struct Mesh& mesh, struct Joint parent, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
void update_child_transformations(struct Mesh& mesh, int current_bone); // Joint parent);
#endif
