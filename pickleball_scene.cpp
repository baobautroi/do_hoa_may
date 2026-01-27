/*
 * Enhanced Pickleball Park Scene
 * OpenGL/GLUT Implementation
 * 
 * Features:
 * - Realistic pickleball court with markings
 * - Detailed animated players with realistic movements
 * - Dynamic day-night lighting cycle with sun rays
 * - Swaying grass and trees in the wind
 * - Enhanced park environment (trees, benches, lamps)
 * - Interactive camera controls
 * 
 * Controls:
 * - Arrow Keys: Adjust time of day
 * - W/S: Zoom in/out
 * - A/D: Rotate camera left/right
 * - Q/E: Adjust camera height
 * - R/F: Adjust wind speed
 * - SPACE: Pause/Resume animations
 * - ESC: Exit
 */

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstdlib>  // for rand()
#include "ModelLoader.h"  // 3D Model loader with Assimp

// Constants
const float PI = 3.14159265359f;
const float COURT_LENGTH = 20.115f;  // Scaled 1.5x (was 13.41f)
const float COURT_WIDTH = 9.15f;     // Scaled 1.5x (was 6.10f)

// Out-of-bounds Area Margins (Global Settings)
const float MARGIN_X = 3.0f;  // Side margins (meters) - UNIFORM
const float MARGIN_Z = 10.0f;  // End margins (meters) - SCALED TO MATCH

// Global variables
float timeOfDay = 0.5f;  // 0=midnight, 0.5=noon, 1.0=midnight again

// Ball state for rally simulation - IMPROVED for continuous rally
float ballPosX = -3.5f;  
float ballPosY = 1.5f;   // Start well above net (0.914m)
float ballPosZ = 0.0f;   
float ballVelocityX = 0.08f;  // Moderate horizontal speed
float ballVelocityY = 0.03f;  // Gentle arc for consistent net clearance  
float ballVelocityZ = 0.0f;   
bool isPaused = false;

// Rally control
int rallyCount = 0;
int currentServer = 1;  // 1 or 2
float serveDelay = 0.0f;

// Smooth interpolation for animations
float targetArmSwing1 = 0.0f;
float targetArmSwing2 = 0.0f;
float smoothFactor = 0.15f;  // Interpolation speed

// Camera variables - ADJUSTED for symmetrical view
float cameraDistance = 25.0f;  // Increased for better overview
float cameraAngle = 0.0f;      // Front view (was 45° diagonal)
float cameraHeight = 15.0f;    // Higher for better perspective

// Animation variables
float playerSwing1 = 0.0f;
float playerSwing2 = 0.0f;
float windTime = 0.0f;
float windStrength = 1.0f;
float animationTime = 0.0f;

// 3D Model loaders - NEW!
ModelLoader treeModel;
ModelLoader paddleModel;
ModelLoader playerModel;
bool use3DModels = false;  // Will be set to true if models load successfully

// Player animation states
struct PlayerState {
    float legAngle1, legAngle2;
    float armSwing;
    float bodyTilt;
    float jumpHeight;
    // Movement
    float posX, posZ;        // Current position
    float targetX, targetZ;  // Target position (where ball will land)
    float moveSpeed;         // Movement speed
};

PlayerState player1State = {0, 0, 0, 0, 0, -COURT_LENGTH/4, 0, -COURT_LENGTH/4, 0, 0.08f};
PlayerState player2State = {0, 0, 0, 0, 0, COURT_LENGTH/4, 0, COURT_LENGTH/4, 0, 0.08f};

// Sky color structure
struct SkyColor {
    float r, g, b;
};

// Function to get sky color based on time of day - BRIGHTER
SkyColor getSkyColor(float t) {
    SkyColor color;
    if (t < 0.25f) {  // Night
        float fade = t / 0.25f;
        color.r = 0.15f + fade * 0.4f;
        color.g = 0.2f + fade * 0.5f;
        color.b = 0.3f + fade * 0.5f;
    } else if (t < 0.5f) {  // Dawn to noon
        float fade = (t - 0.25f) / 0.25f;
        color.r = 0.55f + fade * 0.3f;
        color.g = 0.7f + fade * 0.25f;
        color.b = 0.8f + fade * 0.18f;
    } else if (t < 0.75f) {  // Noon to dusk
        float fade = (t - 0.5f) / 0.25f;
        color.r = 0.85f + fade * 0.1f;
        color.g = 0.95f - fade * 0.35f;
        color.b = 0.98f - fade * 0.45f;
    } else {  // Dusk to night
        float fade = (t - 0.75f) / 0.25f;
        color.r = 0.95f - fade * 0.8f;
        color.g = 0.6f - fade * 0.4f;
        color.b = 0.53f - fade * 0.23f;
    }
    return color;
}

// Function to get sun position
void getSunPosition(float t, float &x, float &y, float &z) {
    float angle = t * 2.0f * PI;
    x = 30.0f * cos(angle);
    y = 30.0f * sin(angle);
    z = 0.0f;
}

// Function to set lighting based on time of day
void setupLighting() {
    float sunX, sunY, sunZ;
    getSunPosition(timeOfDay, sunX, sunY, sunZ);
    
    // Main sun light - FIXED position and stronger
    GLfloat lightPos[] = {sunX, sunY, sunZ, 1.0f};
    
    // MUCH BRIGHTER lighting
    float intensity = 1.0f;
    GLfloat lightColor[4];
    
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {  // Night
        intensity = 0.5f;  // Brighter night
        lightColor[0] = 0.6f * intensity;
        lightColor[1] = 0.6f * intensity;
        lightColor[2] = 0.8f * intensity;
    } else if (timeOfDay < 0.35f || timeOfDay > 0.65f) {  // Dawn/Dusk
        intensity = 0.9f;  // Very bright
        lightColor[0] = 1.0f * intensity;
        lightColor[1] = 0.8f * intensity;
        lightColor[2] = 0.6f * intensity;
    } else {  // Day - VERY BRIGHT
        intensity = 1.0f;
        lightColor[0] = 1.0f;
        lightColor[1] = 1.0f;
        lightColor[2] = 0.95f;
    }
    lightColor[3] = 1.0f;
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
    
    // CRITICAL: HIGH AMBIENT LIGHT to prevent black objects
    GLfloat ambient[] = {0.6f, 0.6f, 0.65f, 1.0f};  // Very bright ambient!
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    
    // Global ambient (scene-wide)
    GLfloat globalAmbient[] = {0.4f, 0.4f, 0.45f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    // Street lamps
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
        
        GLfloat lampColor[] = {1.0f, 0.9f, 0.7f, 1.0f};
        GLfloat lampPos1[] = {-COURT_LENGTH/2 - 3, 5.0f, COURT_WIDTH/2 + 3, 1.0f};
        GLfloat lampPos2[] = {COURT_LENGTH/2 + 3, 5.0f, -COURT_WIDTH/2 - 3, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, lampPos1);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lampColor);
        glLightfv(GL_LIGHT2, GL_POSITION, lampPos2);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, lampColor);
    } else {
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
    }
}

// Draw the court surface - FLAT COLOR
void drawCourt() {
    glPushMatrix();
    
    glColor3f(0.5f, 0.6f, 0.75f);  // Blue-gray court
    
    
    // Draw Out-of-bounds area (Safe zone) - COVERS THE GAP
    glPushMatrix();
    glColor3f(0.45f, 0.65f, 0.85f); // Light blue safe zone
    
    // Use Global Constants for perfect sync with grass
    float mx = MARGIN_X;
    float mz = MARGIN_Z;
    
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - mx, -0.01f, -COURT_WIDTH/2 - mz);
    glVertex3f(COURT_LENGTH/2 + mx, -0.01f, -COURT_WIDTH/2 - mz);
    glVertex3f(COURT_LENGTH/2 + mx, -0.01f, COURT_WIDTH/2 + mz);
    glVertex3f(-COURT_LENGTH/2 - mx, -0.01f, COURT_WIDTH/2 + mz);
    glEnd();
    glPopMatrix();
    
    
    // Main Court Surface
    glColor3f(0.3f, 0.4f, 0.65f);  // Darker Blue Royal Court
    
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2, 0, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0, COURT_WIDTH/2);
    glVertex3f(-COURT_LENGTH/2, 0, COURT_WIDTH/2);
    glEnd();
    
    // White lines - BRIGHT
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(4.0f);
    
    // Outer boundary
    glBegin(GL_LINE_LOOP);
    glVertex3f(-COURT_LENGTH/2, 0.01f, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0.01f, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0.01f, COURT_WIDTH/2);
    glVertex3f(-COURT_LENGTH/2, 0.01f, COURT_WIDTH/2);
    glEnd();
    
    // Center line
    glBegin(GL_LINES);
    glVertex3f(0, 0.01f, -COURT_WIDTH/2);
    glVertex3f(0, 0.01f, COURT_WIDTH/2);
    glEnd();
    
    // Service lines (non-volley zone - kitchen)
    float kitchenLine = 2.134f;  // 7 feet from net
    glBegin(GL_LINES);
    // Left kitchen line
    glVertex3f(-kitchenLine, 0.01f, -COURT_WIDTH/2);
    glVertex3f(-kitchenLine, 0.01f, COURT_WIDTH/2);
    // Right kitchen line
    glVertex3f(kitchenLine, 0.01f, -COURT_WIDTH/2);
    glVertex3f(kitchenLine, 0.01f, COURT_WIDTH/2);
    glEnd();
    
    // Center service lines (from kitchen to baseline)
    glBegin(GL_LINES);
    // Left side
    glVertex3f(-kitchenLine, 0.01f, 0);
    glVertex3f(-COURT_LENGTH/2, 0.01f, 0);
    // Right side
    glVertex3f(kitchenLine, 0.01f, 0);
    glVertex3f(COURT_LENGTH/2, 0.01f, 0);
    glEnd();
    
    glPopMatrix();
}

// Draw the net
void drawNet() {
    glPushMatrix();
    
    // Net posts
    GLfloat postMaterial[] = {0.3f, 0.3f, 0.3f, 1.0f};
    //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, postMaterial);
    
    glPushMatrix();
    glTranslatef(0, 0.5f, -COURT_WIDTH/2 - 0.1f);
    glScalef(0.05f, 1.0f, 0.05f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, 0.5f, COURT_WIDTH/2 + 0.1f);
    glScalef(0.05f, 1.0f, 0.05f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Net mesh - WHITE with emission
    GLfloat netColor[] = {1.0f, 1.0f, 1.0f, 0.9f};
    GLfloat netEmission[] = {0.3f, 0.3f, 0.3f, 1.0f};
    //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, netColor);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, netEmission);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBegin(GL_LINES);
    for (float z = -COURT_WIDTH/2; z <= COURT_WIDTH/2; z += 0.1f) {
        glVertex3f(0, 0, z);
        glVertex3f(0, 0.914f, z);
    }
    for (float y = 0; y <= 0.914f; y += 0.1f) {
        glVertex3f(0, y, -COURT_WIDTH/2);
        glVertex3f(0, y, COURT_WIDTH/2);
    }
    glEnd();
    
    // Reset emission
    GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
    
    glPopMatrix();
}

// Draw enhanced tree - with 3D model support!
void drawTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    float swayAngle = sin(windTime + x * 0.5f + z * 0.3f) * windStrength * 3.0f;
    
    // Try to use 3D model if loaded
    if (treeModel.getMeshCount() > 0) {
        // Apply wind sway to model
        glTranslatef(0, 0, 0);
        glRotatef(swayAngle, 0, 0, 1);
        
        // Render 3D tree model with appropriate scale
        treeModel.render(0, 0, 0, 1.5f);  // Scale 1.5x for visibility
    } else {
        // FALLBACK: Original geometric tree
        // Trunk - BRIGHT BROWN
        glColor3f(0.55f, 0.35f, 0.2f);  // Bright brown
        
        glPushMatrix();
        glTranslatef(0, 1.5f, 0);
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.35f, 0.25f, 3.0f, 16, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
        
        // Tree top with sway
        glPushMatrix();
        glTranslatef(0, 3.0f, 0);
        glRotatef(swayAngle, 0, 0, 1);
        
        // FOREST GREEN - BRIGHT!
        glColor3f(0.3f, 0.7f, 0.3f);  // Bright green
        
        glPushMatrix();
        glTranslatef(0, 0.0f, 0);
        glutSolidSphere(1.4f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.35f, 0.75f, 0.35f);  // Even brighter
        glPushMatrix();
        glTranslatef(0, 0.8f, 0);
        glutSolidSphere(1.1f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.4f, 0.8f, 0.4f);  // Lightest green (sun-lit)
        glPushMatrix();
        glTranslatef(0, 1.5f, 0);
        glutSolidSphere(0.8f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.3f, 0.7f, 0.3f);
        for (int i = 0; i < 5; i++) {
            float angle = i * 72.0f;
            glPushMatrix();
            glRotatef(angle, 0, 1, 0);
            glTranslatef(0.9f, 0.5f, 0);
            glutSolidSphere(0.4f, 12, 12);
            glPopMatrix();
        }
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw a bench
void drawBench(float x, float z, float rotation) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotation, 0, 1, 0);
    
    GLfloat benchColor[] = {0.5f, 0.3f, 0.15f, 1.0f};
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, benchColor);
    
    // Seat
    glPushMatrix();
    glTranslatef(0, 0.5f, 0);
    glScalef(1.5f, 0.1f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Back
    glPushMatrix();
    glTranslatef(0, 0.8f, -0.2f);
    glScalef(1.5f, 0.6f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Legs
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            glPushMatrix();
            glTranslatef(-0.6f + i * 1.2f, 0.25f, -0.15f + j * 0.3f);
            glScalef(0.1f, 0.5f, 0.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    
    glPopMatrix();
}

// Draw a street lamp
void drawStreetLamp(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Pole
    GLfloat poleColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, poleColor);
    
    glPushMatrix();
    glTranslatef(0, 2.5f, 0);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.08f, 0.1f, 5.0f, 12, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
    
    // Lamp head
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) {
        GLfloat lampColor[] = {1.0f, 0.9f, 0.6f, 1.0f};
        GLfloat emission[] = {0.8f, 0.7f, 0.4f, 1.0f};
        //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lampColor);
        //glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    } else {
        GLfloat lampColor[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, lampColor);
        //glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    }
    
    glPushMatrix();
    glTranslatef(0, 5.0f, 0);
    glutSolidSphere(0.3f, 12, 12);
    glPopMatrix();
    
    // Reset emission
    GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    //glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    
    glPopMatrix();
}

// Draw a fence section
void drawFence(float x, float z, float rotation) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotation, 0, 1, 0);
    
    glColor3f(0.45f, 0.35f, 0.25f);  // Brown wood
    
    // Posts
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(-0.5f + i * 0.5f, 0.5f, 0);
        glScalef(0.08f, 1.0f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    // Horizontal bars
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        glTranslatef(0, 0.3f + i * 0.4f, 0);
        glScalef(1.1f, 0.05f, 0.05f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw a path/walkway section
void drawPath(float x, float z, float width, float length, float rotation) {
    glPushMatrix();
    glTranslatef(x, 0.01f, z);
    glRotatef(rotation, 0, 1, 0);
    
    glColor3f(0.6f, 0.5f, 0.4f);  // Sandy path color
    
    glBegin(GL_QUADS);
    glVertex3f(-width/2, 0, -length/2);
    glVertex3f(width/2, 0, -length/2);
    glVertex3f(width/2, 0, length/2);
    glVertex3f(-width/2, 0, length/2);
    glEnd();
    
    glPopMatrix();
}

// Draw a bush (smaller than tree)
void drawBush(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    float swayAngle = sin(windTime + x * 0.3f) * windStrength * 2.0f;
    glRotatef(swayAngle, 0, 0, 1);
    
    // Dark green bush
    glColor3f(0.2f, 0.5f, 0.2f);
    
    glPushMatrix();
    glTranslatef(0, 0.4f, 0);
    glutSolidSphere(0.5f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.2f, 0.3f, 0);
    glutSolidSphere(0.35f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.2f, 0.3f, 0);
    glutSolidSphere(0.35f, 12, 12);
    glPopMatrix();
    
    glPopMatrix();
}

// Draw flowers
void drawFlowers(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Flower colors (random picks)
    float colors[][3] = {
        {1.0f, 0.2f, 0.3f},  // Red
        {1.0f, 0.8f, 0.2f},  // Yellow
        {0.8f, 0.3f, 0.9f},  // Purple
        {1.0f, 0.5f, 0.8f}   // Pink
    };
    
    for (int i = 0; i < 5; i++) {
        float angle = i * 72.0f;
        float radius = 0.15f;
        
        glPushMatrix();
        glTranslatef(radius * cos(angle * PI / 180.0f), 0.1f, 
                     radius * sin(angle * PI / 180.0f));
        
        // Stem
        glColor3f(0.2f, 0.6f, 0.2f);
        glPushMatrix();
        glScalef(0.02f, 0.15f, 0.02f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Flower head
        int colorIdx = i % 4;
        glColor3f(colors[colorIdx][0], colors[colorIdx][1], colors[colorIdx][2]);
        glPushMatrix();
        glTranslatef(0, 0.1f, 0);
        glutSolidSphere(0.05f, 8, 8);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw a decorative garden bed with multiple flower types
void drawGardenBed(float x, float z, float width, float length) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Garden soil base (dark brown)
    glColor3f(0.35f, 0.25f, 0.18f);
    glBegin(GL_QUADS);
    glVertex3f(-width/2, 0.0f, -length/2);
    glVertex3f(width/2, 0.0f, -length/2);
    glVertex3f(width/2, 0.0f, length/2);
    glVertex3f(-width/2, 0.0f, length/2);
    glEnd();
    
    // Garden edge/border (stone gray)
    glColor3f(0.5f, 0.5f, 0.55f);
    glLineWidth(4.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-width/2, 0.01f, -length/2);
    glVertex3f(width/2, 0.01f, -length/2);
    glVertex3f(width/2, 0.01f, length/2);
    glVertex3f(-width/2, 0.01f, length/2);
    glEnd();
    
    // Plant flowers in the bed
    int flowersPerRow = (int)(width / 0.4f);
    int rows = (int)(length / 0.4f);
    
    for (int i = 0; i < flowersPerRow; i++) {
        for (int j = 0; j < rows; j++) {
            float fx = -width/2 + (i + 0.5f) * (width / flowersPerRow);
            float fz = -length/2 + (j + 0.5f) * (length / rows);
            
            // Vary flower types
            int flowerType = (i + j) % 4;
            float colors[][3] = {
                {1.0f, 0.2f, 0.3f},  // Red
                {1.0f, 0.8f, 0.2f},  // Yellow
                {0.8f, 0.3f, 0.9f},  // Purple
                {1.0f, 0.5f, 0.8f}   // Pink
            };
            
            // Flower stem
            glColor3f(0.2f, 0.6f, 0.2f);
            glPushMatrix();
            glTranslatef(fx, 0.08f, fz);
            glScalef(0.02f, 0.16f, 0.02f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            // Flower head
            glColor3f(colors[flowerType][0], colors[flowerType][1], colors[flowerType][2]);
            glPushMatrix();
            glTranslatef(fx, 0.16f, fz);
            glutSolidSphere(0.06f, 8, 8);
            glPopMatrix();
        }
    }
    
    glPopMatrix();
}

// Draw a circular flower patch
void drawCircularFlowerPatch(float x, float z, float radius) {
    glPushMatrix();
    glTranslatef(x, 0.01f, z);
    
    // Draw circular patch of flowers
    int segments = 12;
    for (int i = 0; i < segments; i++) {
        float angle1 = (i * 2.0f * PI) / segments;
        float angle2 = ((i + 1) * 2.0f * PI) / segments;
        
        // Draw flowers in rings
        for (float r = 0.2f; r < radius; r += 0.3f) {
            float fx = cos(angle1) * r;
            float fz = sin(angle1) * r;
            
            int colorIdx = (i + (int)(r * 10)) % 4;
            float colors[][3] = {
                {1.0f, 0.2f, 0.3f},  // Red
                {1.0f, 0.8f, 0.2f},  // Yellow
                {0.8f, 0.3f, 0.9f},  // Purple
                {1.0f, 0.5f, 0.8f}   // Pink
            };
            
            // Stem
            glColor3f(0.2f, 0.6f, 0.2f);
            glPushMatrix();
            glTranslatef(fx, 0.05f, fz);
            glScalef(0.02f, 0.1f, 0.02f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            // Flower
            glColor3f(colors[colorIdx][0], colors[colorIdx][1], colors[colorIdx][2]);
            glPushMatrix();
            glTranslatef(fx, 0.1f, fz);
            glutSolidSphere(0.05f, 6, 6);
            glPopMatrix();
        }
    }
    
    glPopMatrix();
}

// Draw decorative rocks/stones
void drawRockCluster(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Draw several rocks of varying sizes
    float rockPositions[][3] = {
        {0, 0, 0},
        {0.3f, 0, 0.2f},
        {-0.2f, 0, 0.3f},
        {0.1f, 0, -0.25f},
        {-0.3f, 0, -0.1f}
    };
    
    for (int i = 0; i < 5; i++) {
        float grayShade = 0.45f + (i * 0.05f);
        glColor3f(grayShade, grayShade, grayShade + 0.05f);
        
        glPushMatrix();
        glTranslatef(rockPositions[i][0], 0.08f, rockPositions[i][2]);
        glScalef(1.0f, 0.6f, 0.8f);
        glutSolidSphere(0.15f - i * 0.02f, 8, 8);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw a picnic table
void drawPicnicTable(float x, float z, float rotation) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotation, 0, 1, 0);
    
    // Wood color
    glColor3f(0.55f, 0.4f, 0.25f);
    
    // Table top
    glPushMatrix();
    glTranslatef(0, 0.75f, 0);
    glScalef(2.0f, 0.08f, 1.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Table legs
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            glPushMatrix();
            glTranslatef(-0.8f + i * 1.6f, 0.35f, -0.35f + j * 0.7f);
            glScalef(0.1f, 0.7f, 0.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    
    // Benches (2 sides)
    glColor3f(0.5f, 0.35f, 0.2f);
    for (int side = 0; side < 2; side++) {
        float zOffset = side == 0 ? 0.7f : -0.7f;
        
        // Bench seat
        glPushMatrix();
        glTranslatef(0, 0.45f, zOffset);
        glScalef(1.8f, 0.06f, 0.35f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Bench legs
        for (int i = 0; i < 2; i++) {
            glPushMatrix();
            glTranslatef(-0.7f + i * 1.4f, 0.2f, zOffset);
            glScalef(0.08f, 0.4f, 0.08f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
    
    glPopMatrix();
}

// Draw a trash bin
void drawTrashBin(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Bin body - dark gray
    glColor3f(0.3f, 0.3f, 0.3f);
    glPushMatrix();
    glTranslatef(0, 0.35f, 0);
    glScalef(0.3f, 0.7f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Lid - lighter gray
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
    glTranslatef(0, 0.75f, 0);
    glScalef(0.35f, 0.05f, 0.35f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix();
}

// Draw a signpost
void drawSignpost(float x, float z, const char* text) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Pole
    glColor3f(0.4f, 0.3f, 0.2f);
    glPushMatrix();
    glTranslatef(0, 0.75f, 0);
    glScalef(0.08f, 1.5f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Sign board
    glColor3f(0.9f, 0.85f, 0.7f);  // Light wood
    glPushMatrix();
    glTranslatef(0, 1.3f, 0);
    glScalef(0.8f, 0.4f, 0.05f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix();
}

// Draw a small fountain
void drawFountain(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Base - stone gray
    glColor3f(0.6f, 0.6f, 0.65f);
    glPushMatrix();
    glTranslatef(0, 0.2f, 0);
    glScalef(1.2f, 0.4f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Middle tier
    glPushMatrix();
    glTranslatef(0, 0.5f, 0);
    glScalef(0.8f, 0.15f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Central pillar
    glPushMatrix();
    glTranslatef(0, 0.75f, 0);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.1f, 0.08f, 0.5f, 12, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
    
    // Water (light blue)
    glColor3f(0.4f, 0.7f, 0.9f);
    glPushMatrix();
    glTranslatef(0, 0.41f, 0);
    glScalef(0.7f, 0.02f, 0.7f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Water jet (animated)
    float jetHeight = 0.3f + 0.1f * sin(windTime * 3.0f);
    glPushMatrix();
    glTranslatef(0, 1.0f + jetHeight/2, 0);
    glScalef(0.05f, jetHeight, 0.05f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix();
}

// Draw clouds
void drawCloud(float x, float y, float z, float scale) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);
    
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);  // White with transparency
    
    // Cloud is made of several spheres
    for (int i = 0; i < 5; i++) {
        glPushMatrix();
        glTranslatef(-1.0f + i * 0.5f, sin(i * 0.8f) * 0.2f, 0);
        glutSolidSphere(0.5f, 12, 12);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw realistic pickleball paddle face - flat oval/rectangular shape
void drawPaddleFace(float width, float height, float thickness) {
    const int segments = 24;
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;
    float halfT = thickness / 2.0f;
    float cornerRadius = 0.08f;
    
    // Helper to get rounded rectangle point
    auto getRoundedRectPoint = [&](int i, float& x, float& y) {
        float quadrant = floor(i / (float)segments);
        float localAngle = (i % segments) / (float)segments * PI / 2.0f;
        
        if (quadrant == 0) { // Top-right
            x = (halfW - cornerRadius) + cornerRadius * cos(localAngle);
            y = (halfH - cornerRadius) + cornerRadius * sin(localAngle);
        } else if (quadrant == 1) { // Top-left  
            x = -(halfW - cornerRadius) - cornerRadius * sin(localAngle);
            y = (halfH - cornerRadius) + cornerRadius * cos(localAngle);
        } else if (quadrant == 2) { // Bottom-left
            x = -(halfW - cornerRadius) - cornerRadius * cos(localAngle);
            y = -(halfH - cornerRadius) - cornerRadius * sin(localAngle);
        } else { // Bottom-right
            x = (halfW - cornerRadius) + cornerRadius * sin(localAngle);
            y = -(halfH - cornerRadius) - cornerRadius * cos(localAngle);
        }
    };
    
    // Front face - flat with rounded corners
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, halfT);
    
    for (int i = 0; i <= segments * 4; i++) {
        float x, y;
        getRoundedRectPoint(i, x, y);
        glVertex3f(x, y, halfT);
    }
    glEnd();
    
    // Back face (mirror of front)
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0, -halfT);
    
    for (int i = segments * 4; i >= 0; i--) {
        float x, y;
        getRoundedRectPoint(i, x, y);
        glVertex3f(x, y, -halfT);
    }
    glEnd();
    
    // Edge strip connecting front and back
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments * 4; i++) {
        float x, y;
        getRoundedRectPoint(i, x, y);
        
        // Approximate outward normal
        float nx = x / halfW * 0.8f;
        float ny = y / halfH * 0.8f;
        float len = sqrt(nx*nx + ny*ny);
        if (len > 0.01f) {
            nx /= len;
            ny /= len;
        }
        
        glNormal3f(nx, ny, 0);
        glVertex3f(x, y, halfT);
        glVertex3f(x, y, -halfT);
    }
    glEnd();
}

// Draw detailed realistic player with animations
void drawPlayer(float x, float z, PlayerState& state, bool isPlayer1) {
    glPushMatrix();
    glTranslatef(x, state.jumpHeight, z);
    
    // Face TOWARD THE NET (perpendicular to court length)
    if (isPlayer1) {
        // Player 1 on LEFT side: Face RIGHT toward net
        glRotatef(90, 0, 1, 0);  
    } else {
        // Player 2 on RIGHT side: Face LEFT toward net
        glRotatef(-90, 0, 1, 0);  
    }
    
    // Additional rotation to face ball (when hitting)
    if (isPlayer1) {
        float ballDir = atan2(ballPosZ - z, ballPosX - x) * 180.0f / PI;
        glRotatef(ballDir - 90, 0, 1, 0);  // Turn toward ball
    } else {
        float ballDir = atan2(ballPosZ - z, ballPosX - x) * 180.0f / PI;
        glRotatef(ballDir + 90, 0, 1, 0);  // Turn toward ball
    }
    
    // Body tilt for dynamic pose
    glRotatef(state.bodyTilt, 0, 0, 1);
    
    // === LEGS ===
    GLfloat pantsColor[] = {0.2f, 0.3f, 0.6f, 1.0f};  // Blue shorts
    glColor3f(0.2f, 0.3f, 0.6f);
    
    // Left leg
    glPushMatrix();
    glTranslatef(-0.15f, 0.7f, 0);
    glRotatef(state.legAngle1, 1, 0, 0);
    glTranslatef(0, -0.35f, 0);
    
    // Thigh
    glPushMatrix();
    glScalef(0.18f, 0.4f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Knee
    GLfloat skinColor[] = {0.85f, 0.7f, 0.6f, 1.0f};
    glColor3f(0.85f, 0.7f, 0.6f);
    glPushMatrix();
    glTranslatef(0, -0.2f, 0);
    glutSolidSphere(0.12f, 12, 12);
    glPopMatrix();
    
    // Lower leg
    glPushMatrix();
    glTranslatef(0, -0.5f, 0);
    glScalef(0.15f, 0.35f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Shoe
    GLfloat shoeColor[] = {0.9f, 0.9f, 0.9f, 1.0f};  // White shoes
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0, -0.72f, 0.08f);
    glScalef(0.18f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix(); // End left leg
    
    // Right leg (similar structure)
    glColor3f(0.2f, 0.3f, 0.6f);
    glPushMatrix();
    glTranslatef(0.15f, 0.7f, 0);
    glRotatef(state.legAngle2, 1, 0, 0);
    glTranslatef(0, -0.35f, 0);
    
    glPushMatrix();
    glScalef(0.18f, 0.4f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, skinColor);
    glPushMatrix();
    glTranslatef(0, -0.2f, 0);
    glutSolidSphere(0.12f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, -0.5f, 0);
    glScalef(0.15f, 0.35f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, shoeColor);
    glPushMatrix();
    glTranslatef(0, -0.72f, 0.08f);
    glScalef(0.18f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix(); // End right leg
    
    // === TORSO ===
    GLfloat shirtColor[] = {0.9f, 0.3f, 0.2f, 1.0f};  // Red shirt
    glColor3f(0.9f, 0.3f, 0.2f);
    
    glPushMatrix();
    glTranslatef(0, 1.3f, 0);
    glScalef(0.5f, 0.7f, 0.28f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === NECK ===
    glColor3f(0.85f, 0.7f, 0.6f);
    glPushMatrix();
    glTranslatef(0, 1.75f, 0);
    glScalef(0.15f, 0.15f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === HEAD ===
    glPushMatrix();
    glTranslatef(0, 1.95f, 0);
    
    // Face
    glutSolidSphere(0.22f, 16, 16);
    
    // Eyes
    GLfloat eyeColor[] = {0.1f, 0.1f, 0.1f, 1.0f};
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, eyeColor);
    glPushMatrix();
    glTranslatef(-0.08f, 0.05f, 0.18f);
    glutSolidSphere(0.03f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, 0.18f);
    glutSolidSphere(0.03f, 8, 8);
    glPopMatrix();
    
    // Hair
    GLfloat hairColor[] = {0.2f, 0.15f, 0.1f, 1.0f};
    glColor3f(0.2f, 0.15f, 0.1f);
    glPushMatrix();
    glTranslatef(0, 0.15f, 0);
    glScalef(1.1f, 0.8f, 1.0f);
    glutSolidSphere(0.22f, 12, 12);
    glPopMatrix();
    
    glPopMatrix(); // End head
    
    // === ARMS ===
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, skinColor);
    
    // Left arm (non-paddle arm)
    glPushMatrix();
    glTranslatef(-0.35f, 1.5f, 0);
    glRotatef(-20, 0, 0, 1);
    glRotatef(15, 1, 0, 0);
    glTranslatef(0, -0.25f, 0);
    
    // Upper arm
    glPushMatrix();
    glScalef(0.12f, 0.3f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Elbow
    glPushMatrix();
    glTranslatef(0, -0.18f, 0);
    glutSolidSphere(0.08f, 10, 10);
    glPopMatrix();
    
    // Forearm
    glPushMatrix();
    glTranslatef(0, -0.4f, 0);
    glScalef(0.1f, 0.25f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Hand
    glPushMatrix();
    glTranslatef(0, -0.58f, 0);
    glutSolidSphere(0.08f, 10, 10);
    glPopMatrix();
    
    glPopMatrix(); // End left arm
    
    // Right arm (paddle arm) with swing animation
    glPushMatrix();
    glTranslatef(0.35f, 1.5f, 0);
    
    // More realistic arm positioning for pickleball
    glRotatef(state.armSwing, 1, 0, 0);  // Swing motion (unchanged)
    glRotatef(30, 0, 0, 1);     // Angle outward more (was 20)
    glRotatef(-15, 0, 1, 0);    // Rotate arm forward slightly (NEW)
    
    glTranslatef(0, -0.25f, 0);
    
    // Upper arm
    glPushMatrix();
    glScalef(0.12f, 0.3f, 0.12f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Elbow
    glPushMatrix();
    glTranslatef(0, -0.18f, 0);
    glutSolidSphere(0.08f, 10, 10);
    glPopMatrix();
    
    // Forearm
    glPushMatrix();
    glTranslatef(0, -0.4f, 0);
    glScalef(0.1f, 0.25f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Hand
    glPushMatrix();
    glTranslatef(0, -0.58f, 0);
    glutSolidSphere(0.08f, 10, 10);
    
    // === ENHANCED REALISTIC PADDLE ===
    GLfloat paddleColor[] = {0.98f, 0.35f, 0.15f, 1.0f};  // Bright orange-red
    GLfloat paddleEdge[] = {0.15f, 0.15f, 0.15f, 1.0f};   // Dark edge
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, paddleColor);
    
    // Grip/Handle - textured with segments
    GLfloat gripColor[] = {0.2f, 0.2f, 0.2f, 1.0f};  // Black grip
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gripColor);
    
    // Handle base
    glPushMatrix();
    glTranslatef(0, -0.12f, 0);
    glScalef(0.065f, 0.3f, 0.065f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Grip wrapping (3 segments)
    for (int i = 0; i < 3; i++) {
        glPushMatrix();
        glTranslatef(0, -0.08f - i * 0.08f, 0);
        glutSolidTorus(0.008f, 0.04f, 6, 12);
        glPopMatrix();
    }
    
    // Paddle face - realistic pickleball paddle shape
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, paddleColor);
    
    glPushMatrix();
    glTranslatef(0, -0.38f, 0);
    
    // REALISTIC GRIP ANGLES
    // Rotate to make paddle face forward (like holding a frying pan)
    glRotatef(90, 1, 0, 0);    // First: make paddle perpendicular to arm
    glRotatef(-20, 0, 1, 0);   // Second: tilt slightly inward (continental grip)
    glRotatef(10, 1, 0, 0);    // Third: angle forward slightly (ready position)
    
    // Main paddle surface - REALISTIC FLAT FACE
    glColor3f(0.98f, 0.35f, 0.15f);  // Bright orange-red paddle
    drawPaddleFace(0.42f, 0.52f, 0.03f); // width, height, thin thickness
    
    // Edge guard (black rim outline)
    glColor3f(0.15f, 0.15f, 0.15f);  // Dark edge
    glLineWidth(3.0f);
    
    // Draw outline around paddle perimeter
    const int outlineSegs = 24;
    float halfW = 0.21f;  // 0.42 / 2
    float halfH = 0.26f;  // 0.52 / 2
    float cornerRadius = 0.08f;
    
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < outlineSegs * 4; i++) {
        float quadrant = floor(i / (float)outlineSegs);
        float localAngle = (i % outlineSegs) / (float)outlineSegs * PI / 2.0f;
        
        float x, y;
        if (quadrant == 0) { // Top-right
            x = (halfW - cornerRadius) + cornerRadius * cos(localAngle);
            y = (halfH - cornerRadius) + cornerRadius * sin(localAngle);
        } else if (quadrant == 1) { // Top-left
            x = -(halfW - cornerRadius) - cornerRadius * sin(localAngle);
            y = (halfH - cornerRadius) + cornerRadius * cos(localAngle);
        } else if (quadrant == 2) { // Bottom-left
            x = -(halfW - cornerRadius) - cornerRadius * cos(localAngle);
            y = -(halfH - cornerRadius) - cornerRadius * sin(localAngle);
        } else { // Bottom-right
            x = (halfW - cornerRadius) + cornerRadius * sin(localAngle);
            y = -(halfH - cornerRadius) - cornerRadius * cos(localAngle);
        }
        
        glVertex3f(x, y, 0.016f);  // Slightly in front
    }
    glEnd();
    
    // Optional: Honeycomb texture pattern
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.2f);  // Dark gray pattern
    glLineWidth(1.0f);
    
    // Simplified hexagon pattern
    for (float py = -0.2f; py <= 0.2f; py += 0.08f) {
        for (float px = -0.15f; px <= 0.15f; px += 0.07f) {
            // Draw small hexagon
            glBegin(GL_LINE_LOOP);
            for (int h = 0; h < 6; h++) {
                float angle = h * PI / 3.0f;
                float hx = px + 0.02f * cos(angle);
                float hy = py + 0.02f * sin(angle);
                glVertex3f(hx, hy, 0.017f);
            }
            glEnd();
        }
    }
    
    glEnable(GL_LIGHTING);
    
    glPopMatrix();  // End paddle face
    
    glPopMatrix(); // End hand with paddle
    
    glPopMatrix(); // End right arm
    
    glPopMatrix(); // End player
}

// Draw improved ground with varied park terrain
void drawGrassField() {
    // === DRAW BASE GROUND PLANE (similar to park terrain) ===
    glPushMatrix();
    
    // Draw ground in sections with different colors for variety
    for (float x = -COURT_LENGTH - 10; x < COURT_LENGTH + 10; x += 2.0f) {
        for (float z = -COURT_WIDTH - 10; z < COURT_WIDTH + 10; z += 2.0f) {
            // Skip the court area - AUTOMATICALLY SYNCED with margins!
            // We subtract a small amount (0.2f) to ensure slight overlap (no gaps)
            if (x > -COURT_LENGTH/2 - (MARGIN_X - 0.2f) && x < COURT_LENGTH/2 + (MARGIN_X - 0.2f) &&
                z > -COURT_WIDTH/2 - (MARGIN_Z - 0.2f) && z < COURT_WIDTH/2 + (MARGIN_Z - 0.2f)) {
                continue;
            }
            
            // Determine ground type based on position
            float distFromCenter = sqrt(x*x + z*z);
            float noiseVal = sin(x * 0.5f) * cos(z * 0.3f);
            
            // Varied ground colors - grass, dirt patches, lighter grass
            if (noiseVal > 0.3f) {
                // Lighter grass patches (sunlit)
                glColor3f(0.4f, 0.65f, 0.35f);
            } else if (noiseVal < -0.2f) {
                // Darker grass (shaded)
                glColor3f(0.25f, 0.5f, 0.25f);
            } else {
                // Medium grass
                glColor3f(0.32f, 0.58f, 0.3f);
            }
            
            // Draw ground quad
            glBegin(GL_QUADS);
            glVertex3f(x, 0.0f, z);
            glVertex3f(x + 2.0f, 0.0f, z);
            glVertex3f(x + 2.0f, 0.0f, z + 2.0f);
            glVertex3f(x, 0.0f, z + 2.0f);
            glEnd();
        }
    }
    
    glPopMatrix();
    
    // === DRAW DECORATIVE GRASS BLADES (sparse for performance) ===
    for (float x = -COURT_LENGTH - 8; x < COURT_LENGTH + 8; x += 1.2f) {
        for (float z = -COURT_WIDTH - 8; z < COURT_WIDTH + 8; z += 1.2f) {
            // Skip the court area - AUTOMATICALLY SYNCED!
            if (x > -COURT_LENGTH/2 - (MARGIN_X - 0.5f) && x < COURT_LENGTH/2 + (MARGIN_X - 0.5f) &&
                z > -COURT_WIDTH/2 - (MARGIN_Z - 0.5f) && z < COURT_WIDTH/2 + (MARGIN_Z - 0.5f)) {
                continue;
            }
            
            // Wind sway calculation
            float swayX = sin(windTime * 2.0f + x * 0.3f + z * 0.2f) * windStrength * 0.15f;
            float swayZ = cos(windTime * 1.5f + x * 0.2f + z * 0.3f) * windStrength * 0.1f;
            
            // Vary grass blade color
            float colorVar = sin(x * 10 + z * 10) * 0.08f;
            glColor3f(0.28f + colorVar, 0.6f + colorVar, 0.28f + colorVar);
            
            glPushMatrix();
            glTranslatef(x, 0, z);
            glRotatef(swayX * 5.0f, 0, 0, 1);
            glRotatef(swayZ * 5.0f, 1, 0, 0);
            
            // Grass blade
            glPushMatrix();
            glTranslatef(swayX, 0.12f, swayZ);
            glScalef(0.03f, 0.24f, 0.03f);
            glutSolidCube(1.0f);
            glPopMatrix();
            
            glPopMatrix();
        }
    }
}

// Draw sun with rays for atmosphere
void drawSun() {
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) return; // Only draw during day
    
    float sunX, sunY, sunZ;
    getSunPosition(timeOfDay, sunX, sunY, sunZ);
    
    if (sunY < 0) return; // Sun is below horizon
    
    glDisable(GL_LIGHTING);
    
    // Sun body
    glPushMatrix();
    glTranslatef(sunX, sunY, sunZ);
    
    // Sun glow
    glColor4f(1.0f, 0.95f, 0.7f, 0.3f);
    glutSolidSphere(3.0f, 20, 20);
    
    // Sun core
    glColor4f(1.0f, 1.0f, 0.8f, 1.0f);
    glutSolidSphere(2.0f, 20, 20);
    
    // Sun rays
    glColor4f(1.0f, 0.95f, 0.6f, 0.2f);
    for (int i = 0; i < 12; i++) {
        float angle = i * 30.0f;
        glPushMatrix();
        glRotatef(angle + windTime * 10.0f, 0, 0, 1);
        glTranslatef(2.5f, 0, 0);
        glScalef(1.5f, 0.2f, 0.2f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
}

// Draw the ball
void drawBall() {
    glPushMatrix();
    glTranslatef(ballPosX, ballPosY, ballPosZ);
    
    GLfloat ballColor[] = {1.0f, 0.9f, 0.1f, 1.0f};
    //glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, ballColor);
    
    glutSolidSphere(0.15f, 16, 16);
    
    glPopMatrix();
}

// Calculate paddle position in world space
void getPaddlePosition(PlayerState& state, bool isPlayer1, float& paddleX, float& paddleY, float& paddleZ) {
    // Paddle is attached to right arm, extended forward
    float armExtension = 0.6f;  // How far paddle extends from body
    
    if (isPlayer1) {
        paddleX = state.posX + armExtension * cos(state.armSwing * PI / 180.0f);
        paddleY = 1.2f + state.jumpHeight + 0.3f * sin(state.armSwing * PI / 180.0f);
        paddleZ = state.posZ;
    } else {
        paddleX = state.posX - armExtension * cos(state.armSwing * PI / 180.0f);
        paddleY = 1.2f + state.jumpHeight + 0.3f * sin(state.armSwing * PI / 180.0f);
        paddleZ = state.posZ;
    }
}

// Check if ball hits paddle
bool checkPaddleHit(float paddleX, float paddleY, float paddleZ) {
    float dx = ballPosX - paddleX;
    float dy = ballPosY - paddleY;
    float dz = ballPosZ - paddleZ;
    float distance = sqrt(dx*dx + dy*dy + dz*dz);
    
    const float PADDLE_RADIUS = 0.5f;  // Paddle hit zone
    return distance < PADDLE_RADIUS;
}

// Update ball physics - CONTINUOUS RALLY without going out of bounds
void updateBall() {
    if (isPaused) return;
    
    animationTime += 0.016f;
    windTime += 0.02f * windStrength;
    
    // Ball physics with gravity
    ballPosX += ballVelocityX;
    ballPosY += ballVelocityY;
    ballPosZ += ballVelocityZ;
    ballVelocityY -= 0.008f;  // Gravity
    
    // === PLAYER MOVEMENT - Chase the ball intelligently ===
    
    // Player 1 movement (left side) - moves to intercept
    if (ballVelocityX < 0) {  // Ball coming to player 1
        player1State.targetX = ballPosX - 0.8f;  // Position to hit
        player1State.targetZ = ballPosZ;         // Track ball Z
        
        // Keep in left side
        if (player1State.targetX > -1.0f) player1State.targetX = -1.0f;
        if (player1State.targetX < -COURT_LENGTH/2 + 0.5f) player1State.targetX = -COURT_LENGTH/2 + 0.5f;
        if (abs(player1State.targetZ) > COURT_WIDTH/2 - 0.5f) {
            player1State.targetZ = (player1State.targetZ > 0 ? 1 : -1) * (COURT_WIDTH/2 - 0.5f);
        }
    } else {
        // Return to center ready position
        player1State.targetX = -COURT_LENGTH/4;
        player1State.targetZ = 0;
    }
    
    // Smooth movement
    player1State.posX += (player1State.targetX - player1State.posX) * 0.12f;
    player1State.posZ += (player1State.targetZ - player1State.posZ) * 0.12f;
    
    // Player 2 movement (right side)
    if (ballVelocityX > 0) {  // Ball coming to player 2
        player2State.targetX = ballPosX + 0.8f;
        player2State.targetZ = ballPosZ;
        
        // Keep in right side
        if (player2State.targetX < 1.0f) player2State.targetX = 1.0f;
        if (player2State.targetX > COURT_LENGTH/2 - 0.5f) player2State.targetX = COURT_LENGTH/2 - 0.5f;
        if (abs(player2State.targetZ) > COURT_WIDTH/2 - 0.5f) {
            player2State.targetZ = (player2State.targetZ > 0 ? 1 : -1) * (COURT_WIDTH/2 - 0.5f);
        }
    } else {
        player2State.targetX = COURT_LENGTH/4;
        player2State.targetZ = 0;
    }
    
    player2State.posX += (player2State.targetX - player2State.posX) * 0.12f;
    player2State.posZ += (player2State.targetZ - player2State.posZ) * 0.12f;
    
    // === PADDLE COLLISION DETECTION ===
    
    float paddle1X, paddle1Y, paddle1Z;
    float paddle2X, paddle2Y, paddle2Z;
    getPaddlePosition(player1State, true, paddle1X, paddle1Y, paddle1Z);
    getPaddlePosition(player2State, false, paddle2X, paddle2Y, paddle2Z);
    
    // Player 1 paddle hit (ball going left)
    if (ballVelocityX < 0 && ballPosX < -0.5f && checkPaddleHit(paddle1X, paddle1Y, paddle1Z)) {
        // HIT! Send to player 2 with proper arc
        ballPosX = paddle1X + 0.5f;
        ballVelocityX = 0.09f + (rand() % 20) * 0.001f;   // To player 2, slight variation
        ballVelocityY = 0.18f + (rand() % 10) * 0.005f;  // Arc over net
        ballVelocityZ = (rand() % 5 - 2) * 0.01f;        // Slight side angle
        rallyCount++;
        
        // Swing animation
        targetArmSwing1 = 70.0f;
        player1State.jumpHeight = 0.2f;
        player1State.bodyTilt = -15.0f;
        
        printf("✓ Player 1 HIT! Rally: %d\n", rallyCount);
    }
    
    // Player 2 paddle hit (ball going right)
    if (ballVelocityX > 0 && ballPosX > 0.5f && checkPaddleHit(paddle2X, paddle2Y, paddle2Z)) {
        // HIT! Send to player 1 with proper arc
        ballPosX = paddle2X - 0.5f;
        ballVelocityX = -0.09f - (rand() % 20) * 0.001f;  // To player 1
        ballVelocityY = 0.18f + (rand() % 10) * 0.005f;   // Arc over net
        ballVelocityZ = (rand() % 5 - 2) * 0.01f;
        rallyCount++;
        
        targetArmSwing2 = 70.0f;
        player2State.jumpHeight = 0.2f;
        player2State.bodyTilt = 15.0f;
        
        printf("✓ Player 2 HIT! Rally: %d\n", rallyCount);
    }
    
    // === COURT BOUNDARIES - Keep ball in play! ===
    
    // Bounce on ground (with energy loss)
    if (ballPosY < 0.15f && ballVelocityY < 0) {
        ballPosY = 0.15f;
        ballVelocityY = -ballVelocityY * 0.6f;  // Bouncy!
    }
    
    // Net collision - if ball hits net, bounce it back gently
    const float NET_HEIGHT = 0.914f;
    if (abs(ballPosX) < 0.2f && ballPosY < NET_HEIGHT && ballPosY > 0) {
        // Net hit! Bounce back
        ballVelocityX *= -0.5f;
        ballVelocityY = 0.12f;  // Pop up
        ballPosX = (ballPosX > 0) ? 0.25f : -0.25f;
        printf("⚠ Net hit! Rally continues...\n");
    }
    
    // Side boundaries - bounce off sides to keep in play
    if (abs(ballPosZ) > COURT_WIDTH/2 - 0.3f) {
        float sign = (ballPosZ > 0) ? 1.0f : -1.0f;
        ballPosZ = sign * (COURT_WIDTH/2 - 0.3f);
        ballVelocityZ *= -0.8f;  // Bounce inward
    }
    
    // PREVENT going out! If ball goes too far, auto-return it
    if (ballPosX < -COURT_LENGTH/2) {
        // Too far left - bounce back toward player 2
        ballPosX = -COURT_LENGTH/2 + 0.3f;
        ballVelocityX = abs(ballVelocityX) * 0.8f;  // Reverse direction
        ballVelocityY = 0.15f;  // Pop up
        printf("⚠ Ball bounced off back wall (left)\n");
    }
    
    if (ballPosX > COURT_LENGTH/2) {
        // Too far right - bounce back toward player 1
        ballPosX = COURT_LENGTH/2 - 0.3f;
        ballVelocityX = -abs(ballVelocityX) * 0.8f;  // Reverse direction
        ballVelocityY = 0.15f;
        printf("⚠ Ball bounced off back wall (right)\n");
    }
    
    // If ball goes too high or too low (emergency reset)
    if (ballPosY > 6.0f || ballPosY < -0.5f) {
        printf("⚠ Emergency reset - ball too high/low\n");
        // Gentle reset to current server
        ballPosX = (currentServer == 1) ? -3.0f : 3.0f;
        ballPosY = 1.5f;
        ballPosZ = 0.0f;
        ballVelocityX = (currentServer == 1) ? 0.08f : -0.08f;
        ballVelocityY = 0.03f;
        ballVelocityZ = 0.0f;
        currentServer = (currentServer == 1) ? 2 : 1;  // Alternate
    }
    
    // === SMOOTH ANIMATIONS ===
    
    // Arm swing decay
    player1State.armSwing += (targetArmSwing1 - player1State.armSwing) * smoothFactor;
    player2State.armSwing += (targetArmSwing2 - player2State.armSwing) * smoothFactor;
    targetArmSwing1 *= 0.90f;  // Decay
    targetArmSwing2 *= 0.90f;
    
    // Walking/running animation based on movement speed
    float moveSpeed1 = sqrt(pow(player1State.posX - player1State.targetX, 2) + 
                            pow(player1State.posZ - player1State.targetZ, 2));
    float moveSpeed2 = sqrt(pow(player2State.posX - player2State.targetX, 2) + 
                            pow(player2State.posZ - player2State.targetZ, 2));
    
    if (moveSpeed1 > 0.08f) {
        // Running!
        player1State.legAngle1 = sin(animationTime * 20.0f) * 35.0f;
        player1State.legAngle2 = -player1State.legAngle1;
    } else if (moveSpeed1 > 0.03f) {
        // Walking
        player1State.legAngle1 = sin(animationTime * 12.0f) * 20.0f;
        player1State.legAngle2 = -player1State.legAngle1;
    } else {
        // Idle stance
        player1State.legAngle1 = sin(animationTime * 2.0f) * 5.0f;
        player1State.legAngle2 = -player1State.legAngle1 * 0.5f;
    }
    
    if (moveSpeed2 > 0.08f) {
        player2State.legAngle1 = sin(animationTime * 20.0f) * 35.0f;
        player2State.legAngle2 = -player2State.legAngle1;
    } else if (moveSpeed2 > 0.03f) {
        player2State.legAngle1 = sin(animationTime * 12.0f) * 20.0f;
        player2State.legAngle2 = -player2State.legAngle1;
    } else {
        player2State.legAngle1 = sin(animationTime * 2.0f) * 5.0f;
        player2State.legAngle2 = -player2State.legAngle1 * 0.5f;
    }
    
    // Decay jump and body tilt smoothly
    player1State.bodyTilt *= 0.85f;
    player1State.jumpHeight *= 0.85f;
    player2State.bodyTilt *= 0.85f;
    player2State.jumpHeight *= 0.85f;
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set sky color - BRIGHT SKY BLUE!
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Sky blue!
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Camera position - Look at center of court
    float camX = cameraDistance * cos(cameraAngle * PI / 180.0f);
    float camZ = cameraDistance * sin(cameraAngle * PI / 180.0f);
    gluLookAt(camX, cameraHeight, camZ,  // Camera position
              0, 0, 0,                    // Look at center of court (FIXED!)
              0, 1, 0);                   // Up vector
    
    // Update lighting based on time of day - CRITICAL!
    setupLighting();
    
    // Draw scene elements
    drawGrassField();  // Draw grass first (background)
    drawSun();         // Draw sun with rays
    drawCourt();
    drawNet();
    drawBall();
    
    // Draw players using DYNAMIC POSITIONS
    drawPlayer(player1State.posX, player1State.posZ, player1State, true);   
    drawPlayer(player2State.posX, player2State.posZ, player2State, false);
    
    // Draw park scenery - BEAUTIFUL ENHANCED PARK ATMOSPHERE! 🌳🌸
    
    // === PATHWAYS - Well-connected walking paths ===
    // Main path around the court
    drawPath(0, -COURT_WIDTH/2 - 2.5f, 2.0f, 20.0f, 0);  // Bottom path
    drawPath(0, COURT_WIDTH/2 + 2.5f, 2.0f, 20.0f, 0);   // Top path
    drawPath(-COURT_LENGTH/2 - 2.5f, 0, 2.0f, 15.0f, 90); // Left path
    drawPath(COURT_LENGTH/2 + 2.5f, 0, 2.0f, 15.0f, 90);  // Right path
    
    // Diagonal decorative paths
    drawPath(-COURT_LENGTH/2 - 5, -COURT_WIDTH/2 - 5, 1.5f, 8.0f, 45);
    drawPath(COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 5, 1.5f, 8.0f, 45);
    
    // === FENCES - Beautiful wooden fencing ===
    // Bottom fence line
    for (float x = -COURT_LENGTH/2 - 8; x <= COURT_LENGTH/2 + 8; x += 1.2f) {
        drawFence(x, -COURT_WIDTH/2 - 5, 0);
    }
    // Top fence line
    for (float x = -COURT_LENGTH/2 - 8; x <= COURT_LENGTH/2 + 8; x += 1.2f) {
        drawFence(x, COURT_WIDTH/2 + 5, 0);
    }
    // Left fence line
    for (float z = -COURT_WIDTH/2 - 5; z <= COURT_WIDTH/2 + 5; z += 1.2f) {
        drawFence(-COURT_LENGTH/2 - 8, z, 90);
    }
    // Right fence line
    for (float z = -COURT_WIDTH/2 - 5; z <= COURT_WIDTH/2 + 5; z += 1.2f) {
        drawFence(COURT_LENGTH/2 + 8, z, 90);
    }
    
    // === TREES - Lush forest-like environment ===
    // Corner trees (large, prominent)
    drawTree(-COURT_LENGTH/2 - 4, -COURT_WIDTH/2 - 4);
    drawTree(-COURT_LENGTH/2 - 4, COURT_WIDTH/2 + 4);
    drawTree(COURT_LENGTH/2 + 4, -COURT_WIDTH/2 - 4);
    drawTree(COURT_LENGTH/2 + 4, COURT_WIDTH/2 + 4);
    
    // Perimeter trees (creating a natural border)
    drawTree(-COURT_LENGTH/2 - 6, 0);
    drawTree(COURT_LENGTH/2 + 6, 0);
    drawTree(0, -COURT_WIDTH/2 - 6);
    drawTree(0, COURT_WIDTH/2 + 6);
    
    // Additional decorative trees - MORE for park feel!
    drawTree(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2 + 2);
    drawTree(-COURT_LENGTH/2 - 7, COURT_WIDTH/2 - 2);
    drawTree(COURT_LENGTH/2 + 7, -COURT_WIDTH/2 + 2);
    drawTree(COURT_LENGTH/2 + 7, COURT_WIDTH/2 - 2);
    
    // Mid-distance trees for depth
    drawTree(-COURT_LENGTH/2 - 5, -COURT_WIDTH/2 - 1);
    drawTree(-COURT_LENGTH/2 - 5, COURT_WIDTH/2 + 1);
    drawTree(COURT_LENGTH/2 + 5, -COURT_WIDTH/2 - 1);
    drawTree(COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 1);
    
    // Far background trees (smaller perspective)
    drawTree(-COURT_LENGTH/2 - 9, -COURT_WIDTH/2 - 6);
    drawTree(-COURT_LENGTH/2 - 9, COURT_WIDTH/2 + 6);
    drawTree(COURT_LENGTH/2 + 9, -COURT_WIDTH/2 - 6);
    drawTree(COURT_LENGTH/2 + 9, COURT_WIDTH/2 + 6);
    
    // Clustered trees for natural look
    drawTree(-COURT_LENGTH/2 - 8, -COURT_WIDTH/2);
    drawTree(COURT_LENGTH/2 + 8, COURT_WIDTH/2);
    drawTree(-3, -COURT_WIDTH/2 - 7);
    drawTree(3, COURT_WIDTH/2 + 7);
    
    // === BUSHES - Abundant low greenery ===
    // Corner bushes
    drawBush(-COURT_LENGTH/2 - 3, -COURT_WIDTH/2 - 2);
    drawBush(-COURT_LENGTH/2 - 3, COURT_WIDTH/2 + 2);
    drawBush(COURT_LENGTH/2 + 3, -COURT_WIDTH/2 - 2);
    drawBush(COURT_LENGTH/2 + 3, COURT_WIDTH/2 + 2);
    
    // Bushes along paths - MANY MORE!
    drawBush(-COURT_LENGTH/2 - 1.5f, -COURT_WIDTH/2 - 3.5f);
    drawBush(COURT_LENGTH/2 + 1.5f, -COURT_WIDTH/2 - 3.5f);
    drawBush(-COURT_LENGTH/2 - 1.5f, COURT_WIDTH/2 + 3.5f);
    drawBush(COURT_LENGTH/2 + 1.5f, COURT_WIDTH/2 + 3.5f);
    
    // Additional decorative bushes
    drawBush(-COURT_LENGTH/2 - 4.5f, -COURT_WIDTH/2 - 3);
    drawBush(-COURT_LENGTH/2 - 4.5f, COURT_WIDTH/2 + 3);
    drawBush(COURT_LENGTH/2 + 4.5f, -COURT_WIDTH/2 - 3);
    drawBush(COURT_LENGTH/2 + 4.5f, COURT_WIDTH/2 + 3);
    
    // Bushes near benches
    drawBush(-COURT_LENGTH/2 - 3, -COURT_WIDTH/2 + 0.5f);
    drawBush(-COURT_LENGTH/2 - 3, COURT_WIDTH/2 - 0.5f);
    drawBush(COURT_LENGTH/2 + 3, -COURT_WIDTH/2 + 0.5f);
    drawBush(COURT_LENGTH/2 + 3, COURT_WIDTH/2 - 0.5f);
    
    // Random scattered bushes for natural look
    drawBush(-COURT_LENGTH/2 - 6.5f, -COURT_WIDTH/2 - 4.5f);
    drawBush(COURT_LENGTH/2 + 6.5f, COURT_WIDTH/2 + 4.5f);
    drawBush(-2.5f, -COURT_WIDTH/2 - 6);
    drawBush(2.5f, COURT_WIDTH/2 + 6);
    drawBush(-COURT_LENGTH/2 - 7.5f, 1);
    drawBush(COURT_LENGTH/2 + 7.5f, -1);
    
    // === FLOWERS - Beautiful colorful gardens! ===
    // Corner flower beds (prominent)
    drawFlowers(-COURT_LENGTH/2 - 5, -COURT_WIDTH/2 - 3);
    drawFlowers(-COURT_LENGTH/2 - 5, COURT_WIDTH/2 + 3);
    drawFlowers(COURT_LENGTH/2 + 5, -COURT_WIDTH/2 - 3);
    drawFlowers(COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 3);
    
    // Flower gardens along paths
    drawFlowers(-COURT_LENGTH/2 + 2, -COURT_WIDTH/2 - 4);
    drawFlowers(COURT_LENGTH/2 - 2, COURT_WIDTH/2 + 4);
    drawFlowers(0, -COURT_WIDTH/2 - 7);
    drawFlowers(0, COURT_WIDTH/2 + 7.5f);
    
    // Additional flower clusters - MUCH MORE COLOR!
    drawFlowers(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 - 5);
    drawFlowers(COURT_LENGTH/2 + 6, COURT_WIDTH/2 + 5);
    drawFlowers(-COURT_LENGTH/2 - 4, -COURT_WIDTH/2 - 5.5f);
    drawFlowers(COURT_LENGTH/2 + 4, COURT_WIDTH/2 + 5.5f);
    
    // Flowers near benches
    drawFlowers(-COURT_LENGTH/2 - 2.5f, -COURT_WIDTH/2 + 2);
    drawFlowers(-COURT_LENGTH/2 - 2.5f, COURT_WIDTH/2 - 2);
    drawFlowers(COURT_LENGTH/2 + 2.5f, -COURT_WIDTH/2 + 2);
    drawFlowers(COURT_LENGTH/2 + 2.5f, COURT_WIDTH/2 - 2);
    
    // Scattered flower patches
    drawFlowers(-4, -COURT_WIDTH/2 - 6.5f);
    drawFlowers(4, COURT_WIDTH/2 + 6.5f);
    drawFlowers(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2 - 2);
    drawFlowers(COURT_LENGTH/2 + 7, COURT_WIDTH/2 + 2);
    
    // Front entrance flowers
    drawFlowers(-1.5f, -COURT_WIDTH/2 - 8);
    drawFlowers(1.5f, -COURT_WIDTH/2 - 8);
    
    // === BENCHES - Plenty of seating! ===
    // Side benches (watching the game)
    drawBench(-COURT_LENGTH/2 - 2, -COURT_WIDTH/2 + 1, 90);
    drawBench(-COURT_LENGTH/2 - 2, COURT_WIDTH/2 - 1, 90);
    drawBench(COURT_LENGTH/2 + 2, -COURT_WIDTH/2 + 1, -90);
    drawBench(COURT_LENGTH/2 + 2, COURT_WIDTH/2 - 1, -90);
    
    // Additional side benches
    drawBench(-COURT_LENGTH/2 - 2, 0, 90);
    drawBench(COURT_LENGTH/2 + 2, 0, -90);
    
    // End zone benches
    drawBench(0, -COURT_WIDTH/2 - 4, 0);
    drawBench(0, COURT_WIDTH/2 + 4, 180);
    drawBench(-3, COURT_WIDTH/2 + 4, 180);
    drawBench(3, COURT_WIDTH/2 + 4, 180);
    
    // Resting area benches (away from court)
    drawBench(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 - 2, 45);
    drawBench(COURT_LENGTH/2 + 6, COURT_WIDTH/2 + 2, -135);
    
    // === STREET LAMPS - Bright illumination at night ===
    // Corner lamps (primary lighting)
    drawStreetLamp(-COURT_LENGTH/2 - 3, COURT_WIDTH/2 + 3);
    drawStreetLamp(COURT_LENGTH/2 + 3, -COURT_WIDTH/2 - 3);
    drawStreetLamp(-COURT_LENGTH/2 - 3, -COURT_WIDTH/2 - 3);
    drawStreetLamp(COURT_LENGTH/2 + 3, COURT_WIDTH/2 + 3);
    
    // Side lamps for complete coverage
    drawStreetLamp(-COURT_LENGTH/2 - 3, 0);
    drawStreetLamp(COURT_LENGTH/2 + 3, 0);
    drawStreetLamp(0, -COURT_WIDTH/2 - 5);
    drawStreetLamp(0, COURT_WIDTH/2 + 5);
    
    // Additional pathway lighting
    drawStreetLamp(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 - 5);
    drawStreetLamp(COURT_LENGTH/2 + 6, COURT_WIDTH/2 + 5);
    
    // === TRASH BINS - Clean park maintenance ===
    drawTrashBin(-COURT_LENGTH/2 - 2.5f, -COURT_WIDTH/2 - 1.5f);
    drawTrashBin(COURT_LENGTH/2 + 2.5f, COURT_WIDTH/2 + 1.5f);
    drawTrashBin(0, -COURT_WIDTH/2 - 4.5f);
    drawTrashBin(0, COURT_WIDTH/2 + 4.5f);
    drawTrashBin(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 - 6);
    drawTrashBin(COURT_LENGTH/2 + 6, COURT_WIDTH/2 + 6);
    
    // === SIGNPOSTS - Informative signs ===
    drawSignpost(-COURT_LENGTH/2 - 6.5f, -COURT_WIDTH/2 - 6, "Welcome");
    drawSignpost(COURT_LENGTH/2 + 6.5f, COURT_WIDTH/2 + 6, "Pickleball");
    drawSignpost(-COURT_LENGTH/2 - 7.5f, COURT_WIDTH/2 + 5, "Park Rules");
    
    // === FOUNTAINS - Beautiful water features! ===
    // Main central fountain behind court
    drawFountain(0, COURT_WIDTH/2 + 7);
    
    // Additional decorative fountains
    drawFountain(-COURT_LENGTH/2 - 8, 0);
    drawFountain(COURT_LENGTH/2 + 8, 0);
    
    // === LARGE GARDEN BEDS - Beautiful flower beds! ===
    // Corner garden beds (large, prominent)
    drawGardenBed(-COURT_LENGTH/2 - 5.5f, -COURT_WIDTH/2 - 4, 2.5f, 2.0f);
    drawGardenBed(COURT_LENGTH/2 + 5.5f, COURT_WIDTH/2 + 4, 2.5f, 2.0f);
    drawGardenBed(-COURT_LENGTH/2 - 5.5f, COURT_WIDTH/2 + 4, 2.5f, 2.0f);
    drawGardenBed(COURT_LENGTH/2 + 5.5f, -COURT_WIDTH/2 - 4, 2.5f, 2.0f);
    
    // Linear garden beds along paths
    drawGardenBed(-COURT_LENGTH/2 + 1, -COURT_WIDTH/2 - 5.5f, 4.0f, 1.5f);
    drawGardenBed(COURT_LENGTH/2 - 1, COURT_WIDTH/2 + 5.5f, 4.0f, 1.5f);
    
    // Side garden beds
    drawGardenBed(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2, 1.8f, 3.0f);
    drawGardenBed(COURT_LENGTH/2 + 7, COURT_WIDTH/2, 1.8f, 3.0f);
    
    // Entrance garden beds
    drawGardenBed(0, -COURT_WIDTH/2 - 7.5f, 3.5f, 1.2f);
    drawGardenBed(0, COURT_WIDTH/2 + 8.5f, 5.0f, 1.5f);
    
    // === CIRCULAR FLOWER PATCHES - Round decorative gardens ===
    // Main circular patches (highly visible)
    drawCircularFlowerPatch(-COURT_LENGTH/2 - 8.5f, -COURT_WIDTH/2 - 5.5f, 0.8f);
    drawCircularFlowerPatch(COURT_LENGTH/2 + 8.5f, COURT_WIDTH/2 + 5.5f, 0.8f);
    
    // Medium circular patches
    drawCircularFlowerPatch(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 + 1, 0.6f);
    drawCircularFlowerPatch(COURT_LENGTH/2 + 6, COURT_WIDTH/2 - 1, 0.6f);
    drawCircularFlowerPatch(-COURT_LENGTH/2 - 3.5f, -COURT_WIDTH/2 - 5, 0.5f);
    drawCircularFlowerPatch(COURT_LENGTH/2 + 3.5f, COURT_WIDTH/2 + 5, 0.5f);
    
    // Small accent patches
    drawCircularFlowerPatch(-COURT_LENGTH/2 - 9, COURT_WIDTH/2 + 2, 0.5f);
    drawCircularFlowerPatch(COURT_LENGTH/2 + 9, -COURT_WIDTH/2 - 2, 0.5f);
    drawCircularFlowerPatch(-2, -COURT_WIDTH/2 - 8.5f, 0.4f);
    drawCircularFlowerPatch(2, COURT_WIDTH/2 + 9, 0.4f);
    
    // Additional scattered patches for natural look
    drawCircularFlowerPatch(-COURT_LENGTH/2 - 5, COURT_WIDTH/2 + 6, 0.45f);
    drawCircularFlowerPatch(COURT_LENGTH/2 + 5, -COURT_WIDTH/2 - 6, 0.45f);
    
    // === PICNIC TABLES - Park seating and gathering areas ===
    // Main picnic area (back of court)
    drawPicnicTable(-4, COURT_WIDTH/2 + 6, 0);
    drawPicnicTable(4, COURT_WIDTH/2 + 6, 0);
    
    // Side picnic areas
    drawPicnicTable(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2 - 4.5f, 45);
    drawPicnicTable(COURT_LENGTH/2 + 7, COURT_WIDTH/2 + 4.5f, -45);
    
    // Additional scattered tables
    drawPicnicTable(-COURT_LENGTH/2 - 6, COURT_WIDTH/2 + 1, 90);
    drawPicnicTable(COURT_LENGTH/2 + 6, -COURT_WIDTH/2 - 1, -90);
    
    // Front entrance area table
    drawPicnicTable(0, -COURT_WIDTH/2 - 7, 0);
    
    // === DECORATIVE ROCK CLUSTERS - Natural landscaping elements ===
    // Corner rock clusters (natural borders)
    drawRockCluster(-COURT_LENGTH/2 - 7.5f, -COURT_WIDTH/2 - 6.5f);
    drawRockCluster(COURT_LENGTH/2 + 7.5f, COURT_WIDTH/2 + 6.5f);
    drawRockCluster(-COURT_LENGTH/2 - 8.5f, COURT_WIDTH/2 + 6.5f);
    drawRockCluster(COURT_LENGTH/2 + 8.5f, -COURT_WIDTH/2 - 6.5f);
    
    // Accent rock clusters near paths
    drawRockCluster(-COURT_LENGTH/2 - 4, -COURT_WIDTH/2 - 6);
    drawRockCluster(COURT_LENGTH/2 + 4, COURT_WIDTH/2 + 6);
    drawRockCluster(-5, -COURT_WIDTH/2 - 7.5f);
    drawRockCluster(5, COURT_WIDTH/2 + 8);
    
    // Decorative rocks near fountains
    drawRockCluster(-COURT_LENGTH/2 - 9, 1);
    drawRockCluster(COURT_LENGTH/2 + 9, -1);
    drawRockCluster(-1.5f, COURT_WIDTH/2 + 8);
    drawRockCluster(1.5f, COURT_WIDTH/2 + 8);
    
    // Natural scattered rocks
    drawRockCluster(-COURT_LENGTH/2 - 6.5f, -COURT_WIDTH/2 - 2.5f);
    drawRockCluster(COURT_LENGTH/2 + 6.5f, COURT_WIDTH/2 + 2.5f);
    drawRockCluster(-COURT_LENGTH/2 - 3, -COURT_WIDTH/2 - 7);
    drawRockCluster(COURT_LENGTH/2 + 3, COURT_WIDTH/2 + 7);

    
    // === CLOUDS - Fluffy sky decoration ===
    if (timeOfDay > 0.25f && timeOfDay < 0.75f) {  // Only during day
        // Large background clouds
        drawCloud(-15, 20, -10, 1.5f);
        drawCloud(15, 22, 5, 1.2f);
        drawCloud(0, 25, -15, 1.8f);
        drawCloud(-8, 18, 10, 1.0f);
        drawCloud(12, 21, -5, 1.3f);
        
        // Additional mid-range clouds
        drawCloud(-20, 19, 8, 1.4f);
        drawCloud(18, 23, -12, 1.1f);
        drawCloud(-5, 24, 12, 1.6f);
        
        // Small foreground clouds
        drawCloud(8, 17, -8, 0.9f);
        drawCloud(-12, 21, 3, 1.0f);
    }
    
    
    glutSwapBuffers();
}

// Reshape function
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

// Timer function
void timer(int value) {
    updateBall();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // ~60 FPS
}

// Keyboard function
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:  // ESC
            exit(0);
            break;
        case ' ':  // Space - pause/resume
            isPaused = !isPaused;
            break;
        case 'w':
        case 'W':
            cameraDistance -= 0.5f;
            if (cameraDistance < 5.0f) cameraDistance = 5.0f;
            break;
        case 's':
        case 'S':
            cameraDistance += 0.5f;
            if (cameraDistance > 40.0f) cameraDistance = 40.0f;
            break;
        case 'a':
        case 'A':
            cameraAngle -= 5.0f;
            break;
        case 'd':
        case 'D':
            cameraAngle += 5.0f;
            break;
        case 'q':
        case 'Q':
            cameraHeight += 0.5f;
            if (cameraHeight > 25.0f) cameraHeight = 25.0f;
            break;
        case 'e':
        case 'E':
            cameraHeight -= 0.5f;
            if (cameraHeight < 2.0f) cameraHeight = 2.0f;
            break;
        case 'r':
        case 'R':
            windStrength += 0.1f;
            if (windStrength > 3.0f) windStrength = 3.0f;
            printf("Wind strength: %.1f\n", windStrength);
            break;
        case 'f':
        case 'F':
            windStrength -= 0.1f;
            if (windStrength < 0.0f) windStrength = 0.0f;
            printf("Wind strength: %.1f\n", windStrength);
            break;
    }
    glutPostRedisplay();
}

// Special keyboard function for arrow keys
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            timeOfDay += 0.02f;
            if (timeOfDay > 1.0f) timeOfDay = 0.0f;
            break;
        case GLUT_KEY_DOWN:
            timeOfDay -= 0.02f;
            if (timeOfDay < 0.0f) timeOfDay = 1.0f;
            break;
    }
    glutPostRedisplay();
}

// Initialize OpenGL - LIGHTING ENABLED with color materials
void init() {
    glEnable(GL_DEPTH_TEST);
    
    // ENABLE lighting for proper illumination!
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);  // Normalize normals for correct lighting
    
    // Enable alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Try to load 3D models - NEW!
    printf("\n=== Loading 3D Models ===\n");
    bool treeLoaded = treeModel.loadModel("models/tree.obj");
    bool paddleLoaded = paddleModel.loadModel("models/paddle.obj");
    bool playerLoaded = playerModel.loadModel("models/player.obj");
    
    if (treeLoaded || paddleLoaded || playerLoaded) {
        use3DModels = true;
        printf("✓ 3D Models loaded successfully!\n");
        if (treeLoaded) printf("  - Tree model loaded\n");
        if (paddleLoaded) printf("  - Paddle model loaded\n");
        if (playerLoaded) printf("  - Player model loaded\n");
    } else {
        printf("ℹ No 3D models found. Using geometric shapes (fallback mode)\n");
        printf("  To use 3D models, place .obj files in models/ folder\n");
    }
    printf("========================\n\n");
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Pickleball Playground Scene");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    
    printf("=== Enhanced Pickleball Park Scene ===\n");
    printf("Controls:\n");
    printf("  Arrow Up/Down: Adjust time of day\n");
    printf("  W/S: Zoom in/out\n");
    printf("  A/D: Rotate camera\n");
    printf("  Q/E: Adjust camera height\n");
    printf("  R/F: Increase/Decrease wind\n");
    printf("  SPACE: Pause/Resume\n");
    printf("  ESC: Exit\n");
    
    glutMainLoop();
    return 0;
}
