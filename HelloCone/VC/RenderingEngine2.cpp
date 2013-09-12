//
//  RenderingEngine1.cpp
//  HelloArrow
//
//  Created by neo on 13-8-31.
//  Copyright (c) 2013年 neo. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <iostream>
#include "IRenderingEngine.hpp"
#include "Quaternion.hpp"
#include <vector>

#define STRINGIFY(A) #A
#include "../shaders/Simple.vert"
#include "../shaders/Simple.frag"


static const float AnimationDuration = 0.25f;

using namespace std;

//顶点
struct Vertex {
    vec3 Position;
    vec4 Color;
};

struct Animation {
	Quaternion Start;
	Quaternion End;
	Quaternion Current;
	float Elapsed;//已用时
	float Duration;//持续时间
};

class RenderingEngine2 : public IRenderingEngine {
private:
    GLuint m_frameBuffer;
    GLuint m_colorRrenderBuffer;
	GLuint m_depthRenderBuffer;
	float m_currentAngle;
	float m_desiredAngle;
	
	GLuint m_simpleProgram;
	Animation m_animation;
	
	vector<Vertex> m_cone;//锥体侧面顶点集
	vector<Vertex> m_disk;//锥体底面顶点集
    
	GLuint BuildShader(const char* source, GLenum shaderType) const;
	GLuint BuildProgram(const char* vShader, const char* fShader) const;

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


RenderingEngine2::RenderingEngine2()
{
	// Create & bind the color buffer so that the caller can allocate its space.
    glGenRenderbuffers(1, &m_colorRrenderBuffer);//创建render（渲染）缓冲区
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRrenderBuffer);//绑定管线
}
void RenderingEngine2::Initalize(int width, int height)
{
	// ---- 生成图元顶点 ----//
	//图元参数定义
	const float coneRadius = 0.9f;
	const float coneHeight = 1.866f;
	const int coneSlices = 40;//碎片数 (图元) -- 采用 GL_TRIANGLE_STRIP 模式绘制
	
	//锥体侧面顶点
	{
		// Allocate space for the cone vertices.
		m_cone.resize((coneSlices + 1) * 2);
		
		// Initialize the vertices of the triangle strip.
		vector<Vertex>::iterator vertex = m_cone.begin();
		const float dtheta = TwoPi / coneSlices;
		for (float theta = 0; vertex != m_cone.end(); theta += dtheta) {
			
			// Grayscale gradient  梯形灰阶
			float brightness = abs(sin(theta));
			vec4 color(brightness, brightness, brightness, 1);
			
			// Apex vertex  尖端顶点
			vertex->Position = vec3(0, 1, 0);
			vertex->Color = color;
			vertex ++;
			
			// Rim vertex  底盘顶点
			vertex->Position.x = coneRadius * cos(theta);
			vertex->Position.y = 1 - coneHeight;
			vertex->Position.z = coneRadius * sin(theta);
			vertex->Color = color;
			vertex ++;
		}
	}
	
	//锥体底面顶点
	{
		// Allocate space for the disk vertices.
		m_disk.resize(coneSlices + 2);
		
		// Initialize the center vertex of the triangle fan.  圆心
        vector<Vertex>::iterator vertex = m_disk.begin();
        vertex->Color = vec4(0.75, 0.75, 0.75, 1);
        vertex->Position.x = 0;
        vertex->Position.y = 1 - coneHeight;
        vertex->Position.z = 0;
        vertex++;
		
		// Initialize the rim vertices of the triangle fan.
        const float dtheta = TwoPi / coneSlices;
        for (float theta = 0; vertex != m_disk.end(); theta += dtheta) {
            vertex->Color = vec4(0.75, 0.75, 0.75, 1);
            vertex->Position.x = coneRadius * cos(theta);
            vertex->Position.y = 1 - coneHeight;
            vertex->Position.z = coneRadius * sin(theta);
            vertex++;
        }
	}
	
	// Create the depth buffer.
	glGenRenderbuffers(1, &m_depthRenderBuffer);//创建render（渲染）缓冲区
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);//绑定管线
	glRenderbufferStorage(GL_RENDERBUFFER,
						  GL_DEPTH_COMPONENT16,
						  width,
						  height);
	
	
    //create the frameBuffer object and attach the depth  and the color buffer
    glGenFramebuffers(1, &m_frameBuffer);//创建frame(帧)缓冲区
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);//绑定管线
    //attach
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
							  GL_COLOR_ATTACHMENT0,
							  GL_RENDERBUFFER,
							  m_colorRrenderBuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
							  GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER,
							  m_depthRenderBuffer);
	
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRrenderBuffer);
	
    //构建坐标系统
    glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	
	m_simpleProgram = BuildProgram(SimpleVertexShader, SimpleFragmentShader);
	
	glUseProgram(m_simpleProgram);
    
    // Set the projection matrix.
    GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
	mat4 projectionMatrix = mat4::Frustum(-1.6f, 1.6, -2.4, 2.4, 4, 10);
	glUniformMatrix4fv(projectionUniform, 1, 0, projectionMatrix.Pointer());
}


void RenderingEngine2::Render() const
{
    //利用灰色清除缓冲区
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	GLuint positionSlot = glGetAttribLocation(m_simpleProgram, "Position");
	GLuint colorSlot = glGetAttribLocation(m_simpleProgram, "SourceColor");
		
    //设置顶点属性
	glEnableVertexAttribArray(positionSlot);
	glEnableVertexAttribArray(colorSlot);
	
	
	//设置模型矩阵
	mat4 rotation(m_animation.Current.ToMatrix());
    mat4 translation = mat4::Translate(0, 0, -10);
	
	GLint modelviewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
    mat4 modelviewMatrix = translation * rotation;
    glUniformMatrix4fv(modelviewUniform, 1, 0, modelviewMatrix.Pointer());
	
	
	//绘制图形
	// Draw the cone.
    {
		GLsizei stride = sizeof(Vertex);
		const GLvoid* pCoords = m_cone[0].Position.Pointer();
		const GLvoid* pColors = m_cone[0].Color.Pointer();
		glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
		glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cone.size());
    }
    
    // Draw the disk that caps off the base of the cone.
    {
		GLsizei stride = sizeof(Vertex);
		const GLvoid* pCoords = m_disk[0].Position.Pointer();
		const GLvoid* pColors = m_disk[0].Color.Pointer();
		glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pCoords);
		glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_disk.size());
    }


    //关闭顶点属性 (后续不会继续使用时关闭)
	glDisableVertexAttribArray(positionSlot);
	glDisableVertexAttribArray(colorSlot);
}


void RenderingEngine2::UpdateAnimation(float timeStep)
{
    if (m_animation.Current == m_animation.End)
        return;
	
	m_animation.Elapsed += timeStep;
	
    if (m_animation.Elapsed >= AnimationDuration) {
        m_animation.Current = m_animation.End;
    } else {
        float mu = m_animation.Elapsed / AnimationDuration;
        m_animation.Current = m_animation.Start.Slerp(mu, m_animation.End);
    }
}

void RenderingEngine2::OnRotate(DeviceOrientation newOrientation)
{
    vec3 direction;
	
	switch (newOrientation) {
		case DeviceOrientationUnknown:
        case DeviceOrientationPortrait:
            direction = vec3(0, 1, 0);
            break;
		case DeviceOrientationPortraitUpsideDown:
            direction = vec3(0, -1, 0);
            break;
            
        case DeviceOrientationFaceDown:
            direction = vec3(0, 0, -1);
            break;
            
        case DeviceOrientationFaceUp:
            direction = vec3(0, 0, 1);
            break;
            
        case DeviceOrientationLandscapeLeft:
            direction = vec3(+1, 0, 0);
            break;
            
        case DeviceOrientationLandscapeRight:
            direction = vec3(-1, 0, 0);
            break;
		default:
			break;
			
	}
	m_animation.Elapsed = 0;
	m_animation.Start = m_animation.Current = m_animation.End;
	m_animation.End = Quaternion::CreateFromVectors(vec3(0, 1, 0), vec3(0, -1, 0));
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

