
#include <stdlib.h>
#include "esUtil.h"

#define VERTEX_POS_SIZE 3
#define VERTEX_TEXCOORD_SIZE 2

#define VERTEX_POS_INDX 0
#define VERTEX_TEXCOORD_INDX 1

//#define VERTEX_STRIDE (sizeof(GLfloat)*(VERTEX_POS_SIZE+VERTEX_TEXCOORD_SIZE))
#define VERTEX_STRIDE 3
//detached
typedef struct
{
	GLuint programObject;
	GLint baseMapLoc;
	GLuint baseMapTexId;

	GLint modelMatrixLoc;
	GLint viewMatrixLoc;
	GLint projectMatrixLoc;

	GLint numOfPrimitivesLoc;
	GLint wingSizeLoc;

	ESMatrix model;
	ESMatrix view;
	ESMatrix project;

	GLuint vboIds[2];
	GLuint vaoId;
} UserData;

///
// Load texture from assets folder
//
GLuint LoadTexture(void *ioContext, char *fileName)
{
   int width,height;
   GLuint texId = esLoadPNG(ioContext, fileName, &width, &height);
   return texId;
}

///
// Initialize the shader and program object
//
int Init(ESContext *esContext)
{
	UserData *userData = esContext->userData;
	char vShaderStr[] =
		"#version 320 es\n"
		"#pragma optimize(off)\n"
		"#pragma debug(on)\n"
		"layout(location = 0) in vec4 position;\n"
		//"layout(location = 1) in vec2 texCoord;\n"
		// "uniform mat4 model;\n"
		// "uniform mat4 view;\n"
		// "uniform mat4 project;\n"
		// "out VertexData\n"\n"
		// "{\n"
		// "    smooth vec2 texCoord;\n"
		// "} vertexOut;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = position;\n"
		//"    vertexOut.texCoord = a_texCoord;\n"
		"}\n";

	char geometryShaderStr[] =
		"#version 320 es\n"
		"#pragma optimize(off)\n"
		"#pragma debug(on)\n"
		"layout(points) in;\n"
		"layout(triangle_strip,max_vertices = 8) out;\n"
		"uniform vec2 wingSize;\n"
		"uniform int numOfPrimitives;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		// "in VertexData
		// "{
		// "    smooth vec2 texCoord;
		// "} vertexIn[];
		"out interpolators\n"
		"{\n"
		"    smooth vec2 texCoord;\n"
		"} interpOut;\n"
		"void main()\n"
		"{\n"
		"  float alpha = radians(5.0f + (float(gl_PrimitiveIDIn)/(float(numOfPrimitives) - 1.0f))*80.0f);\n"
		"  float beta = radians(-90.0f + (float(gl_PrimitiveIDIn)/(float(numOfPrimitives) - 1.0f))*90.0f);\n"
		"  // Matrix to translate the wing to the origin\n"
		"  mat4 T = mat4(vec4(1,0,0,0),\n"
		"                vec4(0,1,0,0),\n"
		"                vec4(0,0,1,0),\n"
		"                vec4(-gl_in[0].gl_Position.xyz,1));\n"
		"  // Matrix to translate the wing back to its original position\n"
		"  mat4 Ti = mat4(vec4(1,0,0,0),\n"
		"                 vec4(0,1,0,0),\n"
		"                 vec4(0,0,1,0),\n"
		"                 vec4(gl_in[0].gl_Position.xyz,1));\n"
		"  //Matrix to rotate the whole butterfly to change its flying direction. This is a Z axis rotation matrix\n"
		"  mat4 Rz = mat4(vec4(cos(beta),sin(beta),0,0),\n"
		"                 vec4(-sin(beta),cos(beta),0,0),\n"
		"                 vec4(0,0,1,0),\n"
		"                 vec4(0,0,0,1));\n"
		"  // Left wing creation.\n"
		"  // Matrix to rotate the left wing and give an appearance of moving wings. This is a Y axis rotation matrix\n"
		"  mat4 Ry = mat4(vec4(cos(alpha),0,-sin(alpha),0),\n"
		"                 vec4(0,1,0,0),\n"
		"                 vec4(sin(alpha),0,cos(alpha),0),\n"
		"                 vec4(0,0,0,1));\n"
		"\n"
		"  mat4 M = projection * model*view * Rz * Ti * Ry * T;\n"
		"  // The angle for the left wing is alpha\n"
		"  // 1st vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y - wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  // Create a proper texture coordinate for this vertex\n"
		"  interpOut.texCoord = vec2(1.0f, 0.0f);\n"
		"  EmitVertex();\n"
		"  // 2nd vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y + wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(1.0f, 1.0f);\n"
		"  EmitVertex();\n"
		"  // 3rd vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x - wingSize.x,gl_in[0].gl_Position.y - wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(0.0f, 0.0f);\n"
		"  EmitVertex();\n"
		"  // 4th vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x - wingSize.x,gl_in[0].gl_Position.y + wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(0.0f, 1.0f);\n"
		"  EmitVertex();\n"
		"  EndPrimitive();\n"
		"\n"
		"  // The angle for the right wing is 180 - alpha\n"
		"  alpha = 3.141592f - alpha;\n"
		"  Ry = mat4(vec4(cos(alpha),0,-sin(alpha),0),\n"
		"            vec4(0,1,0,0),\n"
		"            vec4(sin(alpha),0,cos(alpha),0),\n"
		"            vec4(0,0,0,1));\n"
		"  M = projection * model*view * Rz * Ti * Ry * T;\n"
		"  // 1st vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x + wingSize.x,gl_in[0].gl_Position.y - wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(0.0f, 0.0f);\n"
		"  EmitVertex();\n"
		"  // 2nd vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x + wingSize.x,gl_in[0].gl_Position.y + wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(0.0f, 1.0f);\n"
		"  EmitVertex();\n"
		"  // 3rd vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y - wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(1.0f, 0.0f);\n"
		"  EmitVertex();\n"
		"  // 4th vertex\n"
		"  gl_Position = M * vec4(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y + wingSize.y / 2.0f, gl_in[0].gl_Position.zw);\n"
		"  interpOut.texCoord = vec2(1.0f, 1.0f);\n"
		"  EmitVertex();\n"
		"  EndPrimitive();\n"
		"}\n";

	char fShaderStr[] =
		"#version 320 es\n"
		"#pragma optimize(off)\n"
		"#pragma debug(on)\n"
		"precision mediump float;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"uniform sampler2D s_baseMap;\n"
		"in interpolators\n"
		"{\n"
		"    smooth vec2 texCoord;\n"
		"} interpIn;\n"
		"void main()\n"
		"{\n"
		"  outColor = texture(s_baseMap, interpIn.texCoord);\n"
		"}\n";

	userData->programObject = esLoadProgram_Vert_Geo_Frag(vShaderStr, geometryShaderStr, fShaderStr);
	userData->baseMapLoc = glGetUniformLocation(userData->programObject, "s_baseMap");
	userData->baseMapTexId = LoadTexture(esContext->platformData, "butterfly.png");
	userData->modelMatrixLoc = glGetUniformLocation(userData->programObject,"model");
	userData->viewMatrixLoc = glGetUniformLocation(userData->programObject,"view");
	userData->projectMatrixLoc = glGetUniformLocation(userData->programObject,"project");

	userData->numOfPrimitivesLoc = glGetUniformLocation(userData->programObject,"numOfPrimitives");
	userData->wingSizeLoc = glGetUniformLocation(userData->programObject,"wingSize");
	//esCheckGLError("GetUniformLocation");

	if (userData->baseMapTexId == 0)
	{
		return FALSE;
	}

	// GLfloat vertices[] = { -1.0f,  1.0f, 0.0f,   // Position 0
	// 						 0.0f,  0.0f,        // TexCoord 0
	// 						-1.0f, -1.0f, 0.0f,  // Position 1
	// 						 0.0f,  1.0f,        // TexCoord 1
	// 						 1.0f, -1.0f, 0.0f,  // Position 2
	// 						 1.0f,  1.0f,        // TexCoord 2
	// 						 1.0f,  1.0f, 0.0f,  // Position 3
	// 						 1.0f,  0.0f         // TexCoord 3
	// };
	// GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
	GLfloat vertices[] = { 0.0f,  0.0f, 0.0f,0.0f,  0.5f, 0.0f };   // Position 0

	// Generate VBO Ids and load the VBOs with data
	glGenBuffers ( 2, userData->vboIds );

	glBindBuffer (GL_ARRAY_BUFFER, userData->vboIds[0]);
	glBufferData (GL_ARRAY_BUFFER, sizeof(vertices),
				  vertices, GL_STATIC_DRAW);
	// glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, userData->vboIds[1]);
	// glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices),
	// 			  indices, GL_STATIC_DRAW);

	// Generate VAO Id
	glGenVertexArrays (1, &userData->vaoId);

	// Bind the VAO
	glBindVertexArray (userData->vaoId);

	glBindBuffer (GL_ARRAY_BUFFER, userData->vboIds[0]);
	//glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, userData->vboIds[1]);

	glEnableVertexAttribArray (VERTEX_POS_INDX);
	// glEnableVertexAttribArray (VERTEX_TEXCOORD_INDX);

	glVertexAttribPointer (VERTEX_POS_INDX, VERTEX_POS_SIZE,
						   GL_FLOAT, GL_FALSE, VERTEX_STRIDE, (const void *) NULL);

	// glVertexAttribPointer (VERTEX_TEXCOORD_INDX, VERTEX_TEXCOORD_SIZE,
	// 					   GL_FLOAT, GL_FALSE, VERTEX_STRIDE,
	// 					   (const void *)(VERTEX_POS_SIZE*sizeof(GLfloat)));

	// Reset to the default VAO
	glBindVertexArray (0);

	ESMatrix model,view,project;
	esMatrixLoadIdentity(&model);
	//esRotate(&model,30.0,1.0,0.0,0.0);
	esScale(&model,0.5f,0.5f,0.5f);
	userData->model = model;

	esMatrixLoadIdentity(&view);
	esMatrixLookAt(&view,0.0f,5.0f,-5.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f);
	userData->view = view;

	float aspect = (GLfloat)esContext->width/(GLfloat)esContext->height;
	esMatrixLoadIdentity(&project);
	esPerspective(&project,60.0f,aspect,0.05f,100.0f);
	userData->project = project;

	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	//blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	//depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	return TRUE;
}

void Draw(ESContext *esContext)
{
	UserData *userData = esContext->userData;

	glViewport(0, 0, esContext->width, esContext->height);

	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the program object
	glUseProgram(userData->programObject);

	// Bind the VAO
	glBindVertexArray(userData->vaoId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, userData->baseMapTexId);
	glUniform1i(userData->baseMapLoc, 0);

	glUniformMatrix4fv(userData->modelMatrixLoc,1,GL_FALSE,(GLfloat *)&(userData->model.m[0][0]));
	glUniformMatrix4fv(userData->viewMatrixLoc,1,GL_FALSE,(GLfloat *)&(userData->view.m[0][0]));
	glUniformMatrix4fv(userData->projectMatrixLoc,1,GL_FALSE,(GLfloat *)&(userData->project.m[0][0]));
	glUniform1i(userData->numOfPrimitivesLoc,5);
	glUniform2f(userData->wingSizeLoc,0.05f,0.1f);

	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void*)NULL);
	glDrawArrays(GL_POINTS, 0, 2);
	//Reset to the default VAO
	glBindVertexArray(0);
}

///
// Cleanup
//
void ShutDown(ESContext *esContext)
{
	UserData *userData = esContext->userData;
	glDeleteTextures(1, &userData->baseMapTexId);
	glDeleteProgram(userData->programObject);
	//unnecessary call
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}

int esMain(ESContext *esContext)
{
	esContext->userData = malloc(sizeof(UserData));

	esCreateWindow(esContext, "Butterfly", esContext->width, esContext->height, ES_WINDOW_RGB | ES_WINDOW_DEPTH |ES_WINDOW_ALPHA | ES_WINDOW_STENCIL|ES_WINDOW_MULTISAMPLE);

	if (!Init(esContext))
	{
		return GL_FALSE;
	}

	esRegisterDrawFunc(esContext, Draw);
	esRegisterShutdownFunc(esContext, ShutDown);

	return GL_TRUE;
}
