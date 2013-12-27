#version 330

//---global uniforms
layout(std140) uniform GlobalMatrices
{
	mat4 cameraToClipMatrix;
	mat4 worldToCameraMatrix;
	mat4 uiProjection;
};

//---shader spec
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 color;
smooth out vec2 inTexCoord;
smooth out vec4 inColor;
void main()
{
	inTexCoord=texcoord;
	inColor=color;
	gl_Position = uiProjection*vec4(position,0,1);
}
