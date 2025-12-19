#pragma once
#include <chrono>

GLUquadric* gluObject = nullptr;
struct Vec3 {
    float x, y, z;
};

// Robot variable
// === hand ===
float leftElbowAngle = 0.0f;
float leftWristAngle = 0.0f;
float leftFingersCurlAngle = 0.0f;
float leftShoulderPitchAngle = 0.0f;
float leftShoulderYawAngle = 0.0f;
float rightElbowAngle = 0.0f;
float rightWristAngle = 0.0f;
float rightFingersCurlAngle = 0.0f;
float rightShoulderPitchAngle = 0.0f;
float rightShoulderYawAngle = 0.0f;

//  === end hand ===


// Forward declarations for functions used in main.cpp
void updateProjection(int width, int height);
void drawLeftHand();
void drawDetailedLeftLeg();
void drawDetailedRightLeg();
void drawSword();
void drawShield();
void drawUpperBody();
void drawLowerBody();
void drawGundamHead();
void Display(HWND hWnd);

// You may add additional shared declarations here if needed.

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

void drawCenteredCube(float w, float h, float d) {
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
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, -hh, -hd);
    glEnd();

    // Left
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(-hw, -hh, -hd);
    glVertex3f(-hw, -hh, hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(-hw, hh, -hd);
    glEnd();

    // Right
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glVertex3f(hw, -hh, -hd);
    glVertex3f(hw, hh, -hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, -hh, hd);
    glEnd();

    // Top
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-hw, hh, -hd);
    glVertex3f(-hw, hh, hd);
    glVertex3f(hw, hh, hd);
    glVertex3f(hw, hh, -hd);
    glEnd();

    // Bottom
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

// Draw a tapered cube (top and bottom are rectangles which can differ in size).
// Centered at origin, Y is up.
// Parameters:
//   topW, topD    : width (X) and depth (Z) of the top face
//   bottomW, bottomD: width (X) and depth (Z) of the bottom face
//   height        : total height (distance between top and bottom)
void drawTaperedCube(float topW, float topD, float bottomW, float bottomD, float height)
{
    float halfH = height * 0.5f;

    // top face (y = +halfH), ordered CCW when looking from above
    float top[4][3] = {
        { -topW * 0.5f,  halfH, -topD * 0.5f }, // 0: top-left (near -Z)
        {  topW * 0.5f,  halfH, -topD * 0.5f }, // 1: top-right
        {  topW * 0.5f,  halfH,  topD * 0.5f }, // 2: bottom-right (far +Z)
        { -topW * 0.5f,  halfH,  topD * 0.5f }  // 3: bottom-left
    };

    // bottom face (y = -halfH), ordered CCW when looking from below (so normal points down)
    float bottom[4][3] = {
        { -bottomW * 0.5f, -halfH, -bottomD * 0.5f }, // 0
        {  bottomW * 0.5f, -halfH, -bottomD * 0.5f }, // 1
        {  bottomW * 0.5f, -halfH,  bottomD * 0.5f }, // 2
        { -bottomW * 0.5f, -halfH,  bottomD * 0.5f }  // 3
    };

    // Top face
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 4; ++i) glVertex3f(top[i][0], top[i][1], top[i][2]);
    glEnd();

    // Bottom face (reverse order so normal points down)
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 3; i >= 0; --i) glVertex3f(bottom[i][0], bottom[i][1], bottom[i][2]);
    glEnd();

    // Helper lambdas (or local functions) for vector math
    auto sub = [](const float a[3], const float b[3], float out[3]) {
        out[0] = a[0] - b[0]; out[1] = a[1] - b[1]; out[2] = a[2] - b[2];
        };
    auto cross = [](const float a[3], const float b[3], float out[3]) {
        out[0] = a[1] * b[2] - a[2] * b[1];
        out[1] = a[2] * b[0] - a[0] * b[2];
        out[2] = a[0] * b[1] - a[1] * b[0];
        };
    auto normalize = [](float v[3]) {
        float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        if (len > 1e-6f) { v[0] /= len; v[1] /= len; v[2] /= len; }
        };

    // Sides (4 quads). Each quad connects top[i] -> top[next] -> bottom[next] -> bottom[i]
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;

        // compute normal: cross( top[next] - top[i], bottom[i] - top[i] )
        float a[3], b[3], n[3];
        sub(top[next], top[i], a);
        sub(bottom[i], top[i], b);
        cross(a, b, n);
        normalize(n);

        glNormal3f(n[0], n[1], n[2]);

        glVertex3f(top[i][0], top[i][1], top[i][2]);
        glVertex3f(top[next][0], top[next][1], top[next][2]);
        glVertex3f(bottom[next][0], bottom[next][1], bottom[next][2]);
        glVertex3f(bottom[i][0], bottom[i][1], bottom[i][2]);
    }
    glEnd();
}


// ======================================================= Animation =================================================================
// ---------------- Simple Block animation----------------

enum BlockState {
    BLOCK_IDLE = 0,
    BLOCK_RAISING,
    BLOCK_HOLDING,
    BLOCK_LOWERING,
    BLOCK_RECOVERING
};

struct BlockAnim {
    BlockState state;
    double stateStartTime;
    // snapshot of idle pose (to lerp from/to)
    float idle_shoulderYaw;
    float idle_shoulderPitch;
    float idle_elbow;
    float idle_wrist;
    float idle_fingers;
};

static BlockAnim blockAnim = { BLOCK_IDLE, 0.0, 0,0,0,0,0 };

// Hard-coded targets (tweak as needed)
static const float BLOCK_SHOULDER_YAW_TARGET = -68.0f;
static const float BLOCK_SHOULDER_PITCH_TARGET = -25.0f;
static const float BLOCK_ELBOW_TARGET = -30.0f;
static const float BLOCK_WRIST_TARGET = -38.0f;
static const float BLOCK_FINGERS_TARGET = 8.0f;

// Durations (seconds)
static const double DURATION_RAISING = 0.58;
static const double DURATION_HOLD = 1.55;
static const double DURATION_LOWERING = 0.52;
static const double DURATION_RECOVER = 0.15;

// High-resolution timer (seconds since epoch). Small overhead, good precision.
static double now_seconds()
{
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

// small smooth easing
static float smoothstep_ease(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return t * t * (3.0f - 2.0f * t);
}
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Start the Block animation (callable from keypress)
void startBlock()
{
    // snapshot current pose as idle
    blockAnim.idle_shoulderYaw = leftShoulderYawAngle;
    blockAnim.idle_shoulderPitch = leftShoulderPitchAngle;
    blockAnim.idle_elbow = leftElbowAngle;
    blockAnim.idle_wrist = leftWristAngle;
    blockAnim.idle_fingers = leftFingersCurlAngle;

    blockAnim.state = BLOCK_RAISING;
    blockAnim.stateStartTime = now_seconds();
}

// Update per-frame: call every frame (I add a call in Display())
void updateBlockAnim(double now)
{
    switch (blockAnim.state) {
    case BLOCK_IDLE:
        return;
    case BLOCK_RAISING: {
        double t = (now - blockAnim.stateStartTime) / DURATION_RAISING;
        if (t >= 1.0) t = 1.0;
        float e = smoothstep_ease((float)t);

        leftShoulderYawAngle = lerp(blockAnim.idle_shoulderYaw, BLOCK_SHOULDER_YAW_TARGET, e);
        leftShoulderPitchAngle = lerp(blockAnim.idle_shoulderPitch, BLOCK_SHOULDER_PITCH_TARGET, e);
        leftElbowAngle = lerp(blockAnim.idle_elbow, BLOCK_ELBOW_TARGET, e);
        leftWristAngle = lerp(blockAnim.idle_wrist, BLOCK_WRIST_TARGET, e);
        leftFingersCurlAngle = lerp(blockAnim.idle_fingers, BLOCK_FINGERS_TARGET, e);

        if (t >= 1.0) {
            blockAnim.state = BLOCK_HOLDING;
            blockAnim.stateStartTime = now;
        }
        return;
    }
    case BLOCK_HOLDING: {
        double elapsed = now - blockAnim.stateStartTime;
        leftShoulderYawAngle = BLOCK_SHOULDER_YAW_TARGET;
        leftShoulderPitchAngle = BLOCK_SHOULDER_PITCH_TARGET;
        leftElbowAngle = BLOCK_ELBOW_TARGET;
        leftWristAngle = BLOCK_WRIST_TARGET;
        leftFingersCurlAngle = BLOCK_FINGERS_TARGET;

        if (elapsed >= DURATION_HOLD) {
            blockAnim.state = BLOCK_LOWERING;
            blockAnim.stateStartTime = now;
        }
        return;
    }
    case BLOCK_LOWERING: {
        double t = (now - blockAnim.stateStartTime) / DURATION_LOWERING;
        if (t >= 1.0) t = 1.0;
        float e = smoothstep_ease((float)t);

        leftShoulderYawAngle = lerp(BLOCK_SHOULDER_YAW_TARGET, blockAnim.idle_shoulderYaw, e);
        leftShoulderPitchAngle = lerp(BLOCK_SHOULDER_PITCH_TARGET, blockAnim.idle_shoulderPitch, e);
        leftElbowAngle = lerp(BLOCK_ELBOW_TARGET, blockAnim.idle_elbow, e);
        leftWristAngle = lerp(BLOCK_WRIST_TARGET, blockAnim.idle_wrist, e);
        leftFingersCurlAngle = lerp(BLOCK_FINGERS_TARGET, blockAnim.idle_fingers, e);

        if (t >= 1.0) {
            blockAnim.state = BLOCK_RECOVERING;
            blockAnim.stateStartTime = now;
        }
        return;
    }
    case BLOCK_RECOVERING: {
        double t = (now - blockAnim.stateStartTime) / DURATION_RECOVER;
        if (t >= 1.0) t = 1.0;
        float e = smoothstep_ease((float)t);

        // set final to idle snapshot to avoid numeric drift
        leftShoulderYawAngle = blockAnim.idle_shoulderYaw;
        leftShoulderPitchAngle = blockAnim.idle_shoulderPitch;
        leftElbowAngle = blockAnim.idle_elbow;
        leftWristAngle = blockAnim.idle_wrist;
        leftFingersCurlAngle = blockAnim.idle_fingers;

        if (t >= 1.0) {
            blockAnim.state = BLOCK_IDLE;
        }
        return;
    }
    }
}

// ======================================================= End Animation =================================================================
