#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);
// FIXME: Add functions to generate the bone mesh.
void create_bones(struct Mesh mesh, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
//glm::vec3 recurse_joint(struct Mesh mesh, struct Joint root);
void recurse_joint(struct Mesh mesh, struct Joint parent, glm::vec3 offsetSoFar, std::vector<glm::vec4>& bone_vertices, std::vector<glm::uvec2>& bone_faces);
#endif
