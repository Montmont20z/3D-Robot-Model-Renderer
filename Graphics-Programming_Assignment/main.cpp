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


float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;

//  ---------------- Robot variable ----------------------
float leftHipAngle = 0.0f;
float leftKneeAngle = 0.0f;
float rightHipAngle = 0.0f;
float rightKneeAngle = 0.0f;
float headRotation = 0.0f;
float bodyRotation = 0.0f;
float robotRotateX = 0.0f;
float robotRotateY = 0.0f;
float robotRotateZ = 0.0f;

//Sword Animation
float bladeThick = 0.0f;
float enlargeSpeed = 0.0001f;
bool isEnlarging = false;

// ----------------- Texture ------------------------------
GLuint metalWhiteTexture1 = 0;
GLuint metalWhiteTexture2 = 0;
GLuint metalWhiteTexture3 = 0;
GLuint metalTexture = 0;

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

enum ProjectMode { ORTHO = 0, PERSPECTIVE = 1, FRUSTUM = 2 };
ProjectMode projMode = ORTHO;

float fovy = 45.0f;
float zNear = 0.1f, zFar = 20.0f;

void updateProjection(int width, int height);

void drawLeftHand();
void drawRightHand();
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
    {
        // Check modifier keys
        bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

        switch (wParam)
        {
            // robot control
            // hand controls
            // left hand
        case 'R':
            if (isShiftPressed) leftShoulderYawAngle -= 5.0f;
            else leftShoulderYawAngle += 5.0f;
            break;
        case 'T': 
            if (isShiftPressed) leftShoulderPitchAngle -= 5.0f;
            else leftShoulderPitchAngle += 5.0f;
            break;
        case 'Y': 
            if (isShiftPressed) leftElbowAngle -= 5.0f;
            else leftElbowAngle += 5.0f;
            break;
        case 'U': 
            if (isShiftPressed) leftWristAngle -= 5.0f;
            else leftWristAngle += 5.0f;
            break;
        case 'I':
            if (isShiftPressed) leftFingersCurlAngle -= 5.0f;
            else leftFingersCurlAngle += 5.0f;
            break;

            // right hand
		case 'F':
            if (isShiftPressed) rightShoulderYawAngle -= 5.0f;
            else rightShoulderYawAngle += 5.0f;
            break;
		case 'G':
            if (isShiftPressed) rightShoulderPitchAngle -= 5.0f;
            else rightShoulderPitchAngle += 5.0f;
            break;
        case 'H': 
            if (isShiftPressed) rightElbowAngle -= 5.0f;
            else rightElbowAngle += 5.0f;
            break;
        case 'J': 
            if (isShiftPressed) rightWristAngle -= 5.0f;
            else rightWristAngle += 5.0f;
            break;
        case 'K':
            if (isShiftPressed) rightFingersCurlAngle -= 5.0f;
            else rightFingersCurlAngle += 5.0f;
            break;
        case 'B': // press 'B' to trigger Block animation
            startBlock();
            break;

        case 'A': headRotation += 5.0f; break;
        case 'D': headRotation -= 5.0f; break;
        case 'W': bodyRotation += 5.0f; break;
        case 'S': bodyRotation -= 5.0f; break;


        case 'l':
		case 'L': 
            isEnlarging = !isEnlarging;
			break;
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
        case '1': robotRotateX += 5.0f; break;
        case '2': robotRotateX -= 5.0f; break;
        case '3': robotRotateY += 5.0f; break;
        case '4': robotRotateY -= 5.0f; break;
        case '5': robotRotateZ += 5.0f; break;
        case '6': robotRotateZ -= 5.0f; break;
        default:
            break;
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}
//--------------------------- Initialization -----------------------------------------
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
    GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
}

void initTexture() {
	metalWhiteTexture1 = LoadBMPTexture("metal-white-1.bmp");
	metalWhiteTexture2 = LoadBMPTexture("metal-white-2.bmp");
	metalWhiteTexture3 = LoadBMPTexture("metal-white-3.bmp");
    metalTexture = LoadBMPTexture("metal.bmp");

}

//--------------------------------------------------------------------

void UpdateSword() {
    if (isEnlarging) {
		bladeThick += enlargeSpeed;
        if (bladeThick >= 0.03f) {
            bladeThick = 0.03f;
        }
    }
    else {
        bladeThick -= enlargeSpeed;
        if (bladeThick == 0.0f) {
            bladeThick = 0.0f;
        }
    }
}

void Display(HWND hWnd)
{
    // update animations
    updateBlockAnim(now_seconds());

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
   
    glRotatef(robotRotateX, 1.0f, 0.0f, 0.0f); 
    glRotatef(robotRotateY, 0.0f, 1.0f, 0.0f); 
    glRotatef(robotRotateZ, 0.0f, 0.0f, 1.0f);

    glPushMatrix();
    glTranslatef(1.0f, 0.0f, 1.0f);
    drawSword();
    glTranslatef(1.0f, 1.0f, 1.0f);
    //drawShield();
    glPopMatrix();

    glPushMatrix();
    glScalef(0.5f, 0.5f, 0.5f);

    glPushMatrix();
    glTranslatef(0.0f, 3.2f, 0.0f);

    // Head 
    glPushMatrix();
    glScalef(0.65f, 0.65f, 0.65f);
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
    glTranslatef(0.0f, 0.32f, 0.0f);
    glScalef(0.55f, 0.55f, 0.55f);
    drawLowerBody();
    glPopMatrix();

    //right Hand
    glPushMatrix();
    glTranslatef(2.2f, 1.0f, 0.0f);
    drawLeftHand();
    glPopMatrix();

    //left Hand
    glPushMatrix();
    glTranslatef(-2.2f, 1.0f, 0.0f);
    drawRightHand();
    glPopMatrix();

    //Left Leg 
    glPushMatrix();
    glTranslatef(-0.9f, -0.9f, 0.0f);
    drawLeftLeg();
    glPopMatrix();

    //Right Leg
    glPushMatrix();
    glTranslatef(0.3f, -0.9f, 0.0f);
    drawRightLeg();
    glPopMatrix();

    glPopMatrix();

}


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
    initTexture();


    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		UpdateSword();
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

    glPopMatrix(); 
}

void drawUpperBody()
{
    glPushMatrix();
    glRotatef(bodyRotation, 0.0f, 1.0f, 0.0f);
    glScalef(2.2f, 2.2f, 2.2f);

    // Main Chest Block
    glColor3f(0.0f, 0.35f, 0.7f);
    glPushMatrix();
    glScalef(2.4f, 1.3f, 1.0f);
    drawCube1(1.0f);
    glPopMatrix();

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.3f, 0.65f);

    //Front Panel
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.55f, 0.65f, 0.9f);
    glVertex3f(0.55f, 0.65f, 0.9f);
    glVertex3f(0.55f, -0.65f, 0.9f);
    glVertex3f(-0.55f, -0.65f, 0.9f);

    //Left Armor Side
    glNormal3f(-0.8f, 0.0f, 0.6f);
    glVertex3f(-0.55f, 0.65f, 0.9f);
    glVertex3f(-1.2f, 0.65f, 0.5f);
    glVertex3f(-1.2f, -0.65f, 0.5f);
    glVertex3f(-0.55f, -0.65f, 0.9f);

    //Right Armor Side 
    glNormal3f(0.8f, 0.0f, 0.6f);
    glVertex3f(0.55f, 0.65f, 0.9f);
    glVertex3f(0.55f, -0.65f, 0.9f);
    glVertex3f(1.2f, -0.65f, 0.5f);
    glVertex3f(1.2f, 0.65f, 0.5f);

    //Top Cover
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.55f, 0.65f, 0.9f);
    glVertex3f(0.55f, 0.65f, 0.9f);
    glVertex3f(1.2f, 0.65f, 0.5f);
    glVertex3f(-1.2f, 0.65f, 0.5f);

    //Bottom Cover
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-0.55f, -0.65f, 0.9f);
    glVertex3f(-1.2f, -0.65f, 0.5f);
    glVertex3f(1.2f, -0.65f, 0.5f);
    glVertex3f(0.55f, -0.65f, 0.9f);
    glEnd();

    //Red Cockpit Cover
    glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.3f, 1.0f);
    glScalef(0.55f, 0.65f, 0.2f);
    drawCube1(1.0f);
    glPopMatrix();

    // Add frame to cockpit
    glColor3f(0.6f, 0.05f, 0.05f); // Darker red for frame
    glLineWidth(4.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-0.275f, 0.025f, 1.1f);
    glVertex3f(0.275f, 0.025f, 1.1f);
    glVertex3f(0.275f, -0.625f, 1.1f);
    glVertex3f(-0.275f, -0.625f, 1.1f);
    glEnd();

    //Yellow Vents 
    glColor3f(1.0f, 0.8f, 0.0f);
    float ventY[] = { 0.35f, 0.15f, -0.05f };
    for (int i = 0; i < 3; i++) {
        // Left Side Vents
        glPushMatrix();
        glTranslatef(-0.95f, ventY[i], 0.72f);
        glRotatef(-32, 0, 1, 0);
        glScalef(0.55f, 0.12f, 0.1f);
        drawCube1(1.0f);
        glPopMatrix();

        // Right Side Vents
        glPushMatrix();
        glTranslatef(0.95f, ventY[i], 0.72f);
        glRotatef(32, 0, 1, 0);
        glScalef(0.55f, 0.12f, 0.1f);
        drawCube1(1.0f);
        glPopMatrix();
    }

    //Abdomen Red Layering
    glColor3f(0.8f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f);
    glScalef(2.0f, 0.2f, 1.0f); // First Layer
    drawCube1(1.0f);
    glPopMatrix();

    glPopMatrix();
}

void drawLowerBody() {
    glPushMatrix();
    glRotatef(bodyRotation, 0.0f, 1.0f, 0.0f);
    glScalef(1.4f, 1.4f, 1.4f);  

    // Main abdomen core 
    glColor3f(0.95f, 0.95f, 0.95f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glPushMatrix();
    glScalef(2.6f, 0.9f, 1.55f); 
    drawCube1(1.0f);
    glPopMatrix();

    // Abdomen details 
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.85f);
    glScalef(1.8f, 0.25f, 0.09f); 
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -0.3f, 0.85f);
    glScalef(1.8f, 0.25f, 0.09f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Waist 
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.95f, 0.0f);
    glScalef(2.4f, 1.0f, 1.6f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Belt detail 
    glColor3f(0.9f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.0f);
    glScalef(2.5f, 0.35f, 1.65f);  
    drawCube1(1.0f);
    glPopMatrix();

    glColor3f(1.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.8f, 0.85f);
    glScalef(0.5f, 0.3f, 0.1f);
    drawCube1(1.0f);
    glPopMatrix();

    // Front groin armor 
    glColor3f(0.85f, 0.85f, 0.85f);
    glPushMatrix();
    glTranslatef(0.0f, -1.3f, 0.55f);
    glScalef(1.8f, 0.25f, 0.35f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Front center skirt 
    glColor3f(0.1f, 0.3f, 0.9f);
    glPushMatrix();
    glTranslatef(0.0f, -1.6f, 0.68f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.8f, 1.3f, 0.13f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Yellow detail on front skirt 
    glColor3f(1.0f, 0.85f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, -1.9f, 0.82f);
    glRotatef(-20.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.2f, 0.35f, 0.11f); 
    drawCube1(1.0f);
    glPopMatrix();

    // Side skirts
    glColor3f(0.95f, 0.95f, 0.95f);
    // Left side skirt
    glPushMatrix();
    glTranslatef(-1.3f, -1.6f, 0.0f);  
    glRotatef(-10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.8f, 1.4f, 1.3f); 
    drawCube1(1.0f);
    glPopMatrix();

    // Right side skirt
    glPushMatrix();
    glTranslatef(1.3f, -1.6f, 0.0f);  
    glRotatef(10.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.8f, 1.4f, 1.3f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Back left skirt 
    glPushMatrix();
    glTranslatef(-0.8f, -1.6f, -0.65f); 
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.8f, 1.3f, 0.13f); 
    drawCube1(1.0f);
    glPopMatrix();

    // Back right skirt 
    glPushMatrix();
    glTranslatef(0.8f, -1.6f, -0.65f);   
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.8f, 1.3f, 0.13f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Additional hip details
    glColor3f(0.7f, 0.7f, 0.7f);
    glPushMatrix();
    glTranslatef(-1.5f, -1.3f, 0.0f);  
    glScalef(0.45f, 0.7f, 0.9f);  
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.5f, -1.3f, 0.0f);  
    glScalef(0.45f, 0.7f, 0.9f);  
    drawCube1(1.0f);
    glPopMatrix();

    // Back thrusters
    glColor3f(0.2f, 0.2f, 0.2f);
    glPushMatrix();
    glTranslatef(-0.7f, -1.0f, -0.8f);
    glScalef(0.4f, 0.6f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.7f, -1.0f, -0.8f);
    glScalef(0.4f, 0.6f, 0.8f);
    drawCube1(1.0f);
    glPopMatrix();

    // Thruster details
    glColor3f(0.1f, 0.1f, 0.8f);
    glPushMatrix();
    glTranslatef(-0.7f, -1.0f, -1.2f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.15f, 0.25f, 0.4f, 8, 1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.7f, -1.0f, -1.2f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.15f, 0.25f, 0.4f, 8, 1);
    glPopMatrix();

    // Back sensor array
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0.0f, -0.5f, -0.9f);
    glScalef(0.8f, 0.2f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    // Sensor details
    glColor3f(0.1f, 0.8f, 0.1f);
    glPushMatrix();
    glTranslatef(-0.3f, -0.5f, -1.05f);
    gluSphere(gluObject, 0.08f, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -0.5f, -1.05f);
    gluSphere(gluObject, 0.08f, 8, 8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.3f, -0.5f, -1.05f);
    gluSphere(gluObject, 0.08f, 8, 8);
    glPopMatrix();

    glPopMatrix();
}

void drawLeftLeg() {
    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Hip joint 
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.3f, -0.1f, 0.0f);
    gluSphere(gluObject, 0.25f, 16, 16);
    glPopMatrix();

    //Front Upper Thigh (Inner)
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.3f, -0.7f, 0.15f);
    glScalef(0.6f, 1.0f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Upper Thigh (Inner
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.35f, -0.2f);
    glScalef(0.6f, 0.3f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Light
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.55f, -0.2f);
    glScalef(0.6f, 0.1f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Lower Thigh (Inner)
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.3f, -0.9f, -0.2f);
    glScalef(0.6f, 0.6f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Upper Thigh Armor

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.7f, 0.5f);
    drawChamferedCube(0.8f, 1.0f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(-0.1f, -0.6f, 0.05f);
    drawChamferedCube(0.1f, 0.8f, 0.9f, 0.05f);
    glPopMatrix();

    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0.7f, -0.6f, 0.05f);
    drawChamferedCube(0.1f, 0.8f, .9f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.6f, -0.45f);
    glRotatef(90.0, 1.0f, 0.0f, 0.0f);
    drawTaperedCube(0.8f, 0.8f, 0.7f, 0.5f, 0.2f);
    glPopMatrix();

    glTranslatef(0.3f, -1.3f, 0.0f);
    glRotatef(leftKneeAngle, 1.0f, 0.0f, 0.0f); // Bend backward
    glTranslatef(-0.3f, 1.3f, 0.0f);

    //Joint
    glColor3f(0.75f, 0.75f, 0.75f);
    glPushMatrix();
    glTranslatef(0.3f, -1.3f, -0.0f);
    glRotatef(90.0f, 0.0f, .0f, 1.0f);
    drawCenteredCylinder(0.38f, 0.7f, 10);
    glPopMatrix();

    //Lower leg
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.0f, -0.0f);
    glScalef(0.5f, 2.2f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    //Lower Leg Armor
	//Right Side
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.7f, -2.2f, -0.0f);
    drawChamferedCube(0.1f,1.6f, 0.8f, 0.05f);
    glPopMatrix();

    glColor3f(.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.75f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.5f, 0.6f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.8f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

	//Left Side
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.1f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.6f, 0.8f, 0.05f);
    glPopMatrix();

    glColor3f(.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.15f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.5f, 0.6f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

	//Front
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.4f);
    drawChamferedCube(0.8f, 1.6f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.45f);
    drawChamferedCube(0.6f, 1.5f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.0f, 0.4f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.4f);
    drawChamferedCube(0.1f, 0.8f, 0.4f, 0.05f);
    glPopMatrix();


    //Back
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, -0.35f);
	glRotatef(5.0f, -1.0f, 0.0f, 0.0f);
    drawChamferedCube(0.8f, 1.6f, 0.1f, 0.05f);
    glPopMatrix();


    //Booster
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, -0.4f);
	glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -1.6f, -0.45f);
	glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.8f, -0.35f);
	glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    //Leg Ring
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.15f, -0.0f);
    drawCenteredCylinder(0.2f, 0.1f, 10);
    glPopMatrix();

    //Foot Connector
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -3.2f, -0.0f);
    drawCenteredCylinder(0.1f, 0.3f, 16);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -3.35f, -0.0f);
    gluSphere(gluObject, 0.15f, 16, 16);
    glPopMatrix();

    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.4f, -0.0f);
    drawCenteredCylinder(0.25f, 0.2f, 16);
    glPopMatrix();

    //Foot base
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.55f, 0.3f);
    glScalef(0.65f, 0.335f, 1.5f);
    drawCube1(1.0f);
    glPopMatrix();

    //Foot front
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.65f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.2f, 0.7f, -0.2f);
    glScalef(0.45f, 0.7f, 0.45f);
    drawPyramid1(1.0f);
    glPopMatrix();

    //heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.3f, -0.5f);
    drawChamferedCube(0.75, 0.45, 0.1, 0.05);
    glPopMatrix();

    //side Heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.65f, -3.3f, -0.15f);
    drawChamferedCube(0.1, 0.45, 0.75, 0.05);
    glPopMatrix();

    //side Heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.05f, -3.3f, -0.15f);
    drawChamferedCube(0.1, 0.45, 0.75, 0.05);
    glPopMatrix();

}

void drawRightLeg() {
    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Apply hip rotation for movement
    glRotatef(leftHipAngle, 1.0f, 0.0f, 0.0f);

    // Hip joint 
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.3f, -0.1f, 0.0f);
    gluSphere(gluObject, 0.25f, 16, 16);
    glPopMatrix();

    //Front Upper Thigh (Inner)
    glColor3f(0.8f, 0.8f, 0.8f);
    glPushMatrix();
    glTranslatef(0.3f, -0.7f, 0.15f);
    glScalef(0.6f, 1.0f, 0.4f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Upper Thigh (Inner
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.35f, -0.2f);
    glScalef(0.6f, 0.3f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Light
    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.55f, -0.2f);
    glScalef(0.6f, 0.1f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Back Lower Thigh (Inner)
    glColor3f(0.6f, 0.6f, 0.6f);
    glPushMatrix();
    glTranslatef(0.3f, -0.9f, -0.2f);
    glScalef(0.6f, 0.6f, 0.3f);
    drawCube1(1.0f);
    glPopMatrix();

    //Upper Thigh Armor

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.7f, 0.5f);
    drawChamferedCube(0.8f, 1.0f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(-0.1f, -0.6f, 0.05f);
    drawChamferedCube(0.1f, 0.8f, 0.9f, 0.05f);
    glPopMatrix();

    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0.7f, -0.6f, 0.05f);
    drawChamferedCube(0.1f, 0.8f, .9f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -0.6f, -0.45f);
    glRotatef(90.0, 1.0f, 0.0f, 0.0f);
    drawTaperedCube(0.8f, 0.8f, 0.7f, 0.5f, 0.2f);
    glPopMatrix();

    //Joint
    glColor3f(0.75f, 0.75f, 0.75f);
    glPushMatrix();
    glTranslatef(0.3f, -1.3f, -0.0f);
    glRotatef(90.0f, 0.0f, .0f, 1.0f);
    drawCenteredCylinder(0.38f, 0.7f, 10);
    glPopMatrix();


    //Lower leg
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.0f, -0.0f);
    glScalef(0.5f, 2.2f, 0.5f);
    drawCube1(1.0f);
    glPopMatrix();

    //Lower Leg Armor
    //Right Side
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.7f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.6f, 0.8f, 0.05f);
    glPopMatrix();

    glColor3f(.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.75f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.5f, 0.6f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.8f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

    //Left Side
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.1f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.6f, 0.8f, 0.05f);
    glPopMatrix();

    glColor3f(.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(-0.15f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 1.5f, 0.6f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.2f, -2.2f, -0.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

    //Front
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.4f);
    drawChamferedCube(0.8f, 1.6f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.45f);
    drawChamferedCube(0.6f, 1.5f, 0.1f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.0f, 0.4f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    drawChamferedCube(0.1f, 0.4f, 0.4f, 0.05f);
    glPopMatrix();

    glColor3f(1.0f, 1.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, 0.4f);
    drawChamferedCube(0.1f, 0.8f, 0.4f, 0.05f);
    glPopMatrix();


    //Back
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, -0.35f);
    glRotatef(5.0f, -1.0f, 0.0f, 0.0f);
    drawChamferedCube(0.8f, 1.6f, 0.1f, 0.05f);
    glPopMatrix();


    //Booster
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.2f, -0.4f);
    glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -1.6f, -0.45f);
    glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -2.8f, -0.35f);
    glRotatef(175.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, 0.1f, 0.25f, 0.3f, 6, 1);
    glPopMatrix();

    //Leg Ring
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.15f, -0.0f);
    drawCenteredCylinder(0.2f, 0.1f, 10);
    glPopMatrix();

    //Foot Connector
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -3.2f, -0.0f);
    drawCenteredCylinder(0.1f, 0.3f, 16);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0.3f, -3.35f, -0.0f);
    gluSphere(gluObject, 0.15f, 16, 16);
    glPopMatrix();

    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.4f, -0.0f);
    drawCenteredCylinder(0.25f, 0.2f, 16);
    glPopMatrix();

    //Foot base
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.55f, 0.3f);
    glScalef(0.65f, 0.335f, 1.5f);
    drawCube1(1.0f);
    glPopMatrix();

    //Foot front
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.65f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.2f, 0.7f, -0.2f);
    glScalef(0.45f, 0.7f, 0.45f);
    drawPyramid1(1.0f);
    glPopMatrix();

    //heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.3f, -3.3f, -0.5f);
    drawChamferedCube(0.75, 0.45, 0.1, 0.05);
    glPopMatrix();

    //side Heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.65f, -3.3f, -0.15f);
    drawChamferedCube(0.1, 0.45, 0.75, 0.05);
    glPopMatrix();

    //side Heel
    glColor3f(1.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(-0.05f, -3.3f, -0.15f);
    drawChamferedCube(0.1, 0.45, 0.75, 0.05);
    glPopMatrix();
}

 //Polygon Function
void drawSword() {
    // Handle
    glColor3f(1.0f, 1.0f, 1.0f);  //White
    glPushMatrix();
    glTranslatef(0.1f, 0.6f, 0.2f);
	drawCenteredCylinder(0.0325f, 0.25f, 12);
    glPopMatrix();

    // lower hilt
    glColor3f(0.8f, 0.8f, 0.8f);  //White
    glPushMatrix();
    glTranslatef(0.1f, 0.45f, 0.2f);
    drawCenteredCylinder(0.04f, 0.05f, 8);
    glPopMatrix();

    // lower stud
    glColor3f(1.0f, 1.0f, 1.0f);  //White
    glPushMatrix();
    glTranslatef(0.1f, 0.405f, 0.2f);
    drawCenteredCylinder(0.0325f, 0.04f, 12);
    glPopMatrix();

    // upper hilt
    glColor3f(0.8f, 0.8f, 0.8f);  //White
    glPushMatrix();
    glTranslatef(0.1f, 0.75f, 0.2f);
    drawCenteredCylinder(0.05f, 0.05f, 8);
    glPopMatrix();

	// upper stud
    glColor3f(1.0f, 1.0f, 1.0f);  //White
    glPushMatrix();
    glTranslatef(0.1f, 0.8f, 0.2f);
    drawCenteredCylinder(0.0325f, 0.04f, 12);
    glPopMatrix();


    // Blade
    glColor3f(1.0f, 0.1f, 0.9f); // light
    glPushMatrix();
    glTranslatef(0.1f, 1.75f, 0.2f);
	drawCenteredCylinder(bladeThick, 1.9f, 6);    
    glPopMatrix();

    //Blade tip
    glColor3f(1.0f, 0.1f, 0.9f); // light
    glPushMatrix();
    glTranslatef(0.1f, 2.7f, 0.2f);
	glRotatef(-90.0, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluObject, bladeThick, 0.0f, 0.02f, 20, 20);
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
    glScalef(2.2f, 1.6f, 1.0f);

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
    glPushMatrix(); // Start Star
    glTranslatef(0, -0.2f, 0);

    float zBase = 0.0f;  // Surface of shield
    float zPeak = 0.12f; // How much it sticks out (Convex height)

    // --- Vertical Cross Geometry ---
    float vx_w = 0.05f;    // Width
    float vy_top = 0.5f;  // Top Y
    float vy_bot = -1.0f; // Bottom Y

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
    float hy_w = 0.05f; // Half-height of horizontal arm
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

    glPopMatrix(); // End Star


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

// faces count
// 105
void drawLeftHand()
{
    float white[3] = { 0.95f, 0.95f, 0.95f };
    float darkGrey[3] = { 0.25f, 0.25f, 0.25f };
    float red[3] = { 0.8f, 0.1f, 0.1f };

    // enable texture 
	glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, metalWhiteTexture1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // or GL_REPLACE
    gluQuadricTexture(gluObject, GL_TRUE); // auto-generates texcoords for quadrics (spheres/cylinders/disks)
    //gluQuadricDrawStyle(gluObject, GLU_FILL);

    // ---------------------------
    // ARM HIERARCHY (rooted at the shoulder pivot)
    //   glPushMatrix() here marks "entire-arm movement" region
    // ---------------------------
    glPushMatrix();
    // Move origin to shoulder pivot (same location as the shoulder armor)
    glTranslatef(-0.4f, 0.4f, 0.0f);
  

    // Whole-arm rotations (shoulder-level)
    glRotatef(leftShoulderYawAngle, 0.0f, 1.0f, 0.0f);    // yaw around Y
    glRotatef(leftShoulderPitchAngle, 1.0f, 0.0f, 0.0f);  // pitch around X

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
    for (int i = 0; i < 7; i++) {
        glTexCoord2f(front[i].x,front[i].y);
        glVertex3f(front[i].x, front[i].y, front[i].z);
    }
    glEnd();

    // draw Back face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 6; i >= 0; i--) {
        glTexCoord2f(back[i].x,back[i].y);
        glVertex3f(back[i].x, back[i].y, back[i].z);
    }
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
        glTexCoord2f(front[i].x,front[i].z);
        glVertex3f(front[i].x, front[i].y, front[i].z);
        glTexCoord2f(front[next].x,front[next].z);
        glVertex3f(front[next].x, front[next].y, front[next].z);
        glTexCoord2f(back[next].x,back[next].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);
        glTexCoord2f(back[i].x,back[i].z);
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
    for (int i = 0; i < 7; i++) {
        glTexCoord2f(front2[i].x,front2[i].y);
        glVertex3f(front2[i].x, front2[i].y, front2[i].z);
    }
    glEnd();

    // draw Back face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 6; i >= 0; i--) {
        glTexCoord2f(back2[i].x,back2[i].y);
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    }
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
        glTexCoord2f(front2[i].x,front2[i].z);
        glVertex3f(front2[i].x, front2[i].y, front2[i].z);
        glTexCoord2f(front2[next].x,front2[next].z);
        glVertex3f(front2[next].x, front2[next].y, front2[next].z);
        glTexCoord2f(back2[next].x,back2[next].z);
        glVertex3f(back2[next].x, back2[next].y, back2[next].z);
        glTexCoord2f(back2[i].x,back2[i].z);
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
        glTexCoord2f(back[i].x,back[i].z);
        glVertex3f(back[i].x, back[i].y, back[i].z);           // Current vertex on front shoulder back
        glTexCoord2f(back[next].x,back[next].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);   // Next vertex on front shoulder back
        glTexCoord2f(back2[next].x,back2[next].z);
        glVertex3f(back2[next].x, back2[next].y, back2[next].z); // Next vertex on back shoulder back
        glTexCoord2f(back2[i].x,back2[i].z);
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
    drawCenteredCylinder(0.32f, 0.4f, 20);
    glPopMatrix();

    // Decorative rings on flat faces
    glColor3fv(white);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.00f, 0.0f, 0.0f);
    drawCenteredCylinder(0.22f, 0.6f, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.26f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.22f, 0.30f, 20, 1);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix(); // End shoulder armor


    // ---------------------------
    // Shoulder connector (local offset preserved)
    // ---------------------------
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    glColor3fv(white);
    drawCenteredCube(0.50f, 0.45f, 0.50f);
    glPopMatrix();

    // ---------------------------
    // Upper arm (still under shoulder rotations)
    // ---------------------------
    glPushMatrix();
    glTranslatef(0.0f, -0.25f, 0.0f);
    glColor3fv(white);
    drawChamferedCube(0.6f, 0.7f, 0.6f, 0.05f);
    glPopMatrix();

    // ---------------------------
    // Elbow group (this group rotates about the elbow pivot)
    // original elbow translation from connector: (0.0f, -0.75f, 0.0f)
    // ---------------------------
    glPushMatrix();

    // ====================== Draw Shield ===
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    glTranslatef(0, -0.5f, 0.8f);
    drawShield();
    glPopMatrix();
    // ====================== End Shield ===



    // move to elbow pivot
    glTranslatef(0.0f, -0.75f, 0.0f);

    // elbow bend: rotates everything "downstream" (forearm, cuff, hand)
    glRotatef(leftElbowAngle, 1.0f, 0.0f, 0.0f);

    // --- draw elbow cuboid (still at elbow pivot) ---
    glColor3fv(darkGrey);
    drawChamferedCube(0.5, 0.8, 0.59, 0.1);

    // --- draw elbow cylinder (decorative) ---
    glColor3fv(white);
    glPushMatrix();
    glTranslatef(0, -0.1f, 0);
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    drawCenteredCylinder(0.28f, 0.7f, 20);
    glRotatef(-90, 0.0f, 0.0f, 1.0f);

    // decorative rings
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.36f, 0.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    gluDisk(gluObject, 0.0f, 0.07f, 20, 1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.36f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    gluDisk(gluObject, 0.0f, 0.07f, 20, 1);
    glPopMatrix();
    glPopMatrix(); // end elbow cylinder

    // ---------------------------
    // Forearm (child of elbow, therefore rotated with elbow)
    // original forearm global translation: (0.0f, -1.5f, 0.0f)
    // relative to elbow pivot we need to go further -0.75f
    // ---------------------------
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f); // from elbow -> forearm
    glColor3fv(white);
    drawChamferedCube(0.6f, 0.9f, 0.6f, 0.05f);

    // --- Forearm cuff (follows forearm) ---
    // original cuff global: -2.10 ; relative to forearm (-1.5 -> -2.10) delta -0.6
    glPushMatrix();
    glTranslatef(0.0f, -0.60f, 0.0f); // from forearm -> cuff
    glColor3fv(white);
    drawTaperedCube(0.55f, 0.55f, 0.4f, 0.4f, 0.3f);

    // ---------------------------
    // Wrist / Palm container:
    //   - apply wristDeg to rotate entire hand (including palm + fingers)
    //   - then inside there is a palm-only rotation (palmDeg)
    // ---------------------------
    glPushMatrix();
    // from cuff (-2.10) to palm (-2.40) delta -0.3
    glTranslatef(0.0f, -0.30f, 0.0f);

    // wrist rotation (rotates whole hand)
    glRotatef(leftWristAngle, 1.0f, 0.0f, 0.0f);

    // ---------- palm-only group ----------
    glPushMatrix();
    // shield 

    glColor3fv(darkGrey);
    drawChamferedCube(0.4f, 0.4f, 0.4f, 0.05f);

    // --- Finger mass (child of palm: can curl) ---
    glPushMatrix();
    // original finger mass translation from palm: (0.0f, -0.45f, 0.0f)
    glTranslatef(0.0f, -0.45f, 0.0f);

    // apply finger curl around X axis (curl downwards)
    glRotatef(leftFingersCurlAngle, 1.0f, 0.0f, 0.0f);

    // draw finger block (same cube as before)
    glColor3fv(white);
    drawCenteredCube(0.32f, 0.5f, 0.35f);

    // Finger grooves (vertical lines suggesting fingers) - keep relative offsets
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.08f, 0.0f, 0.18f);
        glColor3f(0.1f, 0.1f, 0.1f);
        drawCenteredCube(0.015f, 0.48f, 0.05f);
        glPopMatrix();
    }
    glPopMatrix(); // end fingers

    // --- Thumb (positioned relative to palm; it should rotate with palm)
    glPushMatrix();
    glTranslatef(-0.25f, -0.15f, 0.1f);
    glRotatef(-25, 0.0f, 0.0f, 1.0f);
    glColor3fv(darkGrey);
    drawCenteredCube(0.12f, 0.4f, 0.18f);
    glPopMatrix();

    glPopMatrix(); // end palm-only group

    glPopMatrix(); // end wrist / palm container

    glPopMatrix(); // end forearm cuff
    glPopMatrix(); // end forearm
    glPopMatrix(); // end elbow group

    glPopMatrix(); // end arm root (entire-arm movement)
}

void drawRightHand()
{
    float white[3] = { 0.95f, 0.95f, 0.95f };
    float darkGrey[3] = { 0.25f, 0.25f, 0.25f };
    float red[3] = { 0.8f, 0.1f, 0.1f };

    // enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, metalWhiteTexture1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // or GL_REPLACE
    gluQuadricTexture(gluObject, GL_TRUE); // auto-generates texcoords for quadrics (spheres/cylinders/disks)

    // ---------------------------
    // ARM HIERARCHY (rooted at the shoulder pivot)
    // ---------------------------
    glPushMatrix();
    // Move origin to shoulder pivot
    glTranslatef(0.4f, 0.4f, 0.0f);

    // Whole-arm rotations (shoulder-level) - mirrored angles
    glRotatef(-rightShoulderYawAngle, 0.0f, 1.0f, 0.0f);    // negated yaw
    glRotatef(rightShoulderPitchAngle, 1.0f, 0.0f, 0.0f);   // pitch around X

    // =================================================================
    // PART 1: SHOULDER ARMOR (Shield-like trapezoidal shape)
    // =================================================================
    glPushMatrix();
    glTranslatef(0.4f, 0.4f, 0.0f);  // mirrored X

    // shoulder trapezoidal (front) - mirrored X coordinates
    Vec3 front[7] = {
        { 0.0f,  0.1f,  0.35f},   // Bottom right 1
        {-0.1f,  0.0f,  0.35f},   // Bottom right 2
        {-0.7f,  0.2f,  0.35f},   // Bottom Left 
        {-0.8f,  0.7f,  0.35f},   // Top Left 1
        {-0.6f,  0.9f,  0.35f},   // Top left 2
        {-0.1f,  0.9f,  0.35f},   // Top right 1
        { 0.0f,  0.8f,  0.35f}    // Top right 2
    };

    Vec3 back[7] = {
        { 0.1f,  0.0f,  0.35f},   // Bottom right 1
        {-0.1f, -0.1f,  0.35f},   // Bottom right 2
        {-0.8f,  0.1f,  0.35f},   // Bottom Left 
        {-0.9f,  0.7f,  0.35f},   // Top Left 1
        {-0.7f,  1.0f,  0.35f},   // Top left 2
        {-0.1f,  1.0f,  0.35f},   // Top right 1
        { 0.1f,  0.8f,  0.35f}    // Top right 2
    };

    float backExtrude = 0.10f;

    for (int i = 0; i < 7; i++) {
        back[i].z = back[i].z - backExtrude;
    }

    // draw Front face (with texcoords)
    glColor3fv(white);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < 7; i++) {
        glTexCoord2f(front[i].x, front[i].y);
        glVertex3f(front[i].x, front[i].y, front[i].z);
    }
    glEnd();

    // draw Back face (with texcoords; reversed order)
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 6; i >= 0; i--) {
        glTexCoord2f(back[i].x, back[i].y);
        glVertex3f(back[i].x, back[i].y, back[i].z);
    }
    glEnd();

    // Sloped sides connect front and back edges (with texcoords)
    glColor3fv(white);
    glBegin(GL_QUADS);
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        Vec3 v1 = { back[i].x - front[i].x, back[i].y - front[i].y, back[i].z - front[i].z };
        Vec3 v2 = { front[next].x - front[i].x, front[next].y - front[i].y, front[next].z - front[i].z };

        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(-nx, -ny, -nz);

        glTexCoord2f(front[i].x, front[i].z);
        glVertex3f(front[i].x, front[i].y, front[i].z);

        glTexCoord2f(front[next].x, front[next].z);
        glVertex3f(front[next].x, front[next].y, front[next].z);

        glTexCoord2f(back[next].x, back[next].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);

        glTexCoord2f(back[i].x, back[i].z);
        glVertex3f(back[i].x, back[i].y, back[i].z);
    }
    glEnd();

    // shoulder trapezoidal (back) - mirrored X coordinates
    Vec3 front2[7] = {
        { 0.0f,  0.1f,  -0.35f},   // Bottom right 1
        {-0.1f,  0.0f,  -0.35f},   // Bottom right 2
        {-0.7f,  0.2f,  -0.35f},   // Bottom Left 
        {-0.8f,  0.7f,  -0.35f},   // Top Left 1
        {-0.6f,  0.9f,  -0.35f},   // Top left 2
        {-0.1f,  0.9f,  -0.35f},   // Top right 1
        { 0.0f,  0.8f,  -0.35f}    // Top right 2
    };

    Vec3 back2[7] = {
        { 0.1f,  0.0f,  -0.35f},   // Bottom right 1
        {-0.1f, -0.1f,  -0.35f},   // Bottom right 2
        {-0.8f,  0.1f,  -0.35f},   // Bottom Left 
        {-0.9f,  0.7f,  -0.35f},   // Top Left 1
        {-0.7f,  1.0f,  -0.35f},   // Top left 2
        {-0.1f,  1.0f,  -0.35f},   // Top right 1
        { 0.1f,  0.8f,  -0.35f}    // Top right 2
    };

    for (int i = 0; i < 7; i++) {
        back2[i].z = back2[i].z + backExtrude;
    }

    // draw Front face
    glColor3fv(white);
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 0; i < 7; i++) {
        glTexCoord2f(front2[i].x, front2[i].y);
        glVertex3f(front2[i].x, front2[i].y, front2[i].z);
    }
    glEnd();

    // draw Back face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 6; i >= 0; i--) {
        glTexCoord2f(back2[i].x, back2[i].y);
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    }
    glEnd();

    // Sloped sides
    glColor3fv(white);
    glBegin(GL_QUADS);
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        Vec3 v1 = { back2[i].x - front2[i].x, back2[i].y - front2[i].y, back2[i].z - front2[i].z };
        Vec3 v2 = { front2[next].x - front2[i].x, front2[next].y - front2[i].y, front2[next].z - front2[i].z };

        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(nx, ny, nz);

        glTexCoord2f(front2[i].x, front2[i].z);
        glVertex3f(front2[i].x, front2[i].y, front2[i].z);

        glTexCoord2f(front2[next].x, front2[next].z);
        glVertex3f(front2[next].x, front2[next].y, front2[next].z);

        glTexCoord2f(back2[next].x, back2[next].z);
        glVertex3f(back2[next].x, back2[next].y, back2[next].z);

        glTexCoord2f(back2[i].x, back2[i].z);
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    }
    glEnd();

    // connecting both back faces
    glColor3fv(white);
    glBegin(GL_QUADS);
    for (int i = 0; i < 7; i++) {
        int next = (i + 1) % 7;

        if (i == 2) {
            continue;
        }

        Vec3 v1 = { back2[i].x - back[i].x, back2[i].y - back[i].y, back2[i].z - back[i].z };
        Vec3 v2 = { back[next].x - back[i].x, back[next].y - back[i].y, back[next].z - back[i].z };

        float nx = v1.y * v2.z - v1.z * v2.y;
        float ny = v1.z * v2.x - v1.x * v2.z;
        float nz = v1.x * v2.y - v1.y * v2.x;

        float len = sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.001f) {
            nx /= len; ny /= len; nz /= len;
        }

        glNormal3f(-nx, -ny, -nz);

        glTexCoord2f(back[i].x, back[i].z);
        glVertex3f(back[i].x, back[i].y, back[i].z);

        glTexCoord2f(back[next].x, back[next].z);
        glVertex3f(back[next].x, back[next].y, back[next].z);

        glTexCoord2f(back2[next].x, back2[next].z);
        glVertex3f(back2[next].x, back2[next].y, back2[next].z);

        glTexCoord2f(back2[i].x, back2[i].z);
        glVertex3f(back2[i].x, back2[i].y, back2[i].z);
    }

    glEnd();

    // Internal connecting cylinder
    glColor3fv(darkGrey);
    glPushMatrix();
    glRotatef(-90, 0.0f, 1.0f, 0.0f);  // negated rotation
    glTranslatef(0.0f, 0.4f, 0.5f);
    glColor3fv(darkGrey);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    drawCenteredCylinder(0.32f, 0.4f, 20);
    glPopMatrix();

    // Decorative rings
    glColor3fv(white);
    glPushMatrix();
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.00f, 0.0f, 0.0f);
    drawCenteredCylinder(0.22f, 0.6f, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.26f, 0.0f, 0.0f);  // mirrored X
    glRotatef(90, 0.0f, 1.0f, 0.0f);  // mirrored rotation
    gluDisk(gluObject, 0.22f, 0.30f, 20, 1);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix(); // End shoulder armor

    // Shoulder connector
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    glColor3fv(white);
    drawCenteredCube(0.50f, 0.45f, 0.50f);
    glPopMatrix();

    // Upper arm
    glPushMatrix();
    glTranslatef(0.0f, -0.25f, 0.0f);
    glColor3fv(white);
    drawChamferedCube(0.6f, 0.7f, 0.6f, 0.05f);
    glPopMatrix();

    // Elbow group
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f);
    glRotatef(rightElbowAngle, 1.0f, 0.0f, 0.0f);

    // draw elbow cuboid
    glColor3fv(darkGrey);
    drawChamferedCube(0.5, 0.8, 0.59, 0.1);

    // draw elbow cylinder
    glColor3fv(white);
    glPushMatrix();
    glTranslatef(0, -0.1f, 0);
    glRotatef(90, 0.0f, 0.0f, 1.0f);
    drawCenteredCylinder(0.28f, 0.7f, 20);
    glRotatef(-90, 0.0f, 0.0f, 1.0f);

    // decorative rings
    glColor3f(0.0f, 0.0f, 0.0f);
    glPushMatrix();
    glTranslatef(0.36f, 0.0f, 0.0f);
    glRotatef(90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    gluDisk(gluObject, 0.0f, 0.07f, 20, 1);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.36f, 0.0f, 0.0f);
    glRotatef(-90, 0.0f, 1.0f, 0.0f);
    gluDisk(gluObject, 0.12f, 0.20f, 20, 1);
    gluDisk(gluObject, 0.0f, 0.07f, 20, 1);
    glPopMatrix();
    glPopMatrix(); // end elbow cylinder

    // Forearm
    glPushMatrix();
    glTranslatef(0.0f, -0.75f, 0.0f);
    glColor3fv(white);
    drawChamferedCube(0.6f, 0.9f, 0.6f, 0.05f);

    // Forearm cuff
    glPushMatrix();
    glTranslatef(0.0f, -0.60f, 0.0f);
    glColor3fv(white);
    drawTaperedCube(0.55f, 0.55f, 0.4f, 0.4f, 0.3f);

    // Wrist / Palm container
    glPushMatrix();
    glTranslatef(0.0f, -0.30f, 0.0f);
    glRotatef(rightWristAngle, 1.0f, 0.0f, 0.0f);

    // palm-only group
    glPushMatrix();
    glColor3fv(darkGrey);
    drawChamferedCube(0.4f, 0.4f, 0.4f, 0.05f);

    // Finger mass
    glPushMatrix();
    glTranslatef(0.0f, -0.45f, 0.0f);
    glRotatef(rightFingersCurlAngle, 1.0f, 0.0f, 0.0f);

    // draw finger block
    glColor3fv(white);
    drawCenteredCube(0.32f, 0.5f, 0.35f);

    // Finger grooves
    for (int i = -1; i <= 1; i++) {
        glPushMatrix();
        glTranslatef(i * 0.08f, 0.0f, 0.18f);
        glColor3f(0.1f, 0.1f, 0.1f);
        drawCenteredCube(0.015f, 0.48f, 0.05f);
        glPopMatrix();
    }
    glPopMatrix(); // end fingers

    // Thumb - mirrored position
    glPushMatrix();
    glTranslatef(0.25f, -0.15f, 0.1f);  // mirrored X
    glRotatef(25, 0.0f, 0.0f, 1.0f);   // negated rotation
    glColor3fv(darkGrey);
    drawCenteredCube(0.12f, 0.4f, 0.18f);
    glPopMatrix();

    glPopMatrix(); // end palm-only group
    glPopMatrix(); // end wrist / palm container
    glPopMatrix(); // end forearm cuff
    glPopMatrix(); // end forearm
    glPopMatrix(); // end elbow group
    glPopMatrix(); // end arm root
}
