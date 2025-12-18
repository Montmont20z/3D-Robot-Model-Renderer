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
void drawDetailedLeftLeg();
void drawDetailedRightLeg();

void drawSword();
void drawShield();
void drawDetailedChest();
void drawDetailedAbdomen();
void drawDetailedWaist();
void drawSkirtArmor();
void drawDetailedUpperBody();
void drawDetailedLowerBody();


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

void drawWedge(float width, float height, float depth) {
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-width / 2, 0, depth / 2);
    glVertex3f(width / 2, 0, depth / 2);
    glVertex3f(width / 2, -height, depth / 2);
    glVertex3f(-width / 2, -height, depth / 2);
    // Top face
    glVertex3f(-width / 2, 0, depth / 2);
    glVertex3f(-width / 2, 0, -depth / 2);
    glVertex3f(width / 2, 0, -depth / 2);
    glVertex3f(width / 2, 0, depth / 2);
    // Bottom face
    glVertex3f(-width / 2, -height, depth / 2);
    glVertex3f(width / 2, -height, depth / 2);
    glVertex3f(0, -height, -depth / 2);
    glVertex3f(0, -height, -depth / 2);
    glEnd();
    glBegin(GL_TRIANGLES);
    // Left side
    glVertex3f(-width / 2, 0, depth / 2);
    glVertex3f(0, -height, -depth / 2);
    glVertex3f(-width / 2, 0, -depth / 2);
    // Right side
    glVertex3f(width / 2, 0, depth / 2);
    glVertex3f(width / 2, 0, -depth / 2);
    glVertex3f(0, -height, -depth / 2);
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
    glColor3f(0.95f, 0.95f, 0.95f); // Off-white
    // Central Core
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f);
    glScalef(1.8f, 1.6f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();
    // Top Crest Base
    glPushMatrix();
    glTranslatef(0.0f, 0.9f, 0.0f);
    glScalef(0.6f, 0.3f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();
    // Rear Armor
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, -0.75f);
    glScalef(1.6f, 1.4f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Pointed Chin
    glPushMatrix();
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, -0.9f, 0.0f);
    glScalef(0.8f, 1.0f, 0.8f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // --- Face Plate ---
    glPushMatrix();
    glColor3f(0.85f, 0.85f, 0.85f); // Slightly darker
    glTranslatef(0.0f, -0.1f, 0.71f);
    glScalef(1.6f, 1.0f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left Eye
    glPushMatrix();
    // Eye Socket Housing
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(-0.5f, 0.2f, 0.5f);
    glScalef(0.6f, 0.5f, 0.3f);
    drawCube1(1.0f);
    // Recessed "Lens" (a smaller, darker cube)
    glColor3f(0.05f, 0.05f, 0.05f);
    glTranslatef(0.0f, 0.0f, 0.6f); // Was 0.4f
    glScalef(0.8f, 0.8f, 0.5f);
    drawCube1(1.0f);
    // Inner "Sensor" (a bright, small cube)
    glColor3f(0.2f, 0.8f, 1.0f); // Cyan
    glTranslatef(0.0f, 0.0f, 0.7f); // Was 0.5f
    glScalef(0.5f, 0.5f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right Eye 
    glPushMatrix();
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(0.5f, 0.2f, 0.5f);
    glScalef(0.6f, 0.5f, 0.3f);
    drawCube1(1.0f);

    glColor3f(0.05f, 0.05f, 0.05f);
    glTranslatef(0.0f, 0.0f, 0.6f); // Was 0.4f
    glScalef(0.8f, 0.8f, 0.5f);
    drawCube1(1.0f);

    glColor3f(0.2f, 0.8f, 1.0f);
    glTranslatef(0.0f, 0.0f, 0.7f); // Was 0.5f
    glScalef(0.5f, 0.5f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    glColor3f(0.95f, 0.95f, 0.95f); // White
    // Central Base
    glPushMatrix();
    glTranslatef(0.0f, 1.3f, 0.0f);
    glScalef(0.2f, 0.4f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();
    // Left Blade (Pyramid)
    glPushMatrix();
    glTranslatef(-0.25f, 1.7f, 0.0f); // Positioned on top of the base
    glRotatef(-20.0f, 0.0f, 1.0f, 0.0f); // Tilted outwards
    glScalef(0.3f, 1.2f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();
    // Right Blade (Pyramid)
    glPushMatrix();
    glTranslatef(0.25f, 1.7f, 0.0f); // Positioned on top of the base
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f); // Tilted outwards
    glScalef(0.3f, 1.2f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();

    //Side Armor 
    glColor3f(0.8f, 0.8f, 0.8f); // Gray
    // Left Side Armor
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

    // Right Side Armor
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

    //Rear Sensor 
    glColor3f(0.2f, 0.2f, 0.2f); // Dark gray
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.81f);
    glScalef(0.5f, 0.3f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();
    // Sensor Pyramid
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.0f, 0.5f, -0.85f);
    glScalef(0.3f, 0.3f, 0.3f);
    drawPyramid1(1.0f);
    glPopMatrix();
}

void drawDetailedChest() {
    // Main chest frame (white)
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glScalef(2.8f, 2.2f, 1.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Central blue chest panel
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.92f);
    glScalef(1.6f, 1.4f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    // Yellow chest vents (left)
    glColor3f(1.0f, 0.85f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.7f, 0.4f, 0.96f);
    glScalef(0.35f, 0.7f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.7f, -0.3f, 0.96f);
    glScalef(0.35f, 0.5f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    // Yellow chest vents (right)
    glPushMatrix();
    glTranslatef(0.7f, 0.4f, 0.96f);
    glScalef(0.35f, 0.7f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.7f, -0.3f, 0.96f);
    glScalef(0.35f, 0.5f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    // Red triangle markers (left)
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(-1.0f, 0.1f, 0.93f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.25f, 0.25f, 0.06f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Red triangle markers (right)
    glPushMatrix();
    glTranslatef(1.0f, 0.1f, 0.93f);
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.25f, 0.25f, 0.06f);
    drawPyramid1(1.0f);
    glPopMatrix();

    // Upper chest plates
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.95f, 0.7f);
    glScalef(2.4f, 0.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shoulder mount points
    glColor3f(0.7f, 0.7f, 0.7f);
    // Left mount
    glPushMatrix();
    glTranslatef(-1.5f, 0.8f, 0.0f);
    glScalef(0.4f, 0.7f, 0.9f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right mount
    glPushMatrix();
    glTranslatef(1.5f, 0.8f, 0.0f);
    glScalef(0.4f, 0.7f, 0.9f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back detail panels
    glColor3f(0.3f, 0.3f, 0.35f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.92f);
    glScalef(2.2f, 1.6f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back vents
    glColor3f(0.2f, 0.2f, 0.25f);
    glPushMatrix();
    glTranslatef(-0.5f, 0.5f, -0.96f);
    glScalef(0.4f, 0.8f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, 0.5f, -0.96f);
    glScalef(0.4f, 0.8f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();
}

void drawDetailedAbdomen() {
    // Main abdomen core (white)
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glScalef(2.2f, 1.4f, 1.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Red central block
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(1.6f, 1.0f, 1.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // White detail strips
    glColor3f(0.95f, 0.95f, 0.95f);
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

    // Yellow abdomen vents
    glColor3f(1.0f, 0.85f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.6f, 0.0f, 0.86f);
    glScalef(0.3f, 0.5f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.6f, 0.0f, 0.86f);
    glScalef(0.3f, 0.5f, 0.04f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side hip connections
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(-1.15f, -0.2f, 0.0f);
    glScalef(0.5f, 0.9f, 1.4f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.15f, -0.2f, 0.0f);
    glScalef(0.5f, 0.9f, 1.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Hip joint spheres
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(-0.9f, -0.7f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.9f, -0.7f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();
}

void drawDetailedWaist() {
    // Main waist block
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glScalef(2.0f, 0.9f, 1.5f);
    drawCube1(1.0f);
    glPopMatrix();

    // Belt detail (red)
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(2.1f, 0.3f, 1.55f);
    drawCube1(1.0f);
    glPopMatrix();

    // Front groin armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.5f);
    glScalef(1.4f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();
}

void drawSkirtArmor() {
    // Front center skirt (blue)
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.65f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.4f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Yellow detail on front skirt
    glColor3f(1.0f, 0.85f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.1f, 0.8f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.8f, 0.3f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left front skirt (white)
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(-0.8f, -0.8f, 0.5f);
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(-5.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.6f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right front skirt (white)
    glPushMatrix();
    glTranslatef(0.8f, -0.8f, 0.5f);
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(5.0f, 0.0f, 1.0f, 0.0f);
    glScalef(0.6f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Left side skirt
    glPushMatrix();
    glTranslatef(-1.1f, -0.8f, 0.0f);
    glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.7f, 1.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right side skirt
    glPushMatrix();
    glTranslatef(1.1f, -0.8f, 0.0f);
    glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.7f, 1.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back left skirt
    glPushMatrix();
    glTranslatef(-0.6f, -0.8f, -0.6f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.7f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back right skirt
    glPushMatrix();
    glTranslatef(0.6f, -0.8f, -0.6f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.7f, 1.2f, 0.12f);
    drawCube1(1.0f);
    glPopMatrix();
}

// Enhanced upper body with more details
void drawDetailedUpperBody() {
    // Main chest frame (white)
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glScalef(2.8f, 2.2f, 1.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Central blue chest panel
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.92f);
    glScalef(1.6f, 1.4f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    // Additional chest details
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.9f);
    glScalef(1.2f, 0.3f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Shoulder armor details
    glColor3f(0.7f, 0.7f, 0.7f);
    // Left shoulder detail
    glPushMatrix();
    glTranslatef(-1.8f, 0.8f, 0.0f);
    glScalef(0.6f, 0.4f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Right shoulder detail
    glPushMatrix();
    glTranslatef(1.8f, 0.8f, 0.0f);
    glScalef(0.6f, 0.4f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Chest vents (enhanced)
    glColor3f(1.0f, 0.85f, 0.0f);
    // Left vents
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(-0.8f, 0.6f - i * 0.3f, 0.96f);
        glScalef(0.3f, 0.2f, 0.04f);
        drawCube1(1.0f);
        glPopMatrix();
    }

    // Right vents
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0.8f, 0.6f - i * 0.3f, 0.96f);
        glScalef(0.3f, 0.2f, 0.04f);
        drawCube1(1.0f);
        glPopMatrix();
    }

    // Back armor details
    glColor3f(0.4f, 0.4f, 0.45f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -0.92f);
    glScalef(2.2f, 1.6f, 0.08f);
    drawCube1(1.0f);
    glPopMatrix();

    // Back thrusters
    glColor3f(0.2f, 0.2f, 0.25f);
    glPushMatrix();
    glTranslatef(-0.7f, 0.0f, -0.96f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluCylinder(gluObject, 0.15f, 0.1f, 0.5f, 8, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.7f, 0.0f, -0.96f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluCylinder(gluObject, 0.15f, 0.1f, 0.5f, 8, 1);
    glPopMatrix();
}

// Enhanced lower body with more details
void drawDetailedLowerBody() {
    // Main abdomen core (white)
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glScalef(2.2f, 1.4f, 1.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Red central block
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    glScalef(1.6f, 1.0f, 1.4f);
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

    // Skirt armor (enhanced)
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

    // Back skirts
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
}

// Detailed left leg with multiple segments
void drawDetailedLeftLeg() {
    // Hip joint
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    // Upper thigh
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.4f, 0.35f, 1.0f, 16, 4);
    glPopMatrix();

    // Thigh armor
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.9f, 0.6f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee joint
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    gluSphere(gluObject, 0.4f, 16, 16);
    glPopMatrix();

    // Knee armor
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glScalef(0.8f, 0.3f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Lower shin
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.35f, 0.3f, 1.0f, 16, 4);
    glPopMatrix();

    // Shin armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -2.3f, 0.0f);
    glScalef(0.8f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle joint
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, -3.0f, 0.0f);
    gluSphere(gluObject, 0.3f, 16, 16);
    glPopMatrix();

    // Foot base
    glColor3f(0.8f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.2f, 0.2f);
    glScalef(0.7f, 0.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe armor
    glColor3f(0.9f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.4f, 0.7f);
    glScalef(0.6f, 0.2f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Heel armor
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.4f, -0.3f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side ankle armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(-0.5f, -3.1f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, -3.1f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();
}

// Detailed right leg with multiple segments
void drawDetailedRightLeg() {
    // Hip joint
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    gluSphere(gluObject, 0.35f, 16, 16);
    glPopMatrix();

    // Upper thigh
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.4f, 0.35f, 1.0f, 16, 4);
    glPopMatrix();

    // Thigh armor
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(0.9f, 0.6f, 0.7f);
    drawCube1(1.0f);
    glPopMatrix();

    // Knee joint
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    gluSphere(gluObject, 0.4f, 16, 16);
    glPopMatrix();

    // Knee armor
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -1.5f, 0.0f);
    glScalef(0.8f, 0.3f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Lower shin
    glColor3f(0.95f, 0.95f, 0.95f);
    glPushMatrix();
    glTranslatef(0.0f, -2.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.35f, 0.3f, 1.0f, 16, 4);
    glPopMatrix();

    // Shin armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -2.3f, 0.0f);
    glScalef(0.8f, 0.6f, 0.6f);
    drawCube1(1.0f);
    glPopMatrix();

    // Ankle joint
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.0f, -3.0f, 0.0f);
    gluSphere(gluObject, 0.3f, 16, 16);
    glPopMatrix();

    // Foot base
    glColor3f(0.8f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.2f, 0.2f);
    glScalef(0.7f, 0.3f, 1.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Toe armor
    glColor3f(0.9f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.4f, 0.7f);
    glScalef(0.6f, 0.2f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    // Heel armor
    glColor3f(0.7f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -3.4f, -0.3f);
    glScalef(0.5f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Side ankle armor
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(-0.5f, -3.1f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, -3.1f, 0.0f);
    glScalef(0.2f, 0.4f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();
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
    drawSword();
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

    // Antenna
    glColor3f(1.0f, 0.9f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.45f, 0.0f); // On top
    glScalef(1.5f, 0.4f, 0.1f);
    drawPyramid();
    glPopMatrix();

    glPopMatrix();

    // Upper body (chest)
    glPushMatrix();
    glTranslatef(0.0f, 2.0f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    drawDetailedUpperBody();
    glPopMatrix();

    // Lower body (abdomen, waist, skirt)
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    drawDetailedLowerBody();
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
    drawRightArm();

    // Left Leg (detailed)
    //glPushMatrix();
    //glTranslatef(-0.4f, 0.6f, 0.0f);
    //drawDetailedLeftLeg();
    //glPopMatrix();

    //// Right Leg (detailed)
    //glPushMatrix();
    //glTranslatef(0.4f, 0.6f, 0.0f);
    //drawDetailedRightLeg();
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

    // --- Vertical Arm Geometry ---
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

    // --- Horizontal Arm Geometry ---
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

void drawRightArm()
{
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
}