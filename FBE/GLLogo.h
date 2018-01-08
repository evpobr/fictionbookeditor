#pragma once

#include "resource.h"
#include "stdafx.h"
#include <gl\gl.h>
#include <gl\glu.h>

enum TEXTURES_TYPES
{
	CENTER,
	ORANGE,
	GREEN,
	DOC,
	XML,
	OTHER,
	HTML,
	PRC,
	TEXT
};

class CGLLogoView : public CWindowImpl<CGLLogoView, CStatic>
{
public:
	BEGIN_MSG_MAP(CGLLogoView)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLeftButtonDoubleClick)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
	END_MSG_MAP()

	CGLLogoView()
	{
		m_hRC = NULL;

		// initialize variables
		m_z = -8.0f;         // Scene depth
		m_y = 4.0f;          // Height
		m_dr = 0.01f;        // Rotation increment
		m_ds = 0.001f;       // Rotation auto decrement
		m_alpha = 0.0f;      // Alpha value for info panel
		m_alpha_inc = 0.01f; // Alpha increment
		m_glColorOp = GL_XOR;

		m_xrot = m_yrot = m_zrot = 0.0f;

		m_xspeed = 0.5f;
		m_yspeed = 0.3f;
		m_zspeed = 0.2f;
		m_ds = 0.0f;

		m_bDrawInfo = false;
		m_bShadow = true;     // Draw shadow
		m_bReflection = true; // Draw reflection
		m_bFloor = true;      // Draw floor
		m_bWireFrame = false; // Draw mesh
		m_bCrystallize = false;

		m_Timer = 0;
	}

	~CGLLogoView()
	{
		KillGLWindow();
	}

	BOOL SubclassWindow(HWND hWnd);
	BOOL OpenGLError();

private:
	HDC m_hDC;
	HGLRC m_hRC;

	UINT_PTR m_Timer;

	GLYPHMETRICSFLOAT m_gmf[96];
	GLuint m_font_base;
	GLuint m_logo;
	GLUquadricObj * m_quadric;

	GLfloat m_xrot;      // X rotation
	GLfloat m_yrot;      // Y rotation
	GLfloat m_zrot;      // Z rotation
	GLfloat m_xspeed;    // X rotation speed
	GLfloat m_yspeed;    // Y rotation speed
	GLfloat m_zspeed;    // Z rotation speed
	GLfloat m_z;         // Scene depth
	GLfloat m_y;         // Height
	GLfloat m_dr;        // Rotation increment
	GLfloat m_ds;        // Rotation auto decrement
	GLfloat m_alpha;     // Alpha value for info panel
	GLfloat m_alpha_inc; // Alpha increment
	GLint m_glColorOp;

	bool m_bDrawInfo;
	bool m_bShadow;      // Draw shadow
	bool m_bReflection;  // Draw reflection
	bool m_bFloor;       // Draw floor
	bool m_bWireFrame;   // Draw mesh
	bool m_bCrystallize; // Special effect
	POINT m_lastPos;     // Mouse position
	GLuint m_texture[9]; // Storage for textures

	GLfloat m_fShadowMatrix[16];

	LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL &);
	LRESULT OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL &);
	LRESULT OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL &);
	LRESULT OnLeftButtonDoubleClick(UINT, WPARAM, LPARAM, BOOL &);
	LRESULT OnMouseWheel(UINT, WPARAM wParam, LPARAM, BOOL &);
	LRESULT OnTimer(UINT, WPARAM wParam, LPARAM, BOOL &);

	bool CreateGLWindow();
	bool KillGLWindow();
	bool InitGL();
	void SetShadowMatrix(float fDestMat[16], float fLightPos[4], float fPlane[4]);
	void CreateGLFont();
	void Print(const char * fmt, ...);
	bool LoadTextures();
	void ResizeScene(GLsizei width, GLsizei height);
	void CreateLogo();
	void DrawFloor();
	void DrawGLScene();
};
