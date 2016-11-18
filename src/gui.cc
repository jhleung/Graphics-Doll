#include "gui.h"
#include "config.h"
#include <jpegio.h>
#include "bone_geometry.h"
#include "procedure_geometry.h"
#include "jpegio.cc"

#include <iostream>
#include <debuggl.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

namespace {
	// Intersect a cylinder with radius 1/2, height 1, with base centered at
	// (0, 0, 0) and up direction (0, 1, 0).
	bool IntersectCylinder(const glm::vec3& origin, const glm::vec3& direction,
			float radius, float height, float* t)
	{
		//FIXME perform proper ray-cylinder collision detection
		return true;
	}
}

GUI::GUI(GLFWwindow* window)
	:window_(window)
{
	glfwSetWindowUserPointer(window_, this);
	glfwSetKeyCallback(window_, KeyCallback);
	glfwSetCursorPosCallback(window_, MousePosCallback);
	glfwSetMouseButtonCallback(window_, MouseButtonCallback);

	glfwGetWindowSize(window_, &window_width_, &window_height_);
	float aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ = glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
}

GUI::~GUI()
{
}

void GUI::assignMesh(Mesh* mesh)
{
	mesh_ = mesh;
	center_ = mesh_->getCenter();
}

void GUI::keyCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window_, GL_TRUE);
		return ;
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		//FIXME save out a screenshot using SaveJPEG
	}

	if (captureWASDUPDOWN(key, action))
		return ;
	if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
		float roll_speed;
		if (key == GLFW_KEY_RIGHT)
			roll_speed = -roll_speed_;
		else
			roll_speed = roll_speed_;
		// FIXME: actually roll the bone here
	} else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
		fps_mode_ = !fps_mode_;
	} else if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_RELEASE) {
		current_bone_--;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
	} else if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_RELEASE) {
		current_bone_++;
		current_bone_ += mesh_->getNumberOfBones();
		current_bone_ %= mesh_->getNumberOfBones();
	} else if (key == GLFW_KEY_T && action != GLFW_RELEASE) {
		transparent_ = !transparent_;
	}
	else if (key == GLFW_KEY_J && action != GLFW_RELEASE) {
		unsigned char*  data = new unsigned char[window_width_ * window_height_ * 3];
		glReadPixels(0, 0, window_width_, window_height_, GL_RGB, GL_UNSIGNED_BYTE, data);
		SaveJPEG("../Screenshot.jpg", window_width_, window_height_, data);
		delete [] data;
	}
}

void GUI::mousePosCallback(double mouse_x, double mouse_y)
{
	last_x_ = current_x_;
	last_y_ = current_y_;
	current_x_ = mouse_x;
	current_y_ = window_height_ - mouse_y;
	float delta_x = current_x_ - last_x_;
	float delta_y = current_y_ - last_y_;
	if (sqrt(delta_x * delta_x + delta_y * delta_y) < 1e-15)
		return;
	glm::vec3 mouse_direction = glm::normalize(glm::vec3(delta_x, delta_y, 0.0f));
	glm::vec2 mouse_start = glm::vec2(last_x_, last_y_);
	glm::vec2 mouse_end = glm::vec2(current_x_, current_y_);
	glm::uvec4 viewport = glm::uvec4(0, 0, window_width_, window_height_);

	bool drag_camera = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_RIGHT;
	bool drag_bone = drag_state_ && current_button_ == GLFW_MOUSE_BUTTON_LEFT;

	if (drag_camera) {
		glm::vec3 axis = glm::normalize(
				orientation_ *
				glm::vec3(mouse_direction.y, -mouse_direction.x, 0.0f)
				);
		orientation_ =
			glm::mat3(glm::rotate(rotation_speed_, axis) * glm::mat4(orientation_));
		tangent_ = glm::column(orientation_, 0);
		up_ = glm::column(orientation_, 1);
		look_ = glm::column(orientation_, 2);
	} else if (drag_bone && current_bone_ != -1) {
		// FIXME: Handle bone rotation
		float dx = (float) current_x_ - last_x_;
		float dy = (float) current_y_ - last_y_;
		glm::vec2 norm_delt = glm::normalize(glm::vec2(dx, dy));
		glm::vec3 axis = 1.0f * norm_delt.y * tangent_ + norm_delt.x * up_;

		Bone& currBone = mesh_->skeleton.bones[current_bone_];
		// get parent's accumulated matrix and use it to update child's accumulated
		// so parent*translation*new rotation

		glm::mat4 rotate_matrix = glm::rotate(rotation_speed_, axis) * currBone.rotation;
		// currBone.rotation = rotate_matrix;

		Joint start = mesh_->skeleton.joints[currBone.jointStart];
		Bone parentBone = mesh_->skeleton.bones[start.inBone];

		currBone.transformation = currBone.transformation * glm::inverse(currBone.rotation) * rotate_matrix;
		currBone.rotation = rotate_matrix;

//		currBone.transformation = parentBone.transformation * currBone.translation * rotate_matrix;

		update_child_transformations(*mesh_, current_bone_);

		// Joint end = mesh_->skeleton.joints[currBone.jointEnd];
		// glm::vec4 currBoneWC = currBone.transformation * glm::vec4(0.0f, 0.0f, currBone.length, 1.0f);
		// for (int i = 0; i < end.outBones.size(); i++) {	
		// 	Bone& currChildBone = mesh_->skeleton.bones[end.outBones[i]];

		// 	glm::vec4 translationCoords = glm::inverse(currBone.transformation) * glm::vec4(glm::vec3(currBoneWC), 1.0f);
		// 	currChildBone.translation = glm::transpose(glm::mat4(1.0f, 0.0f, 0.0f, translationCoords[0],
		// 												0.0f, 1.0f, 0.0f, translationCoords[1],
		// 												0.0f, 0.0f, 1.0f, translationCoords[2],
		// 												0.0f, 0.0f, 0.0f, 1.0f));

		// 	glm::mat4 transformationSoFar = currBone.transformation * currChildBone.translation;

		// 	Joint childEnd = mesh_->skeleton.joints[currChildBone.jointEnd];
		// 	glm::vec4 rotationCoords = (glm::inverse(transformationSoFar) * glm::vec4(glm::vec3(currBoneWC) + childEnd.offset, 1.0f)) / currChildBone.length;

		// 	glm::vec3 t = glm::normalize(glm::vec3(rotationCoords));
		// 	glm::vec3 v;
		// 	float min = std::min(std::min(std::abs(t[0]),std::abs(t[1])),std::abs(t[2]));
		// 	if (std::abs(t[0]) == min) {
		// 		v = glm::vec3(1.0f, 0.0f, 0.0f);
		// 	} else if (std::abs(t[1]) == min) {
		// 		v = glm::vec3(0.0f, 1.0f, 0.0f);
		// 	} else if (std::abs(t[2]) == min) {
		// 		v = glm::vec3(0.0f, 0.0f, 1.0f);
		// 	}
		// 	glm::vec3 n = glm::cross(t, v) / glm::length(glm::cross(t, v)); // normalizing t x v is slightly different
		// 	glm::vec3 b = glm::cross(t, n);

		// 	currChildBone.rotation = glm::mat4(n.x, n.y, n.z, 0.0f,
		// 								  b.x, b.y, b.z, 0.0f,
		// 								  t.x, t.y, t.z, 0.0f,
		// 								  0.0f, 0.0f, 0.0f, 1.0f);
		// 	currChildBone.transformation = transformationSoFar * currChildBone.rotation;


		// 	//currChildBone.transformation = currBone.transformation * currChildBone.translation * currChildBone.rotation;
		// }

		pose_changed_ = true;
		// glm::mat4 deformed = glm::mat4(t,u,l, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		// return ;
	}
	// FIXME: highlight bones that have been moused over
	glm::vec3 screenStartPos = glm::vec3(current_x_, current_y_, 0.0f);
	glm::vec3 screenEndPos = glm::vec3(current_x_, current_y_, 1.0f);

	glm::vec3 worldCoordsStart = glm::unProject(screenStartPos, view_matrix_, projection_matrix_, viewport);
	glm::vec3 worldCoordsEnd = glm::unProject(screenEndPos, view_matrix_, projection_matrix_, viewport);

	current_bone_ = intersects(*(mesh_), eye_, glm::normalize(worldCoordsEnd - worldCoordsStart));
	// std::cout << "curr bone: " << current_bone_ << std::endl;
}

void GUI::mouseButtonCallback(int button, int action, int mods)
{
	drag_state_ = (action == GLFW_PRESS);
	current_button_ = button;
}

void GUI::updateMatrices()
{
	// Compute our view, and projection matrices.
	if (fps_mode_)
		center_ = eye_ + camera_distance_ * look_;
	else
		eye_ = center_ - camera_distance_ * look_;

	view_matrix_ = glm::lookAt(eye_, center_, up_);
	light_position_ = glm::vec4(eye_, 1.0f);

	aspect_ = static_cast<float>(window_width_) / window_height_;
	projection_matrix_ =
		glm::perspective((float)(kFov * (M_PI / 180.0f)), aspect_, kNear, kFar);
	model_matrix_ = glm::mat4(1.0f);
}

MatrixPointers GUI::getMatrixPointers() const
{
	MatrixPointers ret;
	ret.projection = &projection_matrix_[0][0];
	ret.model= &model_matrix_[0][0];
	ret.view = &view_matrix_[0][0];
	return ret;
}

bool GUI::setCurrentBone(int i)
{
	if (i < 0 || i >= mesh_->getNumberOfBones())
		return false;
	current_bone_ = i;
	return true;
}

bool GUI::captureWASDUPDOWN(int key, int action)
{
	if (key == GLFW_KEY_W) {
		if (fps_mode_)
			eye_ += zoom_speed_ * look_;
		else
			camera_distance_ -= zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_S) {
		if (fps_mode_)
			eye_ -= zoom_speed_ * look_;
		else
			camera_distance_ += zoom_speed_;
		return true;
	} else if (key == GLFW_KEY_A) {
		if (fps_mode_)
			eye_ -= pan_speed_ * tangent_;
		else
			center_ -= pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_D) {
		if (fps_mode_)
			eye_ += pan_speed_ * tangent_;
		else
			center_ += pan_speed_ * tangent_;
		return true;
	} else if (key == GLFW_KEY_DOWN) {
		if (fps_mode_)
			eye_ -= pan_speed_ * up_;
		else
			center_ -= pan_speed_ * up_;
		return true;
	} else if (key == GLFW_KEY_UP) {
		if (fps_mode_)
			eye_ += pan_speed_ * up_;
		else
			center_ += pan_speed_ * up_;
		return true;
	}
	return false;
}


// Delegrate to the actual GUI object.
void GUI::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->keyCallback(key, scancode, action, mods);
}

void GUI::MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mousePosCallback(mouse_x, mouse_y);
}

void GUI::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	GUI* gui = (GUI*)glfwGetWindowUserPointer(window);
	gui->mouseButtonCallback(button, action, mods);
}
