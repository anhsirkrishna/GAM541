#include "GLQuad.h"
#include "ShaderProgram.h"
#include "Transform.h"
#include "GameObject.h"

#include <SDL.h>
#include <GL\glew.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SDL_Log("OpenGL error (at line GLQuad.cpp:%d): %s\n", __LINE__, glewGetErrorString(err)); exit(-1);} }

/*Helper function to convert colors from 0-255 range to 0-1 range*/
void ConvertColor(std::vector<float>& colors) {
	unsigned int size = colors.size();
	for (unsigned int i = 0; i < size; i++) {
		colors[i] = colors[i] / 255.0;
	}
}

//Helper fuction to convert textures from pixel coords to 0-1 range
//Also swaps the coordinates for use
void ConvertTextureCoords(std::vector<float>& tex_coords, float tex_width, float tex_height) {
	unsigned int size = tex_coords.size();
	for (unsigned int i = 0; i < size; i += 2) {
		tex_coords[i] = tex_coords[i] / tex_width;
		tex_coords[i + 1] = tex_coords[i + 1] / tex_height;
	}
}

GLQuad::GLQuad() : Component("GLQuad"), p_texture(NULL), vao_id(0), p_owner_transform(NULL), texure_list_size(0) {
	tex_offset[0] = tex_offset[1] = 0;
	p_texure_list[0] = p_texure_list[1] = p_texure_list[2] = p_texure_list[3] = p_texure_list[4] = NULL;
}


void GLQuad::Draw(ShaderProgram* program) {
	GLuint loc = glGetUniformLocation(program->program_id, "translateMatrix");
	Matrix3D translate_matrix = p_owner_transform->GetTranslateMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, translate_matrix.GetMatrixP());
	CHECKERROR;

	loc = glGetUniformLocation(program->program_id, "rotateMatrix");
	Matrix3D rotate_matrix = p_owner_transform->GetRotateMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, rotate_matrix.GetMatrixP());
	CHECKERROR;

	loc = glGetUniformLocation(program->program_id, "scaleMatrix");
	Matrix3D scale_matrix = p_owner_transform->GetScaleMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, scale_matrix.GetMatrixP());
	CHECKERROR;

	loc = glGetUniformLocation(program->program_id, "preRotateMatrix");
	Matrix3D pre_rotate_matrix = p_owner_transform->GetPreRotateMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, pre_rotate_matrix.GetMatrixP());
	CHECKERROR;

	loc = glGetUniformLocation(program->program_id, "postRotateMatrix");
	Matrix3D post_rotate_matrix = p_owner_transform->GetPostRotateMatrix();
	glUniformMatrix4fv(loc, 1, GL_FALSE, post_rotate_matrix.GetMatrixP());
	CHECKERROR;

	loc = glGetUniformLocation(program->program_id, "tex_offset");
	glUniform2fv(loc, 1, &(tex_offset[0]));
	CHECKERROR;

	glBindVertexArray(vao_id);
	CHECKERROR;
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
}

void GLQuad::CreateDemo() {
	
	//Create a VAO and put the ID in vao_id
	glGenVertexArrays(1, &vao_id);
	//Use the same VAO for all the following operations
	glBindVertexArray(vao_id);

	//Put a vertex consisting of 3 float coordinates x,y,z into the list of all vertices
	std::vector<float> vertices = {
		0,  0, 0,
		0, 48, 0,
		24, 48, 0,
		24,  0, 0
	};


	//Create a continguous buffer for all the vertices/points
	GLuint point_buffer;
	glGenBuffers(1, &point_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, point_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;

	//Put a color consisting of 4 float values rgba into the list of all colors 
	std::vector<float> colors = {
		  0, 255,  0, 255,
		 50, 255, 50, 255,
		  0,   0,  0, 255,
		 50, 255, 50, 255
	};
	//Convert colors from 0-255 range to 0-1 range
	ConvertColor(colors);
	//Create another continuguous buffer for all the colors for each vertex
	GLuint color_buffer;
	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), &colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;
	//Put a texture coordinate cosisting of 2 uv float values 
	std::vector<float> coord = {
		 0,  0,
		 0, 32,
		 15, 32,
		 15,  0
	};
	//Create another continguous buffer for all the textures for each vertex
	GLuint tex_coord_buffer;
	glGenBuffers(1, &tex_coord_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, tex_coord_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coord.size(), &coord[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECKERROR;
	//IBO data
	GLuint indexData[] = { 0, 1, 2, 3 };
	//Create IBO
	GLuint indeces_buffer;
	glGenBuffers(1, &indeces_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeces_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), indexData, GL_STATIC_DRAW);
	CHECKERROR;
	glBindVertexArray(0);
}

void GLQuad::Link() {
	p_owner_transform = static_cast<Transform*>(GetOwner()->HasComponent("TRANSFORM"));
}

void GLQuad::SetTexOffset(GLfloat tex_offset_x, GLfloat tex_offset_y) {
	tex_offset[0] = tex_offset_x;
	tex_offset[1] = tex_offset_y;
}