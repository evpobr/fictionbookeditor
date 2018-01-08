#include "stdafx.h"
#include "GLLogo.h"

GLfloat LightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
GLfloat LightDiffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};
GLfloat LightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 9.0f, -3.0f, 1.0f};
GLfloat LogoColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
// This is the normal for the plane, {0,1,0}
GLfloat Plane[4] = {0, 1, 0, 0};

// Info panel text
const char * Info[] =
    {
        "     Use mouse to rotate logo",
        "     Press 'G' to toggle gravity %s",
        "     Press 'R' to toggle reflection %s",
        "     Press 'S' to toggle shadow %s",
        "     Press 'F' to toggle floor %s",
        "     Press 'W' to toggle wireframe %s",
        "     Press SPACE to stop rotation"};

#define INFO_SIZE sizeof(Info) / sizeof(char *)
bool InfoValues[INFO_SIZE];

void CGLLogoView::SetShadowMatrix(float fDestMat[16], float fLightPos[4], float fPlane[4])
{
	// dot product of plane and light position
	GLfloat dot = fPlane[0] * fLightPos[0] + fPlane[1] * fLightPos[1] + fPlane[1] * fLightPos[2] + fPlane[3] * fLightPos[3];

	// first column
	fDestMat[0] = dot - fLightPos[0] * fPlane[0];
	fDestMat[4] = 0.0f - fLightPos[0] * fPlane[1];
	fDestMat[8] = 0.0f - fLightPos[0] * fPlane[2];
	fDestMat[12] = 0.0f - fLightPos[0] * fPlane[3];

	// second column
	fDestMat[1] = 0.0f - fLightPos[1] * fPlane[0];
	fDestMat[5] = dot - fLightPos[1] * fPlane[1];
	fDestMat[9] = 0.0f - fLightPos[1] * fPlane[2];
	fDestMat[13] = 0.0f - fLightPos[1] * fPlane[3];

	// third column
	fDestMat[2] = 0.0f - fLightPos[2] * fPlane[0];
	fDestMat[6] = 0.0f - fLightPos[2] * fPlane[1];
	fDestMat[10] = dot - fLightPos[2] * fPlane[2];
	fDestMat[14] = 0.0f - fLightPos[2] * fPlane[3];

	// fourth column
	fDestMat[3] = 0.0f - fLightPos[3] * fPlane[0];
	fDestMat[7] = 0.0f - fLightPos[3] * fPlane[1];
	fDestMat[11] = 0.0f - fLightPos[3] * fPlane[2];
	fDestMat[15] = dot - fLightPos[3] * fPlane[3];
}

void CGLLogoView::CreateGLFont()
{
	HFONT font;
	HFONT oldfont;

	m_font_base = glGenLists(96);
	font = CreateFont(-12,                         // Height Of Font
	                  0,                           // Width Of Font
	                  0,                           // Angle Of Escapement
	                  0,                           // Orientation Angle
	                  FW_BOLD,                     // Font Weight
	                  FALSE,                       // Italic
	                  FALSE,                       // Underline
	                  FALSE,                       // Strikeout
	                  ANSI_CHARSET,                // Character Set Identifier
	                  OUT_TT_PRECIS,               // Output Precision
	                  CLIP_TT_ALWAYS,              // Clipping Precision
	                  CLEARTYPE_NATURAL_QUALITY,   // Output Quality
	                  FF_DONTCARE | DEFAULT_PITCH, // Family And Pitch
	                  L"Arial");                   // Font Name

	oldfont = (HFONT)SelectObject(m_hDC, font); // Selects The Font We Want
	wglUseFontOutlines(m_hDC, 32, 96, m_font_base, 0.02f, 0.0001f, WGL_FONT_POLYGONS, m_gmf);

	SelectObject(m_hDC, oldfont); // Selects The Font We Want
	DeleteObject(font);           // Delete The Font
}

// Custom GL "Print" Routine
void CGLLogoView::Print(const char * fmt, ...)
{
	char text[256]; // Holds Our String
	va_list ap;     // Pointer To List Of Arguments
	if (fmt == nullptr)
		return;

	va_start(ap, fmt);         // Parses The String For Variables
	vsprintf_s(text, fmt, ap); // And Converts Symbols To Actual Numbers
	va_end(ap);                // Results Are Stored In Text

	glPushAttrib(GL_LIST_BIT);                         // Pushes The Display List Bits
	glListBase(m_font_base - 32);                      // Sets The Base Character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // Draws The Display List Text
	glPopAttrib();                                     // Pops The Display List Bits
}

// Load bitmaps from resource and convert to textures
bool CGLLogoView::LoadTextures()
{
	HBITMAP hBMP;
	BITMAP BMP;

	int Texture[9] = {IDB_CENTER, IDB_ORANGE, IDB_GREEN, IDB_XML, IDB_DOC, IDB_HTML, IDB_OTHER, IDB_PRC, IDB_TEXT};
	glGenTextures(9, &m_texture[0]);
	for (int i = 0; i < 9; i++)
	{
		hBMP = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(Texture[i]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		if (hBMP)
		{
			GetObject(hBMP, sizeof(BMP), &BMP);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glBindTexture(GL_TEXTURE_2D, m_texture[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
			DeleteObject(hBMP);
		}
	}
	return true;
}

// Resize And Initialize The GL Window
void CGLLogoView::ResizeScene(GLsizei width, GLsizei height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// All Setup For OpenGL Goes Here
bool CGLLogoView::InitGL()
{
	LoadTextures();

	// what color to clear to in color buffer
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Cull backs of polygons
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LINE_SMOOTH);
	// Enable smooth shading
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Enable a single OpenGL light.
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_quadric = gluNewQuadric();
	gluQuadricNormals(m_quadric, GLU_SMOOTH);
	gluQuadricTexture(m_quadric, GL_TRUE);

	SetShadowMatrix(m_fShadowMatrix, LightPosition, Plane);

	CreateLogo();
	CreateGLFont();

	return true;
}

void CGLLogoView::CreateLogo()
{
	GLfloat angle[] = {45.0f, 90.0f, 90.0f};
	GLfloat axis[][3] = {{0, 0, 1}, {0, 1, 0}, {1, 0, 0}};

	m_logo = glGenLists(1);
	glNewList(m_logo, GL_COMPILE);

	glMaterialfv(GL_FRONT, GL_SPECULAR, LogoColor);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, LogoColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 128.0);

	// draw main sphere
	glPushMatrix();
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(-5.0f, 0.0f, 0.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, m_texture[CENTER]);
	gluSphere(m_quadric, 0.8f, 32, 32);
	glPopMatrix();

	GLfloat dz = 0.0f;
	for (int i = 0; i < 3; i++)
	{
		// draw left cylinder with sphere
		glPushMatrix();
		glTranslatef(0.0f, -0.75f, 0.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glBindTexture(GL_TEXTURE_2D, m_texture[ORANGE]);
		gluCylinder(m_quadric, 0.2f, 0.2f, 1.0f, 32, 1);

		glTranslatef(0.0f, 0.0f, 1.45f);
		glRotatef(-angle[i], axis[i][0], axis[i][1], axis[i][2]);
		glBindTexture(GL_TEXTURE_2D, m_texture[i * 2 + 3]);
		gluSphere(m_quadric, 0.5f, 32, 32);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.0f, 0.75f, 0.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		if (i == 2)
		{
			glBindTexture(GL_TEXTURE_2D, m_texture[GREEN]);
			dz = 0.8f;
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_texture[ORANGE]);
		}
		gluCylinder(m_quadric, 0.2f, 0.2f, 1.0f + dz, 32, 1);

		glTranslatef(0.0f, 0.0f, 1.45f + dz);
		glRotatef(angle[i], axis[i][0], axis[i][1], axis[i][2]);
		glBindTexture(GL_TEXTURE_2D, m_texture[i * 2 + 4]);
		gluSphere(m_quadric, 0.5f, 32, 32);
		glPopMatrix();

		// rotate for next axe
		glRotatef(120.0f, 1.0f, 1.0f, 1.0f);
	}
	glEndList();
}

void CGLLogoView::DrawFloor()
{
	GLfloat fExtent = 40.0f;
	GLfloat fStep = 2.0f;
	GLfloat y = -2.3f;
	GLfloat fColor;
	GLfloat iStrip, iRun;
	GLint iBounce = 0;

	glShadeModel(GL_FLAT);
	glDisable(GL_TEXTURE_2D);

	for (iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
		{
			if ((iBounce % 2) == 0)
				fColor = m_bFloor ? 0.0f : 1.0f;
			else
				fColor = 1.0f;
			glColor4f(fColor, fColor, fColor, 0.7f);
			glVertex3f(iStrip, y, iRun);
			glVertex3f(iStrip + fStep, y, iRun);
			iBounce++;
		}
		glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
}

void CGLLogoView::DrawGLScene()
{
	wglMakeCurrent(m_hDC, m_hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0, m_y, m_z, 0, 0, 0, 0, 1, 0);

	glFrontFace(GL_CCW);
	if (m_bReflection)
	{
		glPushMatrix();
		// Move light under floor to light the "reflected" world
		LightPosition[1] *= -1.0;
		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
		LightPosition[1] *= -1.0;

		// Geometry is mirrored, swap orientation
		glFrontFace(GL_CW);
		glScalef(1.0f, -1.0f, 1.0f);
		glTranslatef(0.0, 5.6f, 0);
		glRotatef(m_xrot, 1, 0, 0);
		glRotatef(m_yrot, 0, 1, 0);
		glRotatef(m_zrot, 0, 0, 1);
		if (m_bCrystallize)
		{
			glLogicOp(m_glColorOp);
			glEnable(GL_COLOR_LOGIC_OP);
		}
		glCallList(m_logo);
		if (m_bCrystallize)
		{
			glLogicOp(GL_CLEAR);
			glDisable(GL_COLOR_LOGIC_OP);
		}
		glFrontFace(GL_CCW);

		glPopMatrix();
	}

	// draw the shadow
	if (m_bShadow)
	{
		glPushMatrix();
		glColor4f(0.3f, 0.3f, 0.3f, 1.0f);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);

		glTranslatef(1.5f, 0.0f, 0.0f);
		glMultMatrixf(m_fShadowMatrix);
		glRotatef(m_xrot, 1, 0, 0);
		glRotatef(m_yrot, 0, 1, 0);
		glRotatef(m_zrot, 0, 0, 1);
		glCallList(m_logo);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	// Draw the ground transparently over the reflection
	glDisable(GL_LIGHTING);
	DrawFloor();
	glEnable(GL_LIGHTING);
	// Restore correct lighting and draw the world correctly
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	// draw object
	glPushMatrix();
	if (m_bCrystallize)
	{
		glLogicOp(m_glColorOp);
		glEnable(GL_COLOR_LOGIC_OP);
	}
	glTranslatef(0.0f, 1.0f, 0.0f);
	glRotatef(m_xrot, 1, 0, 0);
	glRotatef(m_yrot, 0, 1, 0);
	glRotatef(m_zrot, 0, 0, 1);
	glCallList(m_logo);
	if (m_bCrystallize)
	{
		glLogicOp(GL_CLEAR);
		glDisable(GL_COLOR_LOGIC_OP);
	}
	glPopMatrix();

	// draw info window
	if (m_bDrawInfo || (m_alpha_inc < 0.0f))
	{
		glLoadIdentity();
		gluLookAt(0, 0, -5, 0, 0, 0, 0, 1, 0);
		glTranslatef(0.0f, -1.0f, 1.0f);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glColor4f(0.3f, 0.3f, 0.4f, m_alpha - 0.2f);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-2.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(2.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(2.0f, -1.3f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-2.0f, -1.3f, 0.0f);
		glEnd();

		glDisable(GL_DEPTH_TEST);
		glColor4f(1.0f, 1.0f, 0.0f, m_alpha);
		glRotatef(180.0, 0, 1, 0);
		glScalef(0.25f, 0.25f, 0.25f);
		glTranslatef(-8.0f, 2.6f, 0.0f);

		InfoValues[1] = (m_ds == 0.001f);
		InfoValues[2] = m_bReflection;
		InfoValues[3] = m_bShadow;
		InfoValues[4] = m_bFloor;
		InfoValues[5] = m_bWireFrame;
		for (int i = 0; i < INFO_SIZE; i++)
		{
			glPushMatrix();
			glTranslatef(0.0f, -i * 1.2f, 0.0f);
			if (i > 0 && i < INFO_SIZE - 1)
				Print(Info[i], InfoValues[i] ? "off" : "on");
			else
				Print(Info[i]);
			glPopMatrix();
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);

		if (m_alpha < 1.0f || (!m_bDrawInfo && m_alpha > 0.0f))
			m_alpha += m_alpha_inc;
	}

	// Recalculate rotation speed
	if (m_xspeed != 0)
		m_xspeed > 0 ? m_xspeed -= m_ds : m_xspeed += m_ds;
	m_xrot += m_xspeed;
	if (m_yspeed != 0)
		m_yspeed > 0 ? m_yspeed -= m_ds : m_yspeed += m_ds;
	m_yrot += m_yspeed;
	if (m_zspeed != 0)
		m_zspeed > 0 ? m_zspeed -= m_ds : m_zspeed += m_ds;
	m_zrot += m_zspeed;
}

bool CGLLogoView::CreateGLWindow()
{
	GLuint PixelFormat;
	static PIXELFORMATDESCRIPTOR pfd =
	    {
	        sizeof(PIXELFORMATDESCRIPTOR), // Size Of This Pixel Format Descriptor
	        1,                             // Version Number
	        PFD_DRAW_TO_WINDOW |           // Format Must Support Window
	            PFD_SUPPORT_OPENGL |       // Format Must Support OpenGL
	            PFD_DOUBLEBUFFER,          // Must Support Double Buffering
	        PFD_TYPE_RGBA,                 // Request An RGBA Format
	        16,                            // Select Our Color Depth
	        0, 0, 0, 0, 0, 0,              // Color Bits Ignored
	        0,                             // No Alpha Buffer
	        0,                             // Shift Bit Ignored
	        0,                             // No Accumulation Buffer
	        0, 0, 0, 0,                    // Accumulation Bits Ignored
	        16,                            // 16Bit Z-Buffer (Depth Buffer)
	        0,                             // No Stencil Buffer
	        0,                             // No Auxiliary Buffer
	        PFD_MAIN_PLANE,                // Main Drawing Layer
	        0,                             // Reserved
	        0, 0, 0                        // Layer Masks Ignored
	    };

	if ((m_hDC = GetDC()) != nullptr)
		if ((PixelFormat = ChoosePixelFormat(m_hDC, &pfd)) != 0)
			if (SetPixelFormat(m_hDC, PixelFormat, &pfd) != FALSE)
				if ((m_hRC = wglCreateContext(m_hDC)) != nullptr)
					if (wglMakeCurrent(m_hDC, m_hRC) != FALSE)
					{
						RECT rect;
						GetWindowRect(&rect);
						ResizeScene(rect.right - rect.left, rect.bottom - rect.top);

						if (!InitGL())
							return KillGLWindow();

						// set timer
						m_Timer = SetTimer(2, 20, NULL);
						if (!m_Timer)
							return KillGLWindow();

						return TRUE;
					}
	return FALSE;
}

bool CGLLogoView::KillGLWindow()
{
	if (m_Timer)
		::KillTimer(m_hWnd, m_Timer);

	gluDeleteQuadric(m_quadric);

	glDeleteLists(m_logo, 1);
	glDeleteLists(m_font_base, 96);
	glDeleteTextures(9, &m_texture[0]);

	if (m_hRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
		m_hRC = NULL;
	}

	if (m_hDC && !ReleaseDC(m_hDC))
		m_hDC = NULL;

	return 0;
}

BOOL CGLLogoView::SubclassWindow(HWND hWnd)
{
	CWindowImpl<CGLLogoView, CStatic>::SubclassWindow(hWnd);
	if (!CreateGLWindow())
		KillGLWindow();
	return TRUE;
}

BOOL CGLLogoView::OpenGLError()
{
	return !(m_hDC && m_hRC);
}

LRESULT CGLLogoView::OnKeyDown(UINT, WPARAM wParam, LPARAM /*lParam*/, BOOL &)
{
	switch (wParam)
	{
	case VK_HOME:
		m_y -= 0.05f;
		break;
	case VK_END:
		m_y += 0.05f;
		break;
	case VK_PRIOR:
		m_zspeed -= m_dr;
		break; //z-=0.02f;
	case VK_NEXT:
		m_zspeed += m_dr;
		break; //z+=0.02f;
	case VK_UP:
		m_xspeed -= m_dr;
		break;
	case VK_DOWN:
		m_xspeed += m_dr;
		break;
	case VK_LEFT:
		m_yspeed -= m_dr;
		break;
	case VK_RIGHT:
		m_yspeed += m_dr;
		break;
	case VK_SPACE:
	{
		m_xspeed = 0;
		m_yspeed = 0;
		m_zspeed = 0;
		break;
	}
	case 'S': // Toggle shadows
	{
		m_bShadow = !m_bShadow;
		break;
	}
	case 'R': // Toggle reflection
	{
		m_bReflection = !m_bReflection;
		break;
	}
	case 'W': // Toggle wireframe mode
	{
		m_bWireFrame = !m_bWireFrame;
		if (m_bWireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	}
	case 'G': // Toggle gravity
	{
		m_ds > 0 ? m_ds = 0.0f : m_ds = 0.001f;
		break;
	}
	case 'F': // Toggle floor
	{
		m_bFloor = !m_bFloor;
		break;
	}
	case 'I':
	case VK_OEM_2: // Toggle info panel
	{
		m_bDrawInfo = !m_bDrawInfo;
		if (m_alpha > 1.0f)
			m_alpha = 1.0f;
		else if (m_alpha < 0.0f)
			m_alpha = 0.0f;
		if (m_bDrawInfo)
			m_alpha_inc = 0.01f;
		else
			m_alpha_inc = -0.01f;
		break;
	}
	case 'C': // Toggle crystallize
	{
		m_bCrystallize = !m_bCrystallize;
		break;
	}
	}
	return TRUE;
}

LRESULT CGLLogoView::OnSize(UINT, WPARAM, LPARAM lParam, BOOL &)
{
	ResizeScene(LOWORD(lParam), HIWORD(lParam)); // LoWord=width, HiWord=height
	DrawGLScene();
	SwapBuffers(m_hDC);
	return TRUE;
}

LRESULT CGLLogoView::OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL &)
{
	m_lastPos.x = LOWORD(lParam);
	m_lastPos.y = HIWORD(lParam);
	return FALSE;
}

LRESULT CGLLogoView::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL &)
{
	if (wParam == MK_LBUTTON)
	{
		float deltaX = (HIWORD(lParam) - m_lastPos.y) / 3.0f;
		float deltaY = (LOWORD(lParam) - m_lastPos.x) / 3.0f;
		m_xspeed = -deltaX / 10.0f;
		m_yspeed = deltaY / 10.0f;
		m_xrot -= deltaX;
		m_yrot += deltaY;
		m_lastPos.x = LOWORD(lParam);
		m_lastPos.y = HIWORD(lParam);
	}
	return FALSE;
}

LRESULT CGLLogoView::OnMouseWheel(UINT, WPARAM wParam, LPARAM, BOOL &)
{
	GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? m_z -= 0.5f : m_z += 0.5f;
	return TRUE;
}

LRESULT CGLLogoView::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL &)
{
	if (wParam == m_Timer)
	{
		DrawGLScene();
		SwapBuffers(m_hDC);
	}
	return TRUE;
}

LRESULT CGLLogoView::OnLeftButtonDoubleClick(UINT, WPARAM, LPARAM, BOOL &)
{
	::PostMessage(GetParent(), WM_RESIZE_OPENGL_WINDOW, 0, 0);
	return TRUE;
}
