#include <Windows.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>
#include <cmath>

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;

const double PI = 3.14159265358979323846f;

#pragma comment (lib, "OpenGL32.lib")

#define WINDOW_TITLE "OpenGL Window"

int questionToDisplay = 1;
float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;


// ------------------- camera state -----------------------
float camTargetX = 0.0f, camTargetY = 0.0f, camTargetZ = 0.0f;
float camDistance = 4.0f;
float camYaw = 0.0f;
float camPitch = 0.0f;
const float CAM_MIN_DIST = 0.5f, CAM_MAX_DIST = 50.0f;
const float DEG2RAD = PI / 180.0f;

// Mouse interaction state
bool leftMouseDown = false;
bool rightMouseDown = false;
bool middleMouseDown = false;
POINT lastMouse = { 0,0 };
float orbitSpeed = 0.2f; // degree /pixel
float panSpeed = 0.005f;
float zoomSpeed = 0.1f;

// simple 3 float vector helpers
inline void vec3_sub(const float a[3], const float b[3], float out[3]) { out[0] = a[0] - b[0]; out[1] = a[1] - b[1]; out[2] = a[2] - b[2]; }
inline void vec3_add_mut(float a[3], const float b[3]) { a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; }
inline void vec3_scale(const float a[3], float s, float out[3]) { out[0] = a[0] * s; out[1] = a[1] * s; out[2] = a[2] * s; }
inline float vec3_len(const float a[3]) { return sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]); }
inline void vec3_norm(float a[3]) { float l = vec3_len(a); if (l > 1e-6f) { a[0] /= l; a[1] /= l; a[2] /= l; } }
inline void vec3_cross(const float a[3], const float b[3], float out[3]) {
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}


GLUquadric* gluObject = nullptr;
float starPoints = 10.0f;

enum ProjectMode { ORTHO = 0, PERSPECTIVE = 1, FRUSTUM = 2 };
ProjectMode projMode = ORTHO;

float fovy = 45.0f;
float zNear = 0.1f, zFar = 10.0f;

void updateProjection(int width, int height);


LRESULT WINAPI WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		//case WM_LBUTTONDOWN:
		//	leftMouseDown = true;
		//	SetCapture(hWnd); // Capture mouse input to this window (even if cursor leaves window)
		//	lastMouse.x = GET_X_LPARAM(lParam); // store mouse X
		//	lastMouse.y = GET_Y_LPARAM(lParam); // store mouse Y
		//	break;
		//case WM_LBUTTONUP:
		//	leftMouseDown = false;
		//	ReleaseCapture(); // Stop capturing mouse (return to normal behavior)
		//	break;
	case WM_MBUTTONDOWN:
		middleMouseDown = true;
		SetCapture(hWnd); // Capture mouse input to this window (even if cursor leaves window)
		lastMouse.x = GET_X_LPARAM(lParam); // store mouse X
		lastMouse.y = GET_Y_LPARAM(lParam); // store mouse Y
		break;
	case WM_MBUTTONUP:
		middleMouseDown = false;
		ReleaseCapture();
		break;
	case WM_RBUTTONDOWN:
		rightMouseDown = true;
		SetCapture(hWnd); // Capture mouse input to this window (even if cursor leaves window)
		lastMouse.x = GET_X_LPARAM(lParam);
		lastMouse.y = GET_Y_LPARAM(lParam);
		break;
	case WM_RBUTTONUP:
		rightMouseDown = false;
		ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
	{
		int mx = GET_X_LPARAM(lParam);
		int my = GET_Y_LPARAM(lParam);
		int dx = mx - lastMouse.x; // delta x, calculate how much mouse X move
		int dy = my - lastMouse.y; // delta y, calculate how much mouse Y move
		if (rightMouseDown) {
			// orbit/rotate: change yaw/pitch
			// --------------- invert if you prefer -----------
			camYaw += -dx * orbitSpeed; // Mouse left/right rotates camera horizontally
			camPitch += dy * orbitSpeed;  // Mouse up/down rotates camera vertically (negative because screen Y is inverted)
			// --------------- invert if you prefer -----------

			// Clamp pitch to prevent flipping upside down
			if (camPitch > 89.0f) camPitch = 89.0f;  // Can't look more than 89� up
			if (camPitch < -89.0f) camPitch = -89.0f; // Can't look more than 89� down
		}
		else if (middleMouseDown) {
			// pan/translate: move target in camera's right & up directions
			// compute camera basis quickly
			float yawR2 = camYaw * DEG2RAD;
			float pitchR2 = camPitch * DEG2RAD;
			// Calculate camera's viewing direction
			float forward[3] = {
				-cosf(pitchR2) * sinf(yawR2), // Forward X component
				-sinf(pitchR2),                // Forward Y component
				-cosf(pitchR2) * cosf(yawR2)   // Forward Z component
			}; // points from eye to target (negative of eye->target)

			float upv[3] = { 0.0f, 1.0f, 0.0f }; // the world's up vector 
			float rightv[3];
			vec3_cross(upv, forward, rightv); // Calculate camera's right vector // right = up x forward (cross product)
			vec3_norm(rightv); // normalize the vector, only left direction vector
			float camUpWorld[3];
			vec3_cross(forward, rightv, camUpWorld);  // Calculate camera's up vector (forward � right)

			// apply pan (screen-space -> world)
			// Move target based on mouse movement
			// change dx -> -dx and dy -> -dy to inverse cam
			camTargetX += (dx * panSpeed) * rightv[0] + (dy * panSpeed) * camUpWorld[0];
			camTargetY += (dx * panSpeed) * rightv[1] + (dy * panSpeed) * camUpWorld[1];
			camTargetZ += (dx * panSpeed) * rightv[2] + (dy * panSpeed) * camUpWorld[2];
		}
		lastMouse.x = mx;
		lastMouse.y = my;
	}
	break;
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // Get scroll amount (+120 or -120 per notch)
		// zoom (dolly) � scroll forward = zoom in
		float factor = 1.0f - (zDelta / 120.0f) * zoomSpeed; // Calculate zoom factor
		// Apply zoom
		camDistance *= factor;
		// Clamp to prevent getting too close or too far
		if (camDistance < CAM_MIN_DIST) camDistance = CAM_MIN_DIST;
		if (camDistance > CAM_MAX_DIST) camDistance = CAM_MAX_DIST;
	}
	break;



	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1': questionToDisplay = 1; break;
		case '2': questionToDisplay = 2; break;
		case '3': questionToDisplay = 3; break;
		case '4': questionToDisplay = 4; break;
		case '5': questionToDisplay = 5; break;
		case '6': questionToDisplay = 6; break;
		case '7': questionToDisplay = 7; break;
		case '8': questionToDisplay = 8; break;
		case '9': questionToDisplay = 9; break;
		case VK_ESCAPE: PostQuitMessage(0); break;
			// WASD pan target horizontally (relative to camera)
		case 'W': case 'w': // forward pan (toward camera forward)
		{
			// move target along camera forward vector
			float yawR = camYaw * DEG2RAD;
			float pitchR = camPitch * DEG2RAD;
			float forward[3] = {
				-cosf(pitchR) * sinf(yawR),
				-sinf(pitchR),
				-cosf(pitchR) * cosf(yawR)
			};
			camTargetX += forward[0] * 0.1f;
			camTargetY += forward[1] * 0.1f;
			camTargetZ += forward[2] * 0.1f;
		}
		break;
		case 'S': case 's':
		{
			float yawR = camYaw * DEG2RAD;
			float pitchR = camPitch * DEG2RAD;
			float forward[3] = {
				-cosf(pitchR) * sinf(yawR),
				-sinf(pitchR),
				-cosf(pitchR) * cosf(yawR)
			};
			camTargetX -= forward[0] * 0.1f;
			camTargetY -= forward[1] * 0.1f;
			camTargetZ -= forward[2] * 0.1f;
		}
		break;
		case 'A': case 'a':
		{
			// pan left using right vector
			float yawR = camYaw * DEG2RAD;
			float forward[3] = {
				-cosf(camPitch * DEG2RAD) * sinf(yawR),
				-sinf(camPitch * DEG2RAD),
				-cosf(camPitch * DEG2RAD) * cosf(yawR)
			};
			float upv[3] = { 0.0f,1.0f,0.0f };
			float rightv[3];
			vec3_cross(upv, forward, rightv);
			vec3_norm(rightv);
			camTargetX -= rightv[0] * 0.1f;
			camTargetY -= rightv[1] * 0.1f;
			camTargetZ -= rightv[2] * 0.1f;
		}
		break;
		case 'D': case 'd':
		{
			float yawR = camYaw * DEG2RAD;
			float forward[3] = {
				-cosf(camPitch * DEG2RAD) * sinf(yawR),
				-sinf(camPitch * DEG2RAD),
				-cosf(camPitch * DEG2RAD) * cosf(yawR)
			};
			float upv[3] = { 0.0f,1.0f,0.0f };
			float rightv[3];
			vec3_cross(upv, forward, rightv);
			vec3_norm(rightv);
			camTargetX += rightv[0] * 0.1f;
			camTargetY += rightv[1] * 0.1f;
			camTargetZ += rightv[2] * 0.1f;
		}
		break;
		// R / F to move target up/down
		case 'R': case 'r': camTargetY += 0.1f; break;
		case 'F': case 'f': camTargetY -= 0.1f; break;
			// Arrow keys rotate camera a bit (alternate to mouse)
		case VK_LEFT: camYaw -= 2.0f; break;
		case VK_RIGHT: camYaw += 2.0f; break;
		case VK_UP: camPitch += 2.0f; if (camPitch > 89.0f) camPitch = 89.0f; break;
		case VK_DOWN: camPitch -= 2.0f; if (camPitch < -89.0f) camPitch = -89.0f; break;

		case VK_SPACE:
			glLoadIdentity();
			rotateX = rotateY = rotateZ = 0.0f;
			positionX = positionY = positionZ = 0.0f;
			break;

			// Projection controls
		case 'P': case 'p': // perspective (gluPerspective)
			projMode = PERSPECTIVE;
			updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
			break;
			//case 'F': // frustum (glFrustum)
			//	projMode = FRUSTUM;
			//	updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
			//	break;
		case 'O': case 'o': // orthographic
			projMode = ORTHO;
			updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
			break;
		case 'K': case 'k':// decrease fovy
			fovy -= 1.0f; if (fovy < 1.0f) fovy = 1.0f;
			if (projMode == PERSPECTIVE) updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
			break;
		case 'L': case 'l': // increase fovy
			fovy += 1.0f; if (fovy > 179.0f) fovy = 179.0f;
			if (projMode == PERSPECTIVE) updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
			break;

		default:
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}
//--------------------------------------------------------------------

bool initPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.cAlphaBits = 8;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 0;

	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;

	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	// choose pixel format returns the number most similar pixel format available
	int n = ChoosePixelFormat(hdc, &pfd);

	// set pixel format returns whether it sucessfully set the pixel format
	if (SetPixelFormat(hdc, n, &pfd))
	{
		return true;
	}
	else
	{
		return false;
	}
}
//--------------------------------------------------------------------

void drawCube() {
	glBegin(GL_QUADS);
	// Front (Z+)
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);   glVertex3f(-0.5f, 0.5f, 0.5f);
	// Back (Z-)
	glNormal3f(0.0f, 0.0f, -1.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, -0.5f);   glVertex3f(0.5f, -0.5f, -0.5f);
	// Top (Y+)
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.5f, 0.5f, -0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);   glVertex3f(0.5f, 0.5f, -0.5f);
	// Bottom (Y-)
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
	glVertex3f(0.5f, -0.5f, 0.5f);   glVertex3f(-0.5f, -0.5f, 0.5f);
	// Right (X+)
	glNormal3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.5f, -0.5f, -0.5f); glVertex3f(0.5f, 0.5f, -0.5f);
	glVertex3f(0.5f, 0.5f, 0.5f);   glVertex3f(0.5f, -0.5f, 0.5f);
	// Left (X-)
	glNormal3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glVertex3f(-0.5f, 0.5f, 0.5f);   glVertex3f(-0.5f, 0.5f, -0.5f);
	glEnd();
}

void drawPyramid() {
	glBegin(GL_TRIANGLES);
	// Front
	glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, 0.5f);
	// Right
	glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
	// Back
	glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
	// Left
	glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
	glEnd();
}

void Display(HWND hWnd)
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f); // gray

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// compute eye positon from spherical coords (yaw, pitch in degree)
	// ---------- compute yaw, pitch, distance into camera eye position ----------
	float yawR = camYaw * DEG2RAD; // yaw in radian
	float pitchR = camPitch * DEG2RAD; // pitch in radian
	float cp = cosf(pitchR);
	float sx = sinf(yawR);
	float cx = cosf(yawR);
	float sy = sinf(pitchR);

	// eye relative to target
	float eyeX = camTargetX + camDistance * cp * sx;
	float eyeY = camTargetY + camDistance * sy;
	float eyeZ = camTargetZ + camDistance * cp * cx;
	// ----------------------------------------------------------------------------

	// gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ); up usually is (0,1,0)
	gluLookAt(eyeX, eyeY, eyeZ,
		camTargetX, camTargetY, camTargetZ,
		0.0f, 1.0f, 0.0f);

	// shape Rotation and Translation
	//glTranslatef(positionX, positionY, -4.0f + positionZ);
	//glRotatef(rotateY, 0.0f, 1.0f, 0.0f); // y axis
	//glRotatef(rotateX, 1.0f, 0.0f, 0.0f); // x axis
	//glRotatef(rotateZ, 0.0f, 0.0f, 1.0f); // z axis

	switch (questionToDisplay) {
	case 1:
	{
		// high arm
		glPushMatrix();
		glTranslatef(0.5f, 0.5f, 0.5f);
		// Front - black
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.5f, 0.0f);

		glVertex3f(0.0f, 0.5f, 0.0f);
		glEnd();

		// Left - yellow
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glEnd();

		// Top - blue
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 0.0f);
		glEnd();
		// right - green
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(1.0f, 0.5f, 0.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glEnd();
		// Bottom - red
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glEnd();
		// Back - white
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glEnd();
		glPopMatrix();

		// lower arm
		glPushMatrix();
		glTranslatef(-0.5f, 0.0f, 0.0f);
		// Front - white
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glVertex3f(1.0f, 0.5f, 0.0f);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glEnd();

		// Left - red
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glEnd();

		// Top - green
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 0.0f);
		glEnd();
		// right - blue
		glColor3f(0.0f, 0.0f, 1.0f);
		glBegin(GL_QUADS);
		glVertex3f(1.0f, 0.5f, 0.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glEnd();
		// Bottom - yellow
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 0.0f);
		glEnd();
		// Back - black
		glColor3f(0.0f, 0.0f, 0.0f);
		glBegin(GL_QUADS);
		glVertex3f(0.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.5f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 1.0f);
		glEnd();
		glPopMatrix();





	}
	break;
	case 2:
	{
		glPushMatrix();
		glScalef(0.5f, 0.5f, 0.5f);

		glPushMatrix();
		glTranslatef(0.0f, 3.2f, 0.0f);

		// Head (White Box)
		glColor3f(1.0f, 1.0f, 1.0f);
		glPushMatrix();
		glScalef(0.8f, 0.8f, 0.8f);
		drawCube();
		glPopMatrix();

		// Antenna
		glColor3f(1.0f, 0.9f, 0.0f);
		glPushMatrix();
		glTranslatef(0.0f, 0.45f, 0.0f); // On top
		glScalef(1.5f, 0.4f, 0.1f);
		drawPyramid();
		glPopMatrix();

		glPopMatrix();

		// Chest (Blue)
		glPushMatrix();
		glColor3f(0.0f, 0.2f, 0.8f);
		glTranslatef(0.0f, 2.0f, 0.0f);
		glScalef(1.4f, 1.2f, 1.0f);
		drawCube();
		glPopMatrix();

		// Waist (Red)
		glPushMatrix();
		glColor3f(0.8f, 0.0f, 0.0f);
		glTranslatef(0.0f, 1.0f, 0.0f);
		glScalef(1.1f, 0.8f, 0.9f);
		drawCube();
		glPopMatrix();

		// Left Arm
		glPushMatrix();
		glTranslatef(-1.1f, 2.4f, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);     // Grey Shoulder
		gluSphere(gluObject, 0.45f, 20, 20);

		glColor3f(1.0f, 1.0f, 1.0f);     // White Arm
		glPushMatrix();
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(gluObject, 0.3f, 0.25f, 1.5f, 16, 2);
		glPopMatrix();

		glColor3f(0.2f, 0.2f, 0.2f);     // Dark Hand
		glTranslatef(0.0f, -1.6f, 0.0f);
		gluSphere(gluObject, 0.3f, 10, 10);
		glPopMatrix();

		// Right Arm
		glPushMatrix();
		glTranslatef(1.1f, 2.4f, 0.0f);
		glColor3f(0.6f, 0.6f, 0.6f);
		gluSphere(gluObject, 0.45f, 20, 20);

		glColor3f(1.0f, 1.0f, 1.0f);
		glPushMatrix();
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(gluObject, 0.3f, 0.25f, 1.5f, 16, 2);
		glPopMatrix();

		glColor3f(0.2f, 0.2f, 0.2f);
		glTranslatef(0.0f, -1.6f, 0.0f);
		gluSphere(gluObject, 0.3f, 10, 10);
		glPopMatrix();

		// Left Leg
		glPushMatrix();
		glTranslatef(-0.4f, 0.6f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPushMatrix();
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(gluObject, 0.4f, 0.5f, 2.0f, 16, 2);
		glPopMatrix();

		glColor3f(0.8f, 0.0f, 0.0f); // Red Foot
		glTranslatef(0.0f, -2.1f, 0.2f);
		glScalef(0.7f, 0.4f, 1.2f);
		drawCube();
		glPopMatrix();

		// Right Leg
		glPushMatrix();
		glTranslatef(0.4f, 0.6f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPushMatrix();
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(gluObject, 0.4f, 0.5f, 2.0f, 16, 2);
		glPopMatrix();

		glColor3f(0.8f, 0.0f, 0.0f);
		glTranslatef(0.0f, -2.1f, 0.2f);
		glScalef(0.7f, 0.4f, 1.2f);
		drawCube();
		glPopMatrix();

		glPopMatrix();

	}
	break;
	case 3:
	{


	}
	break;

	}
}
//--------------------------------------------------------------------

int main(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = WindowProcedure;
	wc.lpszClassName = WINDOW_TITLE;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) return false;

	HWND hWnd = CreateWindow(WINDOW_TITLE, WINDOW_TITLE, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, wc.hInstance, NULL);

	//--------------------------------
	//	Initialize window for OpenGL
	//--------------------------------

	HDC hdc = GetDC(hWnd);

	//	initialize pixel format for the window
	initPixelFormat(hdc);

	//	get an openGL context
	HGLRC hglrc = wglCreateContext(hdc);

	//	make context current
	if (!wglMakeCurrent(hdc, hglrc)) return false;

	// now it's safe to create GLU objects
	gluObject = gluNewQuadric();

	//--------------------------------
	//	End initialization
	//--------------------------------

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	// set initial projection (will be updated in WM_SIZE too)
	updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT);

	glMatrixMode(GL_MODELVIEW);


	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Display(hWnd);

		SwapBuffers(hdc);
	}

	if (gluObject) gluDeleteQuadric(gluObject);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);

	UnregisterClass(WINDOW_TITLE, wc.hInstance);

	return true;
}
//--------------------------------------------------------------------
void updateProjection(int width, int height)
{
	if (height == 0) height = 1;
	float aspect = (float)width / (float)height;


	glViewport(0, 0, width, height);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	switch (projMode)
	{
	case ORTHO:
		// keep the original small-world units but adapt to aspect
		glOrtho(-1.0 * aspect, 3.0 * aspect, -1.0f, 3.0f, -10.0f, 10.0f);
		break;


	case PERSPECTIVE:
		// gluPerspective(fovy, aspect, zNear, zFar)
		gluPerspective(fovy, aspect, zNear, zFar);
		break;


	case FRUSTUM:
	{
		// derive left/right/top/bottom from fovy and aspect for a symmetric frustum
		float top = zNear * tanf((fovy * (float)PI / 180.0f) * 0.5f);
		float bottom = -top;
		float right = top * aspect;
		float left = -right;
		// glFrustum(left, right, bottom, top, zNear, zFar)
		glFrustum(left, right, bottom, top, zNear, zFar);
	}
	break;
	}


	glMatrixMode(GL_MODELVIEW);
}