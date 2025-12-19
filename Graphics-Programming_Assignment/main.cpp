#include <Windows.h>
#include <windowsx.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>
#include <cmath>
#include "main.h"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;

const double PI = 3.14159265358979323846f;

#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")

#define WINDOW_TITLE "OpenGL Window"

struct Vec3 {
    float x, y, z;
};

float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;

// Robot variable
float leftShoulderAngle = 0.0f;
float leftElbowAngle = 0.0f;
float rightShoulderAngle = 0.0f;
float rightElbowAngle = 0.0f;
float leftHipAngle = 0.0f;
float leftKneeAngle = 0.0f;
float rightHipAngle = 0.0f;
float rightKneeAngle = 0.0f;
float headRotation = 0.0f;
float bodyRotation = 0.0f;

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
void computeNormal(Vec3 a, Vec3 b, Vec3 c) {
    Vec3 u = { b.x - a.x, b.y - a.y, b.z - a.z };
    Vec3 v = { c.x - a.x, c.y - a.y, c.z - a.z };

    Vec3 n = {
        u.y * v.z - u.z * v.y,
        u.z * v.x - u.x * v.z,
        u.x * v.y - u.y * v.x
    };

    float len = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    if (len > 0.0f) {
        n.x /= len; n.y /= len; n.z /= len;
    }

    glNormal3f(n.x, n.y, n.z);
}


GLUquadric* gluObject = nullptr;

enum ProjectMode { ORTHO = 0, PERSPECTIVE = 1, FRUSTUM = 2 };
ProjectMode projMode = ORTHO;

float fovy = 45.0f;
float zNear = 0.1f, zFar = 20.0f;

void updateProjection(int width, int height);

void drawRightArm();
void drawLeftLeg();
void drawRightLeg();

void drawSword();
void drawShield();
void drawUpperBody();
void drawLowerBody();
void drawGundamHead();


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
            if (camPitch > 89.0f) camPitch = 89.0f;  // Can't look more than 89° up
            if (camPitch < -89.0f) camPitch = -89.0f; // Can't look more than 89° down
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
            vec3_cross(forward, rightv, camUpWorld);  // Calculate camera's up vector (forward × right)

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
        // zoom (dolly) – scroll forward = zoom in
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
            // robot control
        case '1': rightShoulderAngle += 5.0f; break;
        case '2': rightShoulderAngle -= 5.0f; break;
        case '3': rightElbowAngle += 5.0f; break;
        case '4': rightElbowAngle -= 5.0f; break;
            // add more ... 
        case 'A': headRotation += 5.0f; break;   
        case 'D': headRotation -= 5.0f; break; 
        case 'W': bodyRotation += 5.0f; break;
        case 'S': bodyRotation -= 5.0f; break;



        case VK_ESCAPE: PostQuitMessage(0); break;
            // Projection controls
        case 'P': case 'p': // perspective (gluPerspective)
            projMode = PERSPECTIVE;
            updateProjection(WINDOW_WIDTH, WINDOW_HEIGHT); // call with a reasonable default - WM_SIZE will set correct viewport
            break;
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

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); // IMPORTANT when scaling
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Light position
    GLfloat light_pos[] = { 5.0f, 5.0f, 5.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
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

void drawCube1(float size) {
    float halfSize = size / 2.0f;
    glBegin(GL_QUADS);
    // Front face
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    // Back face
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    // Top face
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    // Bottom face
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    // Right face
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    // Left face
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
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

void drawPyramid1(float size) {
    float s = size / 2.0f;
    float h = size / 2.0f;

    glBegin(GL_TRIANGLES);
    // Front face
    glNormal3f(0, 0.707, 0.707);
    glVertex3f(0, h, 0);
    glVertex3f(-s, -h, s);
    glVertex3f(s, -h, s);
    // Right face
    glNormal3f(0.707, 0.707, 0);
    glVertex3f(0, h, 0);
    glVertex3f(s, -h, s);
    glVertex3f(s, -h, -s);
    // Back face
    glNormal3f(0, 0.707, -0.707);
    glVertex3f(0, h, 0);
    glVertex3f(s, -h, -s);
    glVertex3f(-s, -h, -s);
    // Left face
    glNormal3f(-0.707, 0.707, 0);
    glVertex3f(0, h, 0);
    glVertex3f(-s, -h, -s);
    glVertex3f(-s, -h, s);
    glEnd();

    // Base
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-s, -h, s);
    glVertex3f(s, -h, s);
    glVertex3f(s, -h, -s);
    glVertex3f(-s, -h, -s);
    glEnd();
}

void drawGundamHead() {
    // Draw neck first 
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.6f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glRotatef(headRotation, 0.0f, 1.0f, 0.0f);  // Rotate around Y axis

    // Centre box
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glScalef(1.8f, 1.6f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Base armor
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, 0.0f);
    glScalef(1.2f, 0.1f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Rear armor
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, -0.75f);
    glScalef(1.6f, 1.4f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Face plate
    glPushMatrix();
    glColor3f(1.0f, 0.85f, 0.85f);
    glTranslatef(0.0f, 0.0f, 0.65f);
    glScalef(1.6f, 1.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left Eye
    glPushMatrix();
    // eyesocket 
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(-0.5f, 0.2f, 0.78f);
    glScalef(0.6f, 0.5f, 0.15f);
    drawCube1(1.0f);
    // eye 
    glColor3f(0.05f, 0.05f, 0.05f);
    glTranslatef(0.0f, 0.0f, 0.8f);
    glScalef(0.8f, 0.8f, 0.5f);
    drawCube1(1.0f);
    // eyeball
    glColor3f(0.3f, 0.3f, 0.3f);
    glTranslatef(0.0f, 0.0f, 0.8f);
    glScalef(0.5f, 0.5f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right Eye 
    glPushMatrix();
    // eyesocket
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(0.5f, 0.2f, 0.78f);
    glScalef(0.6f, 0.5f, 0.15f);
    drawCube1(1.0f);
    // eye
    glColor3f(0.05f, 0.05f, 0.05f);
    glTranslatef(0.0f, 0.0f, 0.8f);
    glScalef(0.8f, 0.8f, 0.5f);
    drawCube1(1.0f);
    // eyeball
    glColor3f(0.3f, 0.3f, 0.3f);
    glTranslatef(0.0f, 0.0f, 0.8f);
    glScalef(0.5f, 0.5f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Mouth
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glColor3f(0.2f, 0.2f, 0.2f);
    // teeth
    glVertex3f(-0.5f, -0.2f, 0.78f);
    glVertex3f(-0.5f, -0.4f, 0.78f);
    glVertex3f(0.5f, -0.2f, 0.78f);
    glVertex3f(0.5f, -0.4f, 0.78f);
    glVertex3f(-0.3f, -0.2f, 0.78f);
    glVertex3f(-0.3f, -0.4f, 0.78f);
    glVertex3f(0.3f, -0.2f, 0.78f);
    glVertex3f(0.3f, -0.4f, 0.78f);
    glVertex3f(-0.1f, -0.2f, 0.78f);
    glVertex3f(-0.1f, -0.4f, 0.78f);
    glVertex3f(0.1f, -0.2f, 0.78f);
    glVertex3f(0.1f, -0.4f, 0.78f);
    glEnd();

    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glColor3f(0.6f, 0.6f, 0.6f);
    glVertex3f(-0.7f, -0.2f, 0.78f);
    glVertex3f(0.7f, -0.2f, 0.78f);
    glVertex3f(0.7f, -0.4f, 0.78f);
    glVertex3f(-0.7f, -0.4f, 0.78f);
    glEnd();

    glColor3f(0.95f, 0.95f, 0.95f);
    // base between blade
    glPushMatrix();
    glTranslatef(0.0f, 1.3f, 0.0f);
    glScalef(0.2f, 0.5f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left Blade
    glPushMatrix();
    glTranslatef(-0.30f, 1.65f, 0.0f);
    glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.3f, 1.2f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Right Blade 
    glPushMatrix();
    glTranslatef(0.30f, 1.65f, 0.0f);
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.3f, 1.2f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Side Armor 
    glColor3f(0.8f, 0.8f, 0.8f);
    // Left Armor
    glPushMatrix();
    glTranslatef(-1.1f, 0.2f, 0.0f);
    glScalef(0.2f, 0.8f, 1.2f);
    drawCube1(1.0f);
    // Layer 1
    glTranslatef(-1.5f, 0.0f, 0.0f);
    glScalef(0.5f, 0.8f, 0.8f);
    drawCube1(1.0f);
    // Layer 2 
    glTranslatef(0.0f, -0.6f, 0.0f);
    glScalef(1.0f, 0.3f, 1.0f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right Armor
    glPushMatrix();
    glTranslatef(1.1f, 0.2f, 0.0f);
    glScalef(0.2f, 0.8f, 1.2f);
    drawCube1(1.0f);
    // Layer 1
    glTranslatef(1.5f, 0.0f, 0.0f);
    glScalef(0.5f, 0.8f, 0.8f);
    drawCube1(1.0f);
    // Layer 2 
    glTranslatef(0.0f, -0.6f, 0.0f);
    glScalef(1.0f, 0.3f, 1.0f);
    drawCube1(1.0f);
    glPopMatrix();

    // Rear Sensor 
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.9f);
    glScalef(0.8f, 0.5f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    glPopMatrix(); // End head rotation matrix
}

void drawUpperBody()
{
    glPushMatrix();
    glRotatef(bodyRotation, 0.0f, 1.0f, 0.0f);
    glScalef(1.2f, 1.2f, 1.2f);
    //main
    glColor3f(0.0f, 0.35f, 0.7f);
    glPushMatrix();
    glScalef(2.2f, 1.4f, 1.7f);
    glTranslatef(0.0f, 0.2f, 0.0f);
    drawCube1(1.0f);
    glPopMatrix();

    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.85f);
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-1.1f, 0.6f, 0.1f);
    glVertex3f(-0.9f, -0.7f, 0.1f);
    glVertex3f(0.0f, 0.7f, 0.2f);
    glVertex3f(0.0f, -0.9f, 0.2f);
    glVertex3f(1.1f, 0.6f, 0.1f);
    glVertex3f(0.9f, -0.7f, 0.1f);
    glEnd();
    glPopMatrix();

    // White Hatch 
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 1.05f);
    glScalef(0.6f, 0.8f, 0.05f);
    drawCube1(1.0f);
    glPopMatrix();

    //yellow vent
    glColor3f(1.0f, 0.8f, 0.0f);
    //Top
    glPushMatrix();
    glTranslatef(-0.75f, 0.6f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();
    //Middle
    glPushMatrix();
    glTranslatef(-0.75f, 0.3f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();
    //Bottom
    glPushMatrix();
    glTranslatef(-0.75f, 0.0f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right Vent
    //Top 
    glPushMatrix();
    glTranslatef(0.75f, 0.6f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();
    //Middle
    glPushMatrix();
    glTranslatef(0.75f, 0.3f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();
    //Bottom
    glPushMatrix();
    glTranslatef(0.75f, 0.0f, 1.0f);
    glScalef(0.5f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    //lower
    glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glScalef(2.1f, 0.2f, 1.55f);
    drawCube1(1.0f);
    glPopMatrix();

    //white center block
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 1.0f);
    glScalef(0.8f, 1.0f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    //backpack
    glColor3f(0.0f, 1.0f, 0.0f);  
    glPushMatrix();
    glTranslatef(0.5f, -1.0f, -1.2f);  
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); 
    gluCylinder(gluObject, 0.3f, 0.3f, 2.0f, 16, 1); 
    glPopMatrix();

    glColor3f(0.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.5f, -1.0f, -1.2f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.3f, 0.3f, 2.0f, 16, 1);
    glPopMatrix();

    glPopMatrix();
}

void drawLowerBody() {
    glPushMatrix();
    glRotatef(bodyRotation, 0.0f, 1.0f, 0.0f);
    glScalef(1.2f, 1.2f, 1.2f);

    // Main abdomen core
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glPushMatrix();
    glScalef(2.2f, 0.8f, 1.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Abdomen details
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.82f);
    glScalef(1.4f, 0.2f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.82f);
    glScalef(1.4f, 0.2f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    // Waist
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(2.0f, 0.9f, 1.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Belt detail (red)
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(2.1f, 0.3f, 1.55f);
    drawCube1(1.0f);
    glPopMatrix();

    // Front groin armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -1.3f, 0.5f);
    glScalef(1.4f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Hip joints
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(-0.9f, -1.5f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.9f, -1.5f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    // Front center skirt (blue)
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, 0.65f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.4f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Yellow detail on front skirt
    glColor3f(1.0f, 0.85f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.8f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.8f, 0.3f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side skirts
    glColor3f(0.95f, 0.95f, 0.95f);
    // Left side skirt
    glPushMatrix();
    glTranslatef(-1.1f, -1.6f, 0.0f);
    glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.7f, 1.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right side skirt
    glPushMatrix();
    glTranslatef(1.1f, -1.6f, 0.0f);
    glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.7f, 1.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back left skirt
    glPushMatrix();
    glTranslatef(-0.6f, -1.6f, -0.6f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.7f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back right skirt
    glPushMatrix();
    glTranslatef(0.6f, -1.6f, -0.6f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.7f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Additional hip details
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(-1.3f, -1.2f, 0.0f);
    glScalef(0.4f, 0.6f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.3f, -1.2f, 0.0f);
    glScalef(0.4f, 0.6f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    glPopMatrix();
}

void drawLeftLeg() {
    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Hip joint (keeping sphere only here as it's the main joint)
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    // Upper thigh - using cube instead of cylinder
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glScalef(0.8f, 1.0f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor with additional details
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.9f, 0.6f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor details
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.4f);
    glScalef(0.7f, 0.4f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor vent
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.42f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.15f, 0.0f, 0.0f);
        glScalef(0.05f, 0.2f, 0.05f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Knee joint with rotation for movement
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glRotatef(leftKneeAngle, 1.0f, 0.0f, 0.0f);

    // Knee joint (keeping sphere only here)
    glColor3f(0.7f, 0.7f, 0.7f);
    gluSphere(gluObject, 0.4f, 16, 16);

    // Knee joint detail using cube instead of disk
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.4f);
    glScalef(0.8f, 0.8f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee armor with more detail
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glScalef(0.8f, 0.3f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee armor detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.35f);
    glScalef(0.6f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Lower shin - using cube instead of cylinder
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glScalef(0.7f, 1.0f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor with additional details
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.8f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.35f);
    glScalef(0.6f, 0.4f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor vent
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.37f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.15f, 0.0f, 0.0f);
        glScalef(0.05f, 0.2f, 0.05f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Ankle joint - using cube instead of sphere
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glScalef(0.6f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle joint detail using cube
    glColor3f(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.3f);
    glScalef(0.5f, 0.5f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Foot base with more detail
    glColor3f(0.8f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.7f, 0.2f);
    glScalef(0.7f, 0.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Foot detail
    glColor3f(0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.7f, 0.7f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe armor with more detail
    glColor3f(0.9f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.7f);
    glScalef(0.6f, 0.2f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.9f);
    glScalef(0.4f, 0.1f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe joints - using small cubes instead of spheres
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.9f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.12f, 0.0f, 0.0f);
        glScalef(0.1f, 0.1f, 0.1f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Heel armor with more detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, -0.3f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Heel detail
    glColor3f(0.5f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, -0.5f);
    glScalef(0.3f, 0.1f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side ankle armor with more detail
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(-0.5f, -1.6f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left ankle armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(-0.6f, -1.6f, 0.0f);
    glScalef(0.1f, 0.3f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, -1.6f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right ankle armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.6f, -1.6f, 0.0f);
    glScalef(0.1f, 0.3f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle thruster detail - using cone/pyramid instead of cylinder
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, -0.6f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.15f, 0.15f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Ankle thruster nozzle - using cube instead of disk
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, -0.9f);
    glScalef(0.2f, 0.2f, 0.05f);
    drawCube1(1.0f);
    glPopMatrix();

    glPopMatrix(); // End knee rotation
}

void drawRightLeg() {
    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Hip joint (keeping sphere only here as it's the main joint)
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    // Upper thigh - using cube instead of cylinder
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glScalef(0.8f, 1.0f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor with additional details
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.9f, 0.6f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor details
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.4f);
    glScalef(0.7f, 0.4f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thigh armor vent
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.42f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.15f, 0.0f, 0.0f);
        glScalef(0.05f, 0.2f, 0.05f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Knee joint with rotation for movement
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glRotatef(leftKneeAngle, 1.0f, 0.0f, 0.0f);

    // Knee joint (keeping sphere only here)
    glColor3f(0.7f, 0.7f, 0.7f);
    gluSphere(gluObject, 0.4f, 16, 16);

    // Knee joint detail using cube instead of disk
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.4f);
    glScalef(0.8f, 0.8f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee armor with more detail
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glScalef(0.8f, 0.3f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee armor detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.35f);
    glScalef(0.6f, 0.2f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Lower shin - using cube instead of cylinder
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glScalef(0.7f, 1.0f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor with additional details
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.8f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.35f);
    glScalef(0.6f, 0.4f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shin armor vent
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.37f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.15f, 0.0f, 0.0f);
        glScalef(0.05f, 0.2f, 0.05f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Ankle joint - using cube instead of sphere
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glScalef(0.6f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle joint detail using cube
    glColor3f(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.3f);
    glScalef(0.5f, 0.5f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Foot base with more detail
    glColor3f(0.8f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.7f, 0.2f);
    glScalef(0.7f, 0.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Foot detail
    glColor3f(0.6f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.7f, 0.7f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe armor with more detail
    glColor3f(0.9f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.7f);
    glScalef(0.6f, 0.2f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.9f);
    glScalef(0.4f, 0.1f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe joints - using small cubes instead of spheres
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.9f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.12f, 0.0f, 0.0f);
        glScalef(0.1f, 0.1f, 0.1f);
        drawCube1(1.0f);
        glPopMatrix();
    }
    glPopMatrix();

    // Heel armor with more detail
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, -0.3f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Heel detail
    glColor3f(0.5f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, -0.5f);
    glScalef(0.3f, 0.1f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side ankle armor with more detail
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(-0.5f, -1.6f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left ankle armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(-0.6f, -1.6f, 0.0f);
    glScalef(0.1f, 0.3f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, -1.6f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right ankle armor detail
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.6f, -1.6f, 0.0f);
    glScalef(0.1f, 0.3f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle thruster detail - using cone/pyramid instead of cylinder
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, -0.6f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.15f, 0.15f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Ankle thruster nozzle - using cube instead of disk
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, -0.9f);
    glScalef(0.2f, 0.2f, 0.05f);
    drawCube1(1.0f);
    glPopMatrix();

    glPopMatrix(); // End knee rotation
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

    glPushMatrix();
    glTranslatef(1.0f, 1.0f, 1.0f);
    //drawSword();
    glTranslatef(1.0f, 1.0f, 1.0f);
    drawShield();
    glPopMatrix();

    glPushMatrix();
    glScalef(0.5f, 0.5f, 0.5f);

    glPushMatrix();
    glTranslatef(0.0f, 3.2f, 0.0f);

    // Head 
    glPushMatrix();
    glScalef(0.8f, 0.8f, 0.8f);
    drawGundamHead();
    glPopMatrix();

    glPopMatrix();

    //Upper body
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    drawUpperBody();
    glPopMatrix();

    //Lower body 
    glPushMatrix();
    glTranslatef(0.0f, 1.1f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    drawLowerBody();
    glPopMatrix();

    //Left Arm
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

    //Right Arm
    glPushMatrix();
    glTranslatef(2.0f, 2.0f, 2.0f);
    drawRightArm();
    glPopMatrix();

    ////Left Leg 
    //glPushMatrix();
    //glTranslatef(-0.6f, 0.1f, 0.0f);
    //drawLeftLeg();
    //glPopMatrix();

    ////Right Leg
    //glPushMatrix();
    //glTranslatef(0.6f, 0.1f, 0.0f);
    //drawRightLeg();
    //glPopMatrix();

    glPopMatrix();

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
    initLighting();


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

// Polygon Function
void drawSword() {
    // Handle
    glColor3f(0.3f, 0.2f, 0.1f); // Brown
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    gluCylinder(gluObject, 0.08f, 0.08f, 0.4f, 12, 2);
    glPopMatrix();

    // Guard
    glColor3f(0.7f, 0.7f, 0.7f); // Silver
    glPushMatrix();
    glTranslatef(0, 0.4f, 0);
    glScalef(0.5f, 0.05f, 0.1f);
    drawCube();
    glPopMatrix();

    // Blade
    glColor3f(0.9f, 0.9f, 1.0f); // Steel
    glPushMatrix();
    glTranslatef(0, 0.45f, 0);
    glScalef(0.08f, 1.5f, 0.15f);
    drawPyramid(); // Use for blade tip
    // Add rectangular blade base
    glPopMatrix();
}

// faces count
// 40 faces
void drawShield() {
    // --- GEOMETRY DEFINITIONS ---

    Vec3 inner[6] = {
        {-0.25f,  1.5f,  0.0f},  // Top Left
        {-0.5f,   0.0f,  0.0f},  // Left
        {-0.25f, -1.5f,  0.0f},  // Bottom Left
        { 0.25f, -1.5f,  0.0f},  // Bottom Right
        { 0.5f,   0.0f,  0.0f},  // Right
        { 0.25f,  1.5f,  0.0f}   // Top Right
    };

    Vec3 outer[6];
    Vec3 back[6];

    float borderScale = 1.1f;
    float backScale = 1.25f;
    float thickness = -0.2f;
    float concaveDepth = -0.35f;

    for (int i = 0; i < 6; i++) {
        outer[i].x = inner[i].x * borderScale;
        outer[i].y = inner[i].y * borderScale;
        outer[i].z = -0.01f;

        back[i].x = inner[i].x * backScale;
        back[i].y = inner[i].y * backScale;
        back[i].z = thickness;
    }

    // --- DRAWING ---

    // 1. Red Face
    glColor3f(0.8f, 0.05f, 0.05f);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 6; i++) glVertex3f(inner[i].x, inner[i].y, inner[i].z);
    glEnd();

    // 2. White Border
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        glVertex3f(inner[i].x, inner[i].y, inner[i].z);
        glVertex3f(outer[i].x, outer[i].y, outer[i].z);
        glVertex3f(outer[next].x, outer[next].y, outer[next].z);
        glVertex3f(inner[next].x, inner[next].y, inner[next].z);
    }
    glEnd();

    // 3. Side Slope
    // --- New Geometry for the split side ---
    Vec3 mid[6];
    for (int i = 0; i < 6; i++) {
        mid[i].x = (outer[i].x + back[i].x) / 2.0f;
        mid[i].y = (outer[i].y + back[i].y) / 2.0f;
        mid[i].z = (outer[i].z + back[i].z) / 2.0f;
    }

    // 3. Side Slope (Front Half - Red)
    glColor3f(0.9f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        float nx = (outer[i].x + outer[next].x) / 2.0f;
        float ny = (outer[i].y + outer[next].y) / 2.0f;
        glNormal3f(nx, ny, 0.0f);

        glVertex3f(outer[i].x, outer[i].y, outer[i].z);
        glVertex3f(mid[i].x, mid[i].y, mid[i].z);
        glVertex3f(mid[next].x, mid[next].y, mid[next].z);
        glVertex3f(outer[next].x, outer[next].y, outer[next].z);
    }
    glEnd();

    // 3. Side Slope (Back Half - White)
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        float nx = (outer[i].x + outer[next].x) / 2.0f;
        float ny = (outer[i].y + outer[next].y) / 2.0f;
        glNormal3f(nx, ny, 0.0f);

        glVertex3f(mid[i].x, mid[i].y, mid[i].z);
        glVertex3f(back[i].x, back[i].y, back[i].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);
        glVertex3f(mid[next].x, mid[next].y, mid[next].z);
    }
    glEnd();

    // 4. Convex Back
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, concaveDepth);
    for (int i = 6; i >= 0; i--) {
        int idx = i % 6;
        //glColor3f(0.85f, 0.0f + i, 0.85f);
        glVertex3f(back[idx].x, back[idx].y, back[idx].z);
    }
    glEnd();

    // 5. The Convex Yellow Star / Cross
    glColor3f(1.0f, 0.9f, 0.0f);

    float zBase = 0.0f;  // Surface of shield
    float zPeak = 0.12f; // How much it sticks out (Convex height)

    // --- Vertical Cross Geometry ---
    float vx_w = 0.1f;    // Width
    float vy_top = 0.5f;  // Top Y
    float vy_bot = -0.4f; // Bottom Y

    glBegin(GL_TRIANGLES);
    // Vertical Ridge (Left Slope)
    glNormal3f(-1.0f, 0.0f, 1.0f); // Normal points Up-Left
    glVertex3f(0.0f, vy_top, zPeak);    // Ridge Top
    glVertex3f(-vx_w, vy_top, zBase);   // Edge Top
    glVertex3f(-vx_w, vy_bot, zBase);   // Edge Bottom

    glVertex3f(0.0f, vy_top, zPeak);    // Ridge Top
    glVertex3f(-vx_w, vy_bot, zBase);   // Edge Bottom
    glVertex3f(0.0f, vy_bot, zPeak);    // Ridge Bottom

    // Vertical Ridge (Right Slope)
    glNormal3f(1.0f, 0.0f, 1.0f); // Normal points Up-Right
    glVertex3f(0.0f, vy_top, zPeak);
    glVertex3f(vx_w, vy_bot, zBase);
    glVertex3f(vx_w, vy_top, zBase);

    glVertex3f(0.0f, vy_top, zPeak);
    glVertex3f(0.0f, vy_bot, zPeak);
    glVertex3f(vx_w, vy_bot, zBase);

    // Vertical Cap (Top Bevel)
    glNormal3f(0.0f, 1.0f, 1.0f);
    glVertex3f(0.0f, vy_top, zPeak);
    glVertex3f(vx_w, vy_top, zBase);
    glVertex3f(-vx_w, vy_top, zBase);

    // Vertical Cap (Bottom Bevel)
    glNormal3f(0.0f, -1.0f, 1.0f);
    glVertex3f(0.0f, vy_bot, zPeak);
    glVertex3f(-vx_w, vy_bot, zBase);
    glVertex3f(vx_w, vy_bot, zBase);
    glEnd();

    // --- Horizontal Cross Geometry ---
    float hx_left = -0.35f;
    float hx_right = 0.35f;
    float hy_w = 0.1f; // Half-height of horizontal arm
    float hy_c = 0.05f; // Center Y offset (cross is slightly higher up)

    glBegin(GL_TRIANGLES);
    // Horizontal Ridge (Top Slope)
    glNormal3f(0.0f, 1.0f, 1.0f);
    glVertex3f(hx_left, hy_c, zPeak);     // Ridge Left
    glVertex3f(hx_right, hy_c, zPeak);    // Ridge Right
    glVertex3f(hx_right, hy_c + hy_w, zBase); // Edge Right

    glVertex3f(hx_left, hy_c, zPeak);
    glVertex3f(hx_right, hy_c + hy_w, zBase);
    glVertex3f(hx_left, hy_c + hy_w, zBase);  // Edge Left

    // Horizontal Ridge (Bottom Slope)
    glNormal3f(0.0f, -1.0f, 1.0f);
    glVertex3f(hx_left, hy_c, zPeak);
    glVertex3f(hx_right, hy_c - hy_w, zBase);
    glVertex3f(hx_right, hy_c, zPeak);

    glVertex3f(hx_left, hy_c, zPeak);
    glVertex3f(hx_left, hy_c - hy_w, zBase);
    glVertex3f(hx_right, hy_c - hy_w, zBase);

    // Horizontal Cap (Left Bevel)
    glNormal3f(-1.0f, 0.0f, 1.0f);
    glVertex3f(hx_left, hy_c, zPeak);
    glVertex3f(hx_left, hy_c + hy_w, zBase);
    glVertex3f(hx_left, hy_c - hy_w, zBase);

    // Horizontal Cap (Right Bevel)
    glNormal3f(1.0f, 0.0f, 1.0f);
    glVertex3f(hx_right, hy_c, zPeak);
    glVertex3f(hx_right, hy_c - hy_w, zBase);
    glVertex3f(hx_right, hy_c + hy_w, zBase);
    glEnd();


    // 6. Back Handle
    glColor3f(0.3f, 0.3f, 0.3f);
    float hDepth = concaveDepth - 0.1f;
    float hWidth = 0.15f;
    float hHeight = 0.3f;

    glBegin(GL_QUADS);
    // Left strut
    glVertex3f(-hWidth, hHeight, concaveDepth);
    glVertex3f(-hWidth, -hHeight, concaveDepth);
    glVertex3f(-hWidth, -hHeight, hDepth);
    glVertex3f(-hWidth, hHeight, hDepth);

    // Right strut
    glVertex3f(hWidth, hHeight, hDepth);
    glVertex3f(hWidth, -hHeight, hDepth);
    glVertex3f(hWidth, -hHeight, concaveDepth);
    glVertex3f(hWidth, hHeight, concaveDepth);

    // Crossbar
    glVertex3f(-hWidth, 0.1f, hDepth);
    glVertex3f(-hWidth, -0.1f, hDepth);
    glVertex3f(hWidth, -0.1f, hDepth);
    glVertex3f(hWidth, 0.1f, hDepth);
    glEnd();
}

void drawCube(float x, float y, float z, float w, float h, float d, float r = 1.0f, float g = 1.0f, float b = 1.0f) {
    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y, z);

    // Front
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(w, 0, 0);
    glVertex3f(w, h, 0);
    glVertex3f(0, h, 0);
    glEnd();

    // Back
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    glBegin(GL_QUADS);
    glVertex3f(0, 0, d);
    glVertex3f(0, h, d);
    glVertex3f(w, h, d);
    glVertex3f(w, 0, d);
    glEnd();

    // Left
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(0, h, 0);
    glVertex3f(0, h, d);
    glVertex3f(0, 0, d);
    glEnd();

    // Right
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glBegin(GL_QUADS);
    glVertex3f(w, 0, 0);
    glVertex3f(w, 0, d);
    glVertex3f(w, h, d);
    glVertex3f(w, h, 0);
    glEnd();

    // Top
    glColor3f(r * 1.1f, g * 1.1f, b * 1.1f);
    glBegin(GL_QUADS);
    glVertex3f(0, h, 0);
    glVertex3f(w, h, 0);
    glVertex3f(w, h, d);
    glVertex3f(0, h, d);
    glEnd();

    // Bottom
    glColor3f(r * 0.7f, g * 0.7f, b * 0.7f);
    glBegin(GL_QUADS);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, d);
    glVertex3f(w, 0, d);
    glVertex3f(w, 0, 0);
    glEnd();

    glPopMatrix();
};

// Draw a cube with chamfered edges (octagon-like cross-section)
// width, height, depth are full dimensions
// chamfer is the size of the beveled edge (e.g., 0.05)
void drawChamferedCube(float width, float height, float depth, float chamfer) {
    float hw = width / 2.0f;   // half width
    float hh = height / 2.0f;  // half height
    float hd = depth / 2.0f;   // half depth

    // Define the 26 vertices of a chamfered cube
    // Each corner becomes 3 vertices, each edge becomes 2 vertices

    // Top face (y = hh)
    Vec3 topVerts[8] = {
        // Front edge
        {-hw + chamfer, hh, hd},           // 0
        {hw - chamfer, hh, hd},            // 1
        // Right edge
        {hw, hh, hd - chamfer},            // 2
        {hw, hh, -hd + chamfer},           // 3
        // Back edge
        {hw - chamfer, hh, -hd},           // 4
        {-hw + chamfer, hh, -hd},          // 5
        // Left edge
        {-hw, hh, -hd + chamfer},          // 6
        {-hw, hh, hd - chamfer}            // 7
    };

    // Bottom face (y = -hh)
    Vec3 bottomVerts[8] = {
        // Front edge
        {-hw + chamfer, -hh, hd},          // 8
        {hw - chamfer, -hh, hd},           // 9
        // Right edge
        {hw, -hh, hd - chamfer},           // 10
        {hw, -hh, -hd + chamfer},          // 11
        // Back edge
        {hw - chamfer, -hh, -hd},          // 12
        {-hw + chamfer, -hh, -hd},         // 13
        // Left edge
        {-hw, -hh, -hd + chamfer},         // 14
        {-hw, -hh, hd - chamfer}           // 15
    };

    // Draw top face (octagon)
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 8; i++) {
        glVertex3f(topVerts[i].x, topVerts[i].y, topVerts[i].z);
    }
    glEnd();

    // Draw bottom face (octagon)
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 7; i >= 0; i--) {
        glVertex3f(bottomVerts[i].x, bottomVerts[i].y, bottomVerts[i].z);
    }
    glEnd();

    // Draw 8 main side faces (large rectangles)
    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(topVerts[0].x, topVerts[0].y, topVerts[0].z);
    glVertex3f(topVerts[1].x, topVerts[1].y, topVerts[1].z);
    glVertex3f(bottomVerts[1].x, bottomVerts[1].y, bottomVerts[1].z);
    glVertex3f(bottomVerts[0].x, bottomVerts[0].y, bottomVerts[0].z);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(topVerts[2].x, topVerts[2].y, topVerts[2].z);
    glVertex3f(topVerts[3].x, topVerts[3].y, topVerts[3].z);
    glVertex3f(bottomVerts[3].x, bottomVerts[3].y, bottomVerts[3].z);
    glVertex3f(bottomVerts[2].x, bottomVerts[2].y, bottomVerts[2].z);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(topVerts[4].x, topVerts[4].y, topVerts[4].z);
    glVertex3f(topVerts[5].x, topVerts[5].y, topVerts[5].z);
    glVertex3f(bottomVerts[5].x, bottomVerts[5].y, bottomVerts[5].z);
    glVertex3f(bottomVerts[4].x, bottomVerts[4].y, bottomVerts[4].z);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(topVerts[6].x, topVerts[6].y, topVerts[6].z);
    glVertex3f(topVerts[7].x, topVerts[7].y, topVerts[7].z);
    glVertex3f(bottomVerts[7].x, bottomVerts[7].y, bottomVerts[7].z);
    glVertex3f(bottomVerts[6].x, bottomVerts[6].y, bottomVerts[6].z);

    glEnd();

    // Draw 8 small chamfer faces (corners)
    glBegin(GL_QUADS);

    // Front-right chamfer
    glNormal3f(0.707f, 0.0f, 0.707f);
    glVertex3f(topVerts[1].x, topVerts[1].y, topVerts[1].z);
    glVertex3f(topVerts[2].x, topVerts[2].y, topVerts[2].z);
    glVertex3f(bottomVerts[2].x, bottomVerts[2].y, bottomVerts[2].z);
    glVertex3f(bottomVerts[1].x, bottomVerts[1].y, bottomVerts[1].z);

    // Back-right chamfer
    glNormal3f(0.707f, 0.0f, -0.707f);
    glVertex3f(topVerts[3].x, topVerts[3].y, topVerts[3].z);
    glVertex3f(topVerts[4].x, topVerts[4].y, topVerts[4].z);
    glVertex3f(bottomVerts[4].x, bottomVerts[4].y, bottomVerts[4].z);
    glVertex3f(bottomVerts[3].x, bottomVerts[3].y, bottomVerts[3].z);

    // Back-left chamfer
    glNormal3f(-0.707f, 0.0f, -0.707f);
    glVertex3f(topVerts[5].x, topVerts[5].y, topVerts[5].z);
    glVertex3f(topVerts[6].x, topVerts[6].y, topVerts[6].z);
    glVertex3f(bottomVerts[6].x, bottomVerts[6].y, bottomVerts[6].z);
    glVertex3f(bottomVerts[5].x, bottomVerts[5].y, bottomVerts[5].z);

    // Front-left chamfer
    glNormal3f(-0.707f, 0.0f, 0.707f);
    glVertex3f(topVerts[7].x, topVerts[7].y, topVerts[7].z);
    glVertex3f(topVerts[0].x, topVerts[0].y, topVerts[0].z);
    glVertex3f(bottomVerts[0].x, bottomVerts[0].y, bottomVerts[0].z);
    glVertex3f(bottomVerts[7].x, bottomVerts[7].y, bottomVerts[7].z);

    glEnd();
}

// Example usage:
// drawChamferedCube(1.0f, 1.0f, 1.0f, 0.1f);  // 1x1x1 cube with 0.1 chamfer

void drawCenteredCube(float w, float h, float d, float r = 1.0f, float g = 1.0f, float b = 1.0f) {
    glColor3f(r, g, b);
    float hw = w / 2.0f;
    float hh = h / 2.0f;
    float hd = d / 2.0f;

    glPushMatrix();

    // Front
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(-hw, hh, hd);
    glEnd();

    // Back
    glColor3f(r * 0.8f, g * 0.8f, b * 0.8f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glEnd();

    // Left
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(-hw, hh, -hd);
    glEnd();

    // Right
    glColor3f(r * 0.9f, g * 0.9f, b * 0.9f);
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, -hh, hd);
    glEnd();

    // Top
    glColor3f(r * 1.1f, g * 1.1f, b * 1.1f);
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, hh, -hd);
    glEnd();

    // Bottom
    glColor3f(r * 0.7f, g * 0.7f, b * 0.7f);
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, -hh, hd);
    glVertex3f(-hw, -hh, hd);
    glEnd();

    glPopMatrix();
}

void drawCenteredCylinder(float radius, float height, int segments) {
    glPushMatrix();
    glTranslatef(0.0f, -height / 2.0f, 0.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, radius, radius, height, segments, 1);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, height);
    gluDisk(gluObject, 0.0f, radius, segments, 1);
    glPopMatrix();

    glRotatef(180, 1.0f, 0.0f, 0.0f);
    gluDisk(gluObject, 0.0f, radius, segments, 1);
    glPopMatrix();
}

void drawRightArm() {
    float white[3] = { 0.95f, 0.95f, 0.95f };
    float darkGrey[3] = { 0.25f, 0.25f, 0.25f };
    float red[3] = { 0.8f, 0.1f, 0.1f };

    // =================================================================
    // PART 1: SHOULDER ARMOR (Shield-like trapezoidal shape)
    // =================================================================
    glPushMatrix();
    glTranslatef(-0.4f, 0.4f, 0.0f);

    // shoulder trapezoidal (front)
    // 7 vertex
    Vec3 front[7] = {
        { 0.0f,  0.1f,  0.35f},  // Bottom left 1
        { 0.1f,  0.0f,  0.35f},  // Bottom left 2
        { 0.7f,  0.2f,  0.35f},  // Bottom Right 
        { 0.8f,  0.7f,  0.35f},   // Top Right 1
        { 0.6f,  0.9f,  0.35f},  // Top right 2
        { 0.1f,  0.9f,  0.35f},   // Top left 1
        { 0.0f,  0.8f,  0.35f}   // Top left 2
    };

    Vec3 back[7] = {
        { -0.1f,  0.0f,  0.35f},  // Bottom left 1
        { 0.1f,  -0.1f,  0.35f},  // Bottom left 2
        { 0.8f,  0.1f,  0.35f},  // Bottom Right 
        { 0.9f,  0.7f,  0.35f},   // Top Right 1
        { 0.7f,  1.0f,  0.35f},  // Top right 2
        { 0.1f,  1.0f,  0.35f},   // Top left 1
        { -0.1f,  0.8f,  0.35f}   // Top left 2
    };

    float backExtrude = 0.10f;

    for (int i = 0; i < 7; i++) {
        //back[i].x = front[i].x * scaleBack;
        //back[i].y = front[i].y * scaleBack;
        back[i].z = back[i].z - backExtrude;
    }

    // draw Front face
    glColor3fv(white);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 7; i++) glVertex3f(front[i].x, front[i].y, front[i].z);
    glEnd();

    // draw Back face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 6; i >= 0; i--) glVertex3f(back[i].x, back[i].y, back[i].z);
    glEnd();

    // Sloped sides connect front and back edges
    glColor3fv(white);
    glBegin(GL_QUADS);
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        // Calculate approximate normal for this side face
        Vec3 v1 = { back[i].x - front[i].x, back[i].y - front[i].y, back[i].z - front[i].z };
        Vec3 v2 = { front[next].x - front[i].x, front[next].y - front[i].y, front[next].z - front[i].z };

        // Cross product for normal
        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        // Normalize
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(nx, ny, nz);
        glVertex3f(front[i].x, front[i].y, front[i].z);
        glVertex3f(front[next].x, front[next].y, front[next].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);
        glVertex3f(back[i].x, back[i].y, back[i].z);
    }
    glEnd();

    // shoulder trapezoidal (back)
    // 7 vertex
    Vec3 front2[7] = {
        { 0.0f,  0.1f,  -0.35f},  // Bottom left 1
        { 0.1f,  0.0f,  -0.35f},  // Bottom left 2
        { 0.7f,  0.2f,  -0.35f},  // Bottom Right 
        { 0.8f,  0.7f,  -0.35f},   // Top Right 1
        { 0.6f,  0.9f,  -0.35f},  // Top right 2
        { 0.1f,  0.9f,  -0.35f},   // Top left 1
        { 0.0f,  0.8f,  -0.35f}   // Top left 2
    };

    Vec3 back2[7] = {
        { -0.1f,  0.0f,  -0.35f},  // Bottom left 1
        { 0.1f,  -0.1f,  -0.35f},  // Bottom left 2
        { 0.8f,  0.1f,  -0.35f},  // Bottom Right 
        { 0.9f,  0.7f,  -0.35f},   // Top Right 1
        { 0.7f,  1.0f,  -0.35f},  // Top right 2
        { 0.1f,  1.0f,  -0.35f},   // Top left 1
        { -0.1f,  0.8f,  -0.35f}   // Top left 2
    };

    //float backExtrude = 0.05f;

    for (int i = 0; i < 7; i++) {
        back2[i].z = back2[i].z + backExtrude;
    }

    // draw Front face
    glColor3fv(white);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < 7; i++) glVertex3f(front2[i].x, front2[i].y, front2[i].z);
    glEnd();

    // draw Back face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 6; i >= 0; i--) glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    glEnd();

    // Sloped sides connect front and back edges
    glColor3fv(white);
    glBegin(GL_QUADS);
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        // Calculate approximate normal for this side face
        Vec3 v1 = { back2[i].x - front2[i].x, back2[i].y - front2[i].y, back2[i].z - front2[i].z };
        Vec3 v2 = { front2[next].x - front2[i].x, front2[next].y - front2[i].y, front2[next].z - front2[i].z };

        // Cross product for normal
        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        // Normalize
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(-nx, -ny, -nz);
        glVertex3f(front2[i].x, front2[i].y, front2[i].z);
        glVertex3f(front2[next].x, front2[next].y, front2[next].z);
        glVertex3f(back2[next].x, back2[next].y, back2[next].z);
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    }
    glEnd();

    // ==================== connecting both back faces ===========================
    glColor3fv(white);
    glBegin(GL_QUADS);

    // Connect each edge between back and back2 arrays
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        // Skip the edge between vertex 3 and 4 (Top Right 1 to Top Right 2)
        if (i == 2) {
            continue; // Skip this quad to leave it open
        }

        // Calculate normal for this connecting face
        Vec3 v1 = { back2[i].x - back[i].x, back2[i].y - back[i].y, back2[i].z - back[i].z };
        Vec3 v2 = { back[next].x - back[i].x, back[next].y - back[i].y, back[next].z - back[i].z };

        // Cross product for normal
        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        // Normalize
        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(nx, ny, nz);
        glVertex3f(back[i].x, back[i].y, back[i].z);           // Current vertex on front shoulder back
        glVertex3f(back[next].x, back[next].y, back[next].z);   // Next vertex on front shoulder back
        glVertex3f(back2[next].x, back2[next].y, back2[next].z); // Next vertex on back shoulder back
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);         // Current vertex on back shoulder back
    }

    glEnd();


    // Internal connecting cylinder
    glColor3fv(darkGrey);
    glPushMatrix();
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.4f, 0.5f);
    glColor3fv(darkGrey);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    drawCenteredCylinder(0.32f, 0.5f, 20);
    glPopMatrix();

    // Decorative rings on flat faces
    glColor3fv(white);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.00f, 0.0f, 0.0f);
    //gluDisk(gluObject, 0.12f, 0.30f, 20, 1);
    drawCenteredCylinder(0.22f, 0.6f, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.26f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.22f, 0.30f, 20, 1);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix(); // End shoulder armor

    // =================================================================
    // PART 2: SHOULDER CONNECTOR
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glColor3fv(white);
    drawCenteredCube(0.50f, 0.25f, 0.50f);
    glPopMatrix();

    // =================================================================
    // PART 3: UPPER ARM
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, -0.25f, 0.0f);
    glColor3fv(white);
    drawChamferedCube(0.6f, 0.7f, 0.6f, 0.05f);
    glPopMatrix();

    // =================================================================
    // PART 4: ELBOW JOINT (Cylinder pointing left-right)
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f);

    glColor3fv(darkGrey);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    drawCenteredCylinder(0.28f, 0.7f, 20);
    glPopMatrix();

    // Decorative rings on flat faces
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.36f, 0.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.36f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    glPopMatrix();

    glPopMatrix(); // End elbow

    // =================================================================
    // PART 5: FOREARM
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glColor3fv(white);
    //drawCenteredCube(0.38f, 0.9f, 0.38f);
    drawChamferedCube(0.6f, 0.9f, 0.6f, 0.05f);
    glPopMatrix();

    // Forearm cuff detail
    glPushMatrix();
    glTranslatef(0.0f, -2.05f, 0.0f);
    glColor3fv(darkGrey);
    drawCenteredCube(0.42f, 0.12f, 0.42f);
    glPopMatrix();

    // =================================================================
    // PART 6: WRIST JOINT (Small connector)
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, -2.25f, 0.0f);
    glColor3fv(darkGrey);
    drawCenteredCube(0.32f, 0.15f, 0.32f);
    glPopMatrix();

    // =================================================================
    // PART 7: HAND (Palm + Fingers)
    // =================================================================
    glPushMatrix();
    glTranslatef(0.0f, -2.55f, 0.0f);

    // Palm base
    glColor3fv(darkGrey);
    drawCenteredCube(0.35f, 0.35f, 0.38f);

    // Finger mass (clenched fist)
    glPushMatrix();
    glTranslatef(0.0f, -0.35f, 0.0f);
    drawCenteredCube(0.32f, 0.4f, 0.35f);

    // Finger grooves (vertical lines suggesting fingers)
    glColor3f(0.15f, 0.15f, 0.15f);
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.08f, 0.0f, 0.18f);
        drawCenteredCube(0.015f, 0.38f, 0.02f);
        glPopMatrix();
    }
    glPopMatrix();

    // Thumb
    glPushMatrix();
    glTranslatef(0.25f, -0.15f, 0.1f);
    glRotatef(-25, 0.0f, 0.0f, 1.0f);
    glColor3fv(darkGrey);
    drawCenteredCube(0.12f, 0.3f, 0.18f);
    glPopMatrix();

    glPopMatrix(); // End hand
}