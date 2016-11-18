R"zzz(#version 330 core
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 transformation;
uniform float bone_length;
uniform float cyl_radius;
in vec4 vertex_position;
void main()
{
	float theta = 2 * 3.14159 * vertex_position.y;
	vec4 vertex_pos_temp = vec4(cos(theta) * cyl_radius, sin(theta) * cyl_radius, vertex_position.z * bone_length, 1.0f);
	gl_Position = projection * view * transformation * model * vertex_pos_temp;
}
)zzz"