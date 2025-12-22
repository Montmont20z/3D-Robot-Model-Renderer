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
void drawSword();
void drawShield();
void drawUpperBody();
void drawLowerBody();
void drawGundamHead();
void Display(HWND hWnd);

// simple 3 float vector helpers
inline void vec3_sub(const float a[3], const float b[3], float out[3]) { out[0] = a[0] - b[0]; out[1] = a[1] - b[1]; out[2] = a[2] - b[2]; }
inline void vec3_add_mut(float a[3], const float b[3]) { a[0] += b[0]; a[1] += b[1]; a[2] += b[2]; }
inline void vec3_scale(const float a[3], float s, float out[3]) { out[0] = a[0] * s; out[1] = a[1] * s; out[2] = a[2] * s; }
inline float vec3_len(const float a[3]) { return sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]); }
inline void vec3_norm(float a[3]) { float l = vec3_len(a); if (l > 1e-6f) { a[0] /= l; a[1] /= l; a[2] /= l; } }
inline void vec3_cross(const float a[3], const float b[3], float out[3]) {
    out[0] = a[1] * b[2] - a[2] * b[1]; // i
    out[1] = a[2] * b[0] - a[0] * b[2]; // j
    out[2] = a[0] * b[1] - a[1] * b[0]; // k
}

static inline void computeNormalFA(const float a[3], const float b[3], const float c[3]) {
    float u[3], v[3], n[3];
    // u = b - a, v = c - a
    vec3_sub(b, a, u);
    vec3_sub(c, a, v);
    // n = u x v
    vec3_cross(u, v, n);
    // normalize and send to GL
    vec3_norm(n);
    glNormal3f(n[0], n[1], n[2]);
}

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
    float s = size / 2.0f;
    glBegin(GL_QUADS);
    // Front Face
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(s, -s, s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(s, s, s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s, s, s);
    // Back Face
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s, s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(s, s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(s, -s, -s);
    // Top Face
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s, s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, s, s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(s, s, s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(s, s, -s);
    // Bottom Face
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(s, -s, -s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(s, -s, s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, s);
    // Right face
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(s, s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(s, s, s);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(s, -s, s);
    // Left face
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s, s, s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s, s, -s);
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

    // Front (+Z)
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, hd);
    glEnd();

    // Back (-Z)
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, hh, -hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, -hd);
    glEnd();

    // Left (-X)
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glEnd();

    // Right (+X)
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, -hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, hh, -hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, hd);
    glEnd();

    // Top (+Y)
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, -hd);
    glEnd();

    // Bottom (-Y)
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, -hh, -hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, hd);
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
        // Front edge (z = hd, the front of the cube)
		{-hw + chamfer, hh, hd},    // 0: Front-left, moved RIGHT by chamfer
		{hw - chamfer, hh, hd},     // 1: Front-right, moved LEFT by chamfer

		// Right edge (x = hw, the right side)
		{hw, hh, hd - chamfer},     // 2: Right-front, moved BACK by chamfer
		{hw, hh, -hd + chamfer},    // 3: Right-back, moved FORWARD by chamfer

		// Back edge (z = -hd, the back of the cube)
		{hw - chamfer, hh, -hd},    // 4: Back-right, moved LEFT by chamfer
		{-hw + chamfer, hh, -hd},   // 5: Back-left, moved RIGHT by chamfer

		// Left edge (x = -hw, the left side)
		{-hw, hh, -hd + chamfer},   // 6: Left-back, moved FORWARD by chamfer
		{-hw, hh, hd - chamfer}     // 7: Left-front, moved BACK by chamfer
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
        float u = (topVerts[i].x + hw) / (width); // range from 0 to 1
        float v = (topVerts[i].z + hd) / (depth); // range from 0 to 1
        glTexCoord2f(u, v);
        glVertex3f(topVerts[i].x, topVerts[i].y, topVerts[i].z);
    }
    glEnd();

    // Draw bottom face (octagon)
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 7; i >= 0; i--) {
        float u = (bottomVerts[i].x + hw) / (width);
        float v = (bottomVerts[i].z + hd) / (depth);
        glTexCoord2f(u, v);
        glVertex3f(bottomVerts[i].x, bottomVerts[i].y, bottomVerts[i].z);
    }
    glEnd();

    // Draw 8 main side faces (large rectangles)
    glBegin(GL_QUADS);

    // Front face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[0].x, topVerts[0].y, topVerts[0].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[1].x, topVerts[1].y, topVerts[1].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[1].x, bottomVerts[1].y, bottomVerts[1].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[0].x, bottomVerts[0].y, bottomVerts[0].z);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[2].x, topVerts[2].y, topVerts[2].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[3].x, topVerts[3].y, topVerts[3].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[3].x, bottomVerts[3].y, bottomVerts[3].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[2].x, bottomVerts[2].y, bottomVerts[2].z);

    // Back face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[4].x, topVerts[4].y, topVerts[4].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[5].x, topVerts[5].y, topVerts[5].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[5].x, bottomVerts[5].y, bottomVerts[5].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[4].x, bottomVerts[4].y, bottomVerts[4].z);

    // Left face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[6].x, topVerts[6].y, topVerts[6].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[7].x, topVerts[7].y, topVerts[7].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[7].x, bottomVerts[7].y, bottomVerts[7].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[6].x, bottomVerts[6].y, bottomVerts[6].z);


    glEnd();

    // Draw 8 small chamfer faces (corners)
    glBegin(GL_QUADS);

    // Front-right chamfer
    glNormal3f(0.707f, 0.0f, 0.707f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[1].x, topVerts[1].y, topVerts[1].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[2].x, topVerts[2].y, topVerts[2].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[2].x, bottomVerts[2].y, bottomVerts[2].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[1].x, bottomVerts[1].y, bottomVerts[1].z);

    // Back-right chamfer
    glNormal3f(0.707f, 0.0f, -0.707f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[3].x, topVerts[3].y, topVerts[3].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[4].x, topVerts[4].y, topVerts[4].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[4].x, bottomVerts[4].y, bottomVerts[4].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[3].x, bottomVerts[3].y, bottomVerts[3].z);

    // Back-left chamfer
    glNormal3f(-0.707f, 0.0f, -0.707f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[5].x, topVerts[5].y, topVerts[5].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[6].x, topVerts[6].y, topVerts[6].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[6].x, bottomVerts[6].y, bottomVerts[6].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[5].x, bottomVerts[5].y, bottomVerts[5].z);

    // Front-left chamfer
    glNormal3f(-0.707f, 0.0f, 0.707f);
    glTexCoord2f(0, 1); glVertex3f(topVerts[7].x, topVerts[7].y, topVerts[7].z);
    glTexCoord2f(1, 1); glVertex3f(topVerts[0].x, topVerts[0].y, topVerts[0].z);
    glTexCoord2f(1, 0); glVertex3f(bottomVerts[0].x, bottomVerts[0].y, bottomVerts[0].z);
    glTexCoord2f(0, 0); glVertex3f(bottomVerts[7].x, bottomVerts[7].y, bottomVerts[7].z);

    glEnd();
}


void drawTaperedCube(float topW, float topD, float bottomW, float bottomD, float height)
{
    // 1) basic setup
    float halfH = height * 0.5f;
    const float EPS = 1e-6f;
    float tw = (fabsf(topW) > EPS) ? topW : EPS;
    float td = (fabsf(topD) > EPS) ? topD : EPS;
    float bw = (fabsf(bottomW) > EPS) ? bottomW : EPS;
    float bd = (fabsf(bottomD) > EPS) ? bottomD : EPS;

    // 2) compute the 8 corner positions (top and bottom rectangles)
    float top[4][3] = {
        { -topW * 0.5f,  halfH, -topD * 0.5f }, // 0
        {  topW * 0.5f,  halfH, -topD * 0.5f }, // 1
        {  topW * 0.5f,  halfH,  topD * 0.5f }, // 2
        { -topW * 0.5f,  halfH,  topD * 0.5f }  // 3
    };

    float bottom[4][3] = {
        { -bottomW * 0.5f, -halfH, -bottomD * 0.5f }, // 0
        {  bottomW * 0.5f, -halfH, -bottomD * 0.5f }, // 1
        {  bottomW * 0.5f, -halfH,  bottomD * 0.5f }, // 2
        { -bottomW * 0.5f, -halfH,  bottomD * 0.5f }  // 3
    };

    // 3) Top face: planar X->U, Z->V mapping, normal up
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 4; ++i) {
        float u = (top[i][0] + topW * 0.5f) / tw;   // X -> [0,1]
        float v = (top[i][2] + topD * 0.5f) / td;   // Z -> [0,1]
        glTexCoord2f(u, v);
        glVertex3fv(top[i]);
    }
    glEnd();

    // 4) Bottom face: same mapping but draw reversed so normal points down
    glBegin(GL_POLYGON);
    glNormal3f(0.0f, -1.0f, 0.0f);
    for (int i = 3; i >= 0; --i) {
        float u = (bottom[i][0] + bottomW * 0.5f) / bw;
        float v = (bottom[i][2] + bottomD * 0.5f) / bd;
        glTexCoord2f(u, v);
        glVertex3fv(bottom[i]);
    }
    glEnd();

    // 5) Sides: 4 quads. Per-quad normal computed with computeNormalFA().
    //    Each side gets simple UVs (0,1),(1,1),(1,0),(0,0).
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; ++i) {
        int next = (i + 1) % 4;

        // compute normal using top[i], top[next], bottom[i]
        computeNormalFA(top[i], top[next], bottom[i]);

        glTexCoord2f(0.0f, 1.0f); glVertex3fv(top[i]);
        glTexCoord2f(1.0f, 1.0f); glVertex3fv(top[next]);
        glTexCoord2f(1.0f, 0.0f); glVertex3fv(bottom[next]);
        glTexCoord2f(0.0f, 0.0f); glVertex3fv(bottom[i]);
    }
    glEnd();
}

// ======================================================= Material ==================================================================
GLuint LoadBMPTexture(const char* filename) {
    HBITMAP hBMP = (HBITMAP)LoadImage(
        NULL,
        filename,
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    if (!hBMP) {
        std::cout << "Failed to load " << filename << std::endl;
        return 0;
    }

    BITMAP BMP;
    GetObject(hBMP, sizeof(BMP), &BMP);

    if (BMP.bmBits == NULL) {
        std::cout << "BMP.bmBits is NULL for " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gluBuild2DMipmaps(
        GL_TEXTURE_2D,
        GL_RGB,
        BMP.bmWidth,
        BMP.bmHeight,
        GL_BGR_EXT,
        GL_UNSIGNED_BYTE,
        BMP.bmBits
    );

    DeleteObject(hBMP);
    return textureID;
}


void setMetallicMaterial() {
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 100.0f };  // High = tight, sharp highlights

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

void setPlasticMaterial() {
    GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat mat_shininess[] = { 32.0f };  // Lower = broader highlights

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// ======================================================= End Material ==================================================================


// ======================================================= Animation =================================================================
enum BlockState {
    BLOCK_IDLE = 0,
    BLOCK_RAISING,
    BLOCK_HOLDING,
    BLOCK_LOWERING
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

// Hard-coded targets 
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

// get current time in seconds since computer start
static double now_seconds()
{
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}

// lineear interpolation
// Start at a, move towards b by t percent
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Start the Block animation
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
        // figure out the current animation completion percentage (0% to 100%)
        double t = (now - blockAnim.stateStartTime) / DURATION_RAISING;
        if (t >= 1.0) t = 1.0; // prevent from going above 100%. t = 0.0 (0%), t = 1.0 (100%)

        leftShoulderYawAngle = lerp(blockAnim.idle_shoulderYaw, BLOCK_SHOULDER_YAW_TARGET, t);
        leftShoulderPitchAngle = lerp(blockAnim.idle_shoulderPitch, BLOCK_SHOULDER_PITCH_TARGET, t);
        leftElbowAngle = lerp(blockAnim.idle_elbow, BLOCK_ELBOW_TARGET, t);
        leftWristAngle = lerp(blockAnim.idle_wrist, BLOCK_WRIST_TARGET, t);
        leftFingersCurlAngle = lerp(blockAnim.idle_fingers, BLOCK_FINGERS_TARGET, t);

        if (t >= 1.0) {
            blockAnim.state = BLOCK_HOLDING; // go to next animation state
            blockAnim.stateStartTime = now; // reset start time
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
        // figure out the current animation completion percentage (0% to 100%)
        double t = (now - blockAnim.stateStartTime) / DURATION_LOWERING;
        if (t >= 1.0) t = 1.0;

        leftShoulderYawAngle = lerp(BLOCK_SHOULDER_YAW_TARGET, blockAnim.idle_shoulderYaw, t);
        leftShoulderPitchAngle = lerp(BLOCK_SHOULDER_PITCH_TARGET, blockAnim.idle_shoulderPitch, t);
        leftElbowAngle = lerp(BLOCK_ELBOW_TARGET, blockAnim.idle_elbow, t);
        leftWristAngle = lerp(BLOCK_WRIST_TARGET, blockAnim.idle_wrist, t);
        leftFingersCurlAngle = lerp(BLOCK_FINGERS_TARGET, blockAnim.idle_fingers, t);

        if (t >= 1.0) {
            blockAnim.state = BLOCK_IDLE;
            blockAnim.stateStartTime = now;
        }
        return;
    }
    }
}

// ======================================================= End Animation =================================================================
