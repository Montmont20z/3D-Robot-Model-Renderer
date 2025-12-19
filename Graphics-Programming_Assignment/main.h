// main.h
// Header file for main.cpp

#pragma once


GLUquadric* gluObject = nullptr;
struct Vec3 {
    float x, y, z;
};


// Forward declarations for functions used in main.cpp
void updateProjection(int width, int height);
void drawRightArm();
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


