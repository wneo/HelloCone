//
//  RenderingEngine1.cpp
//  HelloArrow
//
//  Created by neo on 13-8-31.
//  Copyright (c) 2013年 neo. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <cmath>
#include <iostream>
#include "IRenderingEngine.hpp"


#define STRINGIFY(A) #A
#include "../shaders/Simple.vert"
#include "../shaders/Simple.frag"


static const float RevolutionPerSecond = 1;

class RenderingEngine2 : public IRenderingEngine {
private:
    GLuint m_frameBuffer;
    GLuint m_renderBuffer;
	float m_currentAngle;
	float m_desiredAngle;
	
	GLuint m_simpleProgram;
    
	float RotationDirection() const;
	GLuint BuildShader(const char* source, GLenum shaderType) const;
	GLuint BuildProgram(const char* vShader, const char* fShader) const;
	void ApplyOrtho(float maxX, float maxY) const;
	void ApplyRotation(float degrees) const;
	
public:
    RenderingEngine2();
    void Initalize(int width, int height);
    void Render() const;
    void UpdateAnimation(float timeStep);
    void OnRotate(DeviceOrientation newOrientation);
};

IRenderingEngine *CreateRenderer2()
{
    return new RenderingEngine2();
}

//顶点
struct Vertex {
    float Position[2];
    float Color[4];
};

const Vertex Vertices[] = {
    {{-0.5, -0.866}, {1, 1, 0.5f, 1}},
    {{ 0.5, -0.866}, {1, 1, 0.5f, 1}},
    {{0, 1},         {1, 1, 0.5f, 1}},
    {{-0.5, -0.866}, {0.5f, 0.5f, 0.5f}},
    {{ 0.5, -0.866}, {0.5f, 0.5f, 0.5f}},
    {{0, -0.4f},	 {0.5f, 0.5f, 0.5f}},
};

RenderingEngine2::RenderingEngine2()
{
	// Create & bind the color buffer so that the caller can allocate its space.
    glGenRenderbuffers(1, &m_renderBuffer);//创建render（渲染）缓冲区
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);//绑定管线
}
void RenderingEngine2::Initalize(int width, int height)
{
    //create the frameBuffer object and attach the color buffer
    glGenFramebuffers(1, &m_frameBuffer);//创建frame(帧)缓冲区
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);//绑定管线
    
    //绑定 frame 和 render
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 m_renderBuffer);
    //构建坐标系统
    glViewport(0, 0, width, height);
	
	m_simpleProgram = BuildProgram(SimpleVertexShader, SimpleFragmentShader);
	
	glUseProgram(m_simpleProgram);
    
    //initialize the projection matrix
    ApplyOrtho(2, 3);
	
    //glMatrixMode(GL_MODELVIEW);
	
	// Initialize the rotation animation state.
	OnRotate(DeviceOrientationPortrait);
	m_currentAngle = m_desiredAngle;
}

void RenderingEngine2::ApplyOrtho(float maxX, float maxY) const
{
    float a = 1.0f / maxX;
    float b = 1.0f / maxY;
    float ortho[16] = {
        a, 0,  0, 0,
        0, b,  0, 0,
        0, 0, -1, 0,
        0, 0,  0, 1
    };
    
    GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho[0]);
}

void RenderingEngine2::Render() const
{
    //利用灰色清除缓冲区
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
	
	
//	glPushMatrix();
//	glRotatef(m_currentAngle, 0, 0, 1);
	ApplyRotation(m_currentAngle);
	
	GLuint positionSlot = glGetAttribLocation(m_simpleProgram, "Position");
	GLuint colorSlot = glGetAttribLocation(m_simpleProgram, "SourceColor");
		
    //设置顶点属性
//    glEnableClientState(GL_VERTEX_ARRAY);//位置
//    glEnableClientState(GL_COLOR_ARRAY);//颜色
	glEnableVertexAttribArray(positionSlot);
	glEnableVertexAttribArray(colorSlot);
	
	
    //设置数据格式
//    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &Vertices[0].Position[0]);
//    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &Vertices[0].Color[0]);
	GLsizei stride = sizeof(Vertex);
	const GLvoid* pCoords = &Vertices[0].Position[0];
	const GLvoid* pColors = &Vertices[0].Color[0];
	glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
	glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);
	
    //以顶点属性绘制三角形
    GLsizei vertexCount = sizeof(Vertices) / sizeof(Vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);//从0起点开始绘制三角形
    //关闭顶点属性 (后续不会继续使用时关闭)
//    glDisableClientState(GL_VERTEX_ARRAY);
//    glDisableClientState(GL_COLOR_ARRAY);
	glDisableVertexAttribArray(positionSlot);
	glDisableVertexAttribArray(colorSlot);
	
	//glPopMatrix();
}

float RenderingEngine2::RotationDirection() const
{
	float delta = m_desiredAngle - m_currentAngle;
	if (delta == 0)
		return 0;
	bool counterclockwise = ((delta > 0 && delta <= 180) || (delta < -180));
	return counterclockwise ? +1 : -1;
}

void RenderingEngine2::UpdateAnimation(float timeStep)
{
    float direction = RotationDirection();
	if (direction == 0)
		return;
	float degrees = timeStep * 360 * RevolutionPerSecond;
	m_currentAngle += degrees * direction;
	// Ensure that the angle stays within [0, 360).
	if (m_currentAngle >= 360)
		m_currentAngle -= 360;
	else if (m_currentAngle < 0)
		m_currentAngle += 360;
	// If the rotation direction changed, then we overshot the desired angle.
	if (RotationDirection() != direction)
		m_currentAngle = m_desiredAngle;
}

void RenderingEngine2::OnRotate(DeviceOrientation newOrientation)
{
    float angle = 0;
	switch (newOrientation) {
		case DeviceOrientationLandscapeLeft:
			angle = 270;
			break;
		case DeviceOrientationPortraitUpsideDown:
			angle = 180;
			break;
		case DeviceOrientationLandscapeRight:
			angle = 90;
			break;
		default:
			break;
			
	}
	m_desiredAngle = angle;
}

void RenderingEngine2::ApplyRotation(float degrees) const
{
    float radians = degrees * 3.14159f / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);
    float zRotation[16] = {
        c, s, 0, 0,
		-s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    GLint modelviewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &zRotation[0]);
}

GLuint RenderingEngine2::BuildShader(const char* source, GLenum shaderType) const
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return shaderHandle;
}

GLuint RenderingEngine2::BuildProgram(const char* vertexShaderSource,
                                      const char* fragmentShaderSource) const
{
    GLuint vertexShader = BuildShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    
    return programHandle;
}

