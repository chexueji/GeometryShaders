
#include <stdlib.h>
#include "esUtil.h"

#define VERTEX_POS_SIZE 4

#define VERTEX_POS_INDX 0

#define VERTEX_STRIDE 4

typedef struct
{
	GLuint programObject;

	GLuint vboId;
	GLuint vaoId;
} UserData;

///
// Initialize the shader and program object
//
int Init(ESContext *esContext)
{
	UserData *userData = esContext->userData;
	char vShaderStr[] =
		"#version 320 es\n"
		//"#pragma optimize(off)\n"
		//"#pragma debug(on)\n"
		"layout(location = 0) in vec4 position;\n"
		"out vec3 outColor;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = position;\n"
		"   gl_PointSize = 60.0f;\n"
		"}\n";

	char geometryShaderStr_Line[] =
		"#version 320 es\n"
		//"#pragma optimize(off)\n"
		//"#pragma debug(on)\n"
		"layout(points) in;\n"
		"layout(line_strip,max_vertices = 2) out;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = gl_in[0].gl_Position + vec4(-0.8, 0.0, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  gl_Position = gl_in[0].gl_Position + vec4( 0.8, 0.0, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  EndPrimitive();\n"
		"}\n";
	char geometryShaderStr_House[] =
		"#version 320 es\n"
		//"#pragma optimize(off)\n"
		//"#pragma debug(on)\n"
		"layout(points) in;\n"
		"layout(triangle_strip,max_vertices = 5) out;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = gl_in[0].gl_Position + vec4(-0.4, -0.4, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  gl_Position = gl_in[0].gl_Position + vec4( 0.4, -0.4, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  gl_Position = gl_in[0].gl_Position + vec4(-0.4, 0.4, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  gl_Position = gl_in[0].gl_Position + vec4( 0.4, 0.4, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  gl_Position = gl_in[0].gl_Position + vec4( 0.0, 0.8, 0.0, 0.0);\n"
		"  EmitVertex();\n"
		"  EndPrimitive();\n"
		"}\n";

	char fShaderStr[] =
		"#version 320 es\n"
		//"#pragma optimize(off)\n"
		//"#pragma debug(on)\n"
		"precision highp float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"  outColor = vec4(0.0f,0.0f,1.0f,1.0);\n"
		"}\n";

	userData->programObject = esLoadProgram_Vert_Geo_Frag(vShaderStr, geometryShaderStr_House, fShaderStr);

	GLfloat vertices[] = { 0.0f, 0.0f, 0.0f, 1.0f};   // Position


	// Generate VBO Ids and load the VBOs with data
	glGenBuffers ( 1, &userData->vboId );

	glBindBuffer (GL_ARRAY_BUFFER, userData->vboId);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertices),
				  vertices, GL_STATIC_DRAW);
	// Generate VAO Id
	glGenVertexArrays (1, &userData->vaoId);

	// Bind the VAO
	glBindVertexArray (userData->vaoId);

	glBindBuffer (GL_ARRAY_BUFFER, userData->vboId);

	glEnableVertexAttribArray (VERTEX_POS_INDX);

	glVertexAttribPointer (VERTEX_POS_INDX, VERTEX_POS_SIZE,
						   GL_FLOAT, GL_FALSE, VERTEX_STRIDE, (const void *) NULL);
	// Reset to the default VAO
	glBindVertexArray (0);

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	// donot use any fragment shader
	// glEnable(GL_RASTERIZER_DISCARD);

	return TRUE;
}

void Draw(ESContext *esContext)
{
	UserData *userData = esContext->userData;

	glViewport(0, 0, esContext->width, esContext->height);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);
	// Use the program object
	glUseProgram(userData->programObject);
	// Bind the VAO
	glBindVertexArray(userData->vaoId);

	glDrawArrays(GL_POINTS, 0, 1);
	//Reset to the default VAO
	glBindVertexArray(0);
}

///
// Cleanup
//
void ShutDown(ESContext *esContext)
{
	UserData *userData = esContext->userData;
	glDeleteProgram(userData->programObject);
	// glDisable(GL_RASTERIZER_DISCARD);
}

int esMain(ESContext *esContext)
{
	esContext->userData = malloc(sizeof(UserData));

	esCreateWindow(esContext, "Point2Line", esContext->width, esContext->height, ES_WINDOW_RGB);

	if (!Init(esContext))
	{
		return GL_FALSE;
	}

	esRegisterDrawFunc(esContext, Draw);
	esRegisterShutdownFunc(esContext, ShutDown);

	return GL_TRUE;
}
