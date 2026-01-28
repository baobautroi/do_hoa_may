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
#include "GraphicsUtils_v2.h" // Enhanced graphics: Shadows (Fixed)
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

// Walker animation states (people walking/jogging on track)
struct WalkerState {
    float posX, posZ;        // Current position
    float angle;             // Direction angle (0-360)
    float speed;             // Walking/jogging speed
    float legAngle1, legAngle2;
    float armSwing1, armSwing2;
    int pathSegment;         // Which segment of track (0=bottom, 1=right, 2=top, 3=left)
    float pathProgress;      // Progress along current segment (0-1)
};

// Initialize 4 walkers on different parts of the track
// Track offset = 9.0f, trackWidth = 3.0f, middle of track = 10.5f from center
WalkerState walker1 = {-COURT_LENGTH/2 - 10.5f, -COURT_WIDTH/2 - 10.5f, 0, 0.04f, 0, 0, 0, 0, 3, 0.0f};  // Person with dog
WalkerState walker2 = {COURT_LENGTH/2 + 10.5f, -COURT_WIDTH/2 - 10.0f, 180, 0.04f, 0, 0, 0, 0, 0, 0.5f}; // Walking couple (person 1 - left)
WalkerState walker3 = {COURT_LENGTH/2 + 10.5f, -COURT_WIDTH/2 - 11.0f, 180, 0.04f, 0, 0, 0, 0, 0, 0.5f}; // Walking couple (person 2 - right)
WalkerState walker4 = {-COURT_LENGTH/2 - 10.5f, COURT_WIDTH/2 + 10.5f, 270, 0.04f, 0, 0, 0, 0, 2, 0.7f}; // Walker (same speed)

// Dog position (follows walker1)
float dogPosX = walker1.posX + 1.5f;
float dogPosZ = walker1.posZ;
float dogAngle = walker1.angle; // Dog faces same direction as walker

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
    
    bool isNight = (timeOfDay < 0.3f || timeOfDay > 0.7f);
    
    if (isNight) {
        // === NIGHT MODE: Dark environment with stadium floodlights ===
        
        // Very low ambient light (dark night)
        GLfloat darkAmbient[] = {0.05f, 0.05f, 0.08f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, darkAmbient);
        
        // Disable sun (GL_LIGHT0)
        glDisable(GL_LIGHT0);
        
        // === STADIUM FLOODLIGHTS (4 powerful spotlights) ===
        // Floodlight positions (matching drawCourtFloodlight positions)
        float floodlightData[][3] = {
            // x, z, height at 4 corners
            {-COURT_LENGTH/2 - 2, -COURT_WIDTH/2 - 2, 10.0f},  // Bottom-left
            {COURT_LENGTH/2 + 2, -COURT_WIDTH/2 - 2, 10.0f},   // Bottom-right
            {-COURT_LENGTH/2 - 2, COURT_WIDTH/2 + 2, 10.0f},   // Top-left
            {COURT_LENGTH/2 + 2, COURT_WIDTH/2 + 2, 10.0f}     // Top-right
        };
        
        // Enable and configure 4 floodlights
        GLenum lights[] = {GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4};
        
        for (int i = 0; i < 4; i++) {
            glEnable(lights[i]);
            
            // Position at top of pole
            GLfloat pos[] = {floodlightData[i][0], floodlightData[i][2], floodlightData[i][1], 1.0f};
            glLightfv(lights[i], GL_POSITION, pos);
            
            // VERY BRIGHT white light (stadium quality - ENHANCED)
            GLfloat diffuse[] = {1.5f, 1.5f, 1.4f, 1.0f};  // Boosted intensity
            GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
            GLfloat ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};  // Added ambient contribution
            glLightfv(lights[i], GL_DIFFUSE, diffuse);
            glLightfv(lights[i], GL_SPECULAR, specular);
            glLightfv(lights[i], GL_AMBIENT, ambient);
            
            // Spotlight parameters - ALL lights converge to CENTER
            // All 4 floodlights focus on the exact center of the court
            GLfloat centerY = 0.0f;  // Court surface level
            
            // ALL lights target the CENTER (0, 0)
            GLfloat targetX = 0.0f;  // Center X
            GLfloat targetZ = -3.2f;  // Center Z
            
            GLfloat dirX = targetX - floodlightData[i][0];
            GLfloat dirY = centerY - floodlightData[i][2];
            GLfloat dirZ = targetZ - floodlightData[i][1];
            
            GLfloat spotDir[] = {dirX, dirY, dirZ};
            glLightfv(lights[i], GL_SPOT_DIRECTION, spotDir);
            glLightf(lights[i], GL_SPOT_CUTOFF, 55.0f);  // Wider cone for overlap
            glLightf(lights[i], GL_SPOT_EXPONENT, 6.0f);  // Very soft falloff
            
            // Minimal attenuation for maximum coverage
            glLightf(lights[i], GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(lights[i], GL_LINEAR_ATTENUATION, 0.002f);  // Very low
            glLightf(lights[i], GL_QUADRATIC_ATTENUATION, 0.0002f);
        }

        
    } else {
        // === DAY MODE: Natural sunlight ===
        
        // Disable stadium lights
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
        glDisable(GL_LIGHT4);
        
        // Enable sun
        glEnable(GL_LIGHT0);
        
        // Sun position and color
        GLfloat lightPos[] = {sunX, sunY, sunZ, 1.0f};
        
        float intensity = 1.0f;
        GLfloat lightColor[4];
        
        if (timeOfDay < 0.35f || timeOfDay > 0.65f) {  // Dawn/Dusk
            intensity = 0.9f;
            lightColor[0] = 1.0f * intensity;
            lightColor[1] = 0.8f * intensity;
            lightColor[2] = 0.6f * intensity;
        } else {  // Full daylight
            intensity = 1.0f;
            lightColor[0] = 1.0f;
            lightColor[1] = 1.0f;
            lightColor[2] = 0.95f;
        }
        lightColor[3] = 1.0f;
        
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);
        
        // Bright ambient for daytime
        GLfloat ambient[] = {0.6f, 0.6f, 0.65f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        
        // Bright global ambient
        GLfloat globalAmbient[] = {0.4f, 0.4f, 0.45f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    }
}

// Draw the court surface - FLAT COLOR
void drawCourt() {
    glPushMatrix();
    
    glColor3f(0.5f, 0.6f, 0.75f);  // Blue-gray court
    
    
    // Draw Out-of-bounds area (Safe zone) - COVERS THE GAP
    // glPushMatrix();
    // glColor3f(0.45f, 0.65f, 0.85f); // Light blue safe zone
    
    // // Use Global Constants for perfect sync with grass
    // float mx = MARGIN_X;
    // float mz = MARGIN_Z;
    
    // glBegin(GL_QUADS);
    // glVertex3f(-COURT_LENGTH/2 - mx, -0.01f, -COURT_WIDTH/2 - mz);
    // glVertex3f(COURT_LENGTH/2 + mx, -0.01f, -COURT_WIDTH/2 - mz);
    // glVertex3f(COURT_LENGTH/2 + mx, -0.01f, COURT_WIDTH/2 + mz);
    // glVertex3f(-COURT_LENGTH/2 - mx, -0.01f, COURT_WIDTH/2 + mz);
    // glEnd();
    // glPopMatrix();
    
    
    // === OUT-OF-BOUNDS AREA (Light Blue) - Draw first ===
    // This creates a larger light blue area that the dark blue court sits on top of
    float OOB_MARGIN_X = 3.0f;  // Extra margin beyond main court
    float OOB_MARGIN_Z = 3.0f;  // Extra margin beyond main court
    
    glColor3f(0.5f, 0.65f, 0.85f);  // Light Blue (Out-of-Bounds area)
    
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - OOB_MARGIN_X, 0.005, -COURT_WIDTH/2 - OOB_MARGIN_Z);
    glVertex3f(COURT_LENGTH/2 + OOB_MARGIN_X, 0.005, -COURT_WIDTH/2 - OOB_MARGIN_Z);
    glVertex3f(COURT_LENGTH/2 + OOB_MARGIN_X, 0.005, COURT_WIDTH/2 + OOB_MARGIN_Z);
    glVertex3f(-COURT_LENGTH/2 - OOB_MARGIN_X, 0.005, COURT_WIDTH/2 + OOB_MARGIN_Z);
    glEnd();
    
    // === MAIN COURT SURFACE (Dark Blue) - Draw on top ===
    glColor3f(0.3f, 0.4f, 0.65f);  // Darker Blue Royal Court
    
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2, 0.01, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0.01, -COURT_WIDTH/2);
    glVertex3f(COURT_LENGTH/2, 0.01, COURT_WIDTH/2);
    glVertex3f(-COURT_LENGTH/2, 0.01, COURT_WIDTH/2);
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
        // FALLBACK: Original geometric treed
        // Trunk - BRIGHT BROWN
        glColor3f(0.55f, 0.35f, 0.2f);  // Bright brown
        
        glPushMatrix();
        // Remove offset so tree starts from ground (y=0)
        // glTranslatef(0, 1.5f, 0); 
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

// ============================================================================
// TREE VARIANTS - DIFFERENT SIZES
// ============================================================================

// Draw SMALL tree (60% size) - For background/distance trees
void drawSmallTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    float swayAngle = sin(windTime + x * 0.5f + z * 0.3f) * windStrength * 3.0f;
    
    // Try to use 3D model if loaded
    if (treeModel.getMeshCount() > 0) {
        glRotatef(swayAngle, 0, 0, 1);
        treeModel.render(0, 0, 0, 0.9f);  // 60% scale
    } else {
        // Geometric tree at 60% scale
        glColor3f(0.55f, 0.35f, 0.2f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.21f, 0.15f, 1.8f, 16, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0, 1.8f, 0);
        glRotatef(swayAngle, 0, 0, 1);
        
        glColor3f(0.3f, 0.7f, 0.3f);
        glutSolidSphere(0.84f, 16, 16);
        
        glColor3f(0.35f, 0.75f, 0.35f);
        glPushMatrix();
        glTranslatef(0, 0.48f, 0);
        glutSolidSphere(0.66f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.4f, 0.8f, 0.4f);
        glPushMatrix();
        glTranslatef(0, 0.9f, 0);
        glutSolidSphere(0.48f, 16, 16);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw MEDIUM tree (80% size) - For mid-distance trees
void drawMediumTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    float swayAngle = sin(windTime + x * 0.5f + z * 0.3f) * windStrength * 3.0f;
    
    if (treeModel.getMeshCount() > 0) {
        glRotatef(swayAngle, 0, 0, 1);
        treeModel.render(0, 0, 0, 1.2f);  // 80% scale
    } else {
        glColor3f(0.55f, 0.35f, 0.2f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.28f, 0.20f, 2.4f, 16, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0, 2.4f, 0);
        glRotatef(swayAngle, 0, 0, 1);
        
        glColor3f(0.3f, 0.7f, 0.3f);
        glutSolidSphere(1.12f, 16, 16);
        
        glColor3f(0.35f, 0.75f, 0.35f);
        glPushMatrix();
        glTranslatef(0, 0.64f, 0);
        glutSolidSphere(0.88f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.4f, 0.8f, 0.4f);
        glPushMatrix();
        glTranslatef(0, 1.2f, 0);
        glutSolidSphere(0.64f, 16, 16);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw LARGE tree (120% size) - For emphasis/focal points
void drawLargeTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    float swayAngle = sin(windTime + x * 0.5f + z * 0.3f) * windStrength * 3.0f;
    
    if (treeModel.getMeshCount() > 0) {
        glRotatef(swayAngle, 0, 0, 1);
        treeModel.render(0, 0, 0, 1.8f);  // 120% scale
    } else {
        glColor3f(0.55f, 0.35f, 0.2f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.42f, 0.30f, 3.6f, 16, 4);
        gluDeleteQuadric(quad);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(0, 3.6f, 0);
        glRotatef(swayAngle, 0, 0, 1);
        
        glColor3f(0.3f, 0.7f, 0.3f);
        glutSolidSphere(1.68f, 16, 16);
        
        glColor3f(0.35f, 0.75f, 0.35f);
        glPushMatrix();
        glTranslatef(0, 0.96f, 0);
        glutSolidSphere(1.32f, 16, 16);
        glPopMatrix();
        
        glColor3f(0.4f, 0.8f, 0.4f);
        glPushMatrix();
        glTranslatef(0, 1.8f, 0);
        glutSolidSphere(0.96f, 16, 16);
        glPopMatrix();
        
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

// Draw professional stadium floodlight (high-power, like football stadiums)
void drawCourtFloodlight(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // === BASE (Concrete foundation) ===
    glColor3f(0.4f, 0.4f, 0.4f);
    glPushMatrix();
    glTranslatef(0, 0.2f, 0);
    glScalef(0.5f, 0.4f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === MAIN POLE (Thick metal pole - 10m tall) ===
    glColor3f(0.2f, 0.2f, 0.2f);  // Dark metal gray
    
    glPushMatrix();
    glTranslatef(0, 0.4f, 0);
    glRotatef(-90, 1, 0, 0);
    GLUquadric* pole = gluNewQuadric();
    gluCylinder(pole, 0.18f, 0.14f, 9.6f, 20, 1);  // 10m pole
    gluDeleteQuadric(pole);
    glPopMatrix();
    
    // === TOP PLATFORM (for mounting lights) ===
    glColor3f(0.25f, 0.25f, 0.25f);
    glPushMatrix();
    glTranslatef(0, 10.0f, 0);
    glScalef(1.2f, 0.1f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === LIGHT HEADS (4 large stadium spotlights) ===
    bool lightsOn = (timeOfDay < 0.3f || timeOfDay > 0.7f);  // Night mode
    
    // Floodlight positions (arranged in square pattern)
    float lightPositions[][2] = {
        {-0.45f, 0.45f},   // Top-left
        {0.45f, 0.45f},    // Top-right
        {-0.45f, -0.45f},  // Bottom-left
        {0.45f, -0.45f}    // Bottom-right
    };
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(lightPositions[i][0], 10.0f, lightPositions[i][1]);
        
        // === MOUNTING ARM ===
        glColor3f(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glTranslatef(0, -0.3f, 0);
        glScalef(0.08f, 0.35f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // === SPOTLIGHT HOUSING (Large reflector box) ===
        glColor3f(0.18f, 0.18f, 0.18f);
        glPushMatrix();
        glTranslatef(0, -0.6f, 0);
        glRotatef(45, 1, 0, 0);  // Angled down toward court
        glScalef(0.35f, 0.4f, 0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // === LIGHT LENS/BULB (Front glass) ===
        glPushMatrix();
        glTranslatef(0, -0.75f, 0.12f);
        glRotatef(45, 1, 0, 0);
        
        if (lightsOn) {
            // BRIGHT INTENSE WHITE when ON (like stadium lights)
            glColor3f(1.0f, 1.0f, 1.0f);
            
            // EMISSION for glowing effect
            GLfloat emission[] = {1.0f, 1.0f, 0.9f, 1.0f};  // Bright glow
            glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        } else {
            // Dark gray when OFF
            glColor3f(0.3f, 0.3f, 0.3f);
            
            // No emission
            GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
        }
        
        // Large circular lens
        glScalef(0.28f, 0.28f, 0.15f);
        glutSolidSphere(1.0f, 16, 16);
        glPopMatrix();
        
        // Reset emission after drawing
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
        
        // === LIGHT GLOW EFFECT (when ON) - Enhanced ===
        if (lightsOn) {
            // Enable blending for glow
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glDepthMask(GL_FALSE);
            
            glPushMatrix();
            glTranslatef(0, -0.75f, 0.12f);
            glRotatef(45, 1, 0, 0);
            
            // Inner bright glow
            glColor4f(1.0f, 1.0f, 0.95f, 0.8f);
            glScalef(0.32f, 0.32f, 0.18f);
            glutSolidSphere(1.0f, 12, 12);
            glPopMatrix();
            
            // Outer soft glow
            glPushMatrix();
            glTranslatef(0, -0.75f, 0.12f);
            glRotatef(45, 1, 0, 0);
            glColor4f(1.0f, 1.0f, 0.85f, 0.4f);
            glScalef(0.45f, 0.45f, 0.25f);
            glutSolidSphere(1.0f, 12, 12);
            glPopMatrix();
            
            // Restore states
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw modern street lamp for running track
void drawStreetLamp(float x, float z, float rotation) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotation, 0, 1, 0);
    
    bool isNight = (timeOfDay < 0.25f || timeOfDay > 0.75f);
    
    // === 1. POLE (Metallic Silver) ===
    glColor3f(0.7f, 0.75f, 0.8f);
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, 0.1f, 0.08f, 5.0f, 16, 1);  // 5m height (smaller than floodlights)
    
    // Top cap ball
    glPushMatrix();
    glTranslatef(0, 0, 5.0f);
    glutSolidSphere(0.12f, 12, 12);
    glPopMatrix();
    
    gluDeleteQuadric(quad);
    glPopMatrix();
    
    // === 2. CURVED ARM ===
    glPushMatrix();
    glTranslatef(0, 4.5f, 0); // Start arm 0.5m from top
    // Arm extending outward (no additional rotation - already rotated at start)
    
    // Create a curved effect using rotated cylinder segments
    float armLen = 1.5f;
    float angleStep = 10.0f;
    int segments = 4;
    
    glColor3f(0.7f, 0.75f, 0.8f);
    
    float currentX = 0;
    float currentY = 0;
    
    for(int i=0; i<segments; i++) {
        glPushMatrix();
        glTranslatef(currentX, currentY, 0);
        glRotatef(30.0f - i*angleStep, 0, 0, 1); // Curve upward then flat
        glRotatef(-90, 0, 1, 0); // Cylinder along X
        
        GLUquadric* arm = gluNewQuadric();
        gluCylinder(arm, 0.07f, 0.07f, armLen/segments, 8, 1);
        gluDeleteQuadric(arm);
        glPopMatrix();
        
        // Calculate next pos approx
        float ang = (30.0f - i*angleStep) * PI / 180.0f;
        currentX += (armLen/segments) * cos(ang);
        currentY += (armLen/segments) * sin(ang);
    }
    
    // Save final arm position for light effect
    float finalArmX = currentX;
    float finalArmY = currentY;
    
    // === 3. LAMP HEAD ===
    glTranslatef(currentX, currentY, 0);
    glRotatef(-10, 0, 0, 1); // Tilt head slightly down
    
    // Housing
    glColor3f(0.3f, 0.3f, 0.35f); // Dark grey housing
    glPushMatrix();
    glScalef(0.4f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Light Face
    glTranslatef(0, -0.055f, 0);
    
    if (isNight) {
        // GLOWING YELLOW AT NIGHT
        glColor3f(1.0f, 0.9f, 0.5f);
        GLfloat emission[] = {1.0f, 0.9f, 0.4f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, emission);
    } else {
        glColor3f(0.9f, 0.9f, 0.9f);
        GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    }
    
    glPushMatrix();
    glScalef(0.35f, 0.02f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Reset Emission
    GLfloat noEmission[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    
    glPopMatrix(); // End Arm/Head
    
    // === 4. LIGHT CONE & GROUND SPOT (Visual Effect) ===
    if (isNight) {
        // Use actual arm position
        float lightHeight = 4.5f + finalArmY;
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
        glDepthMask(GL_FALSE);
        
        // 4.1 Light Cone (Shaft) - pointing downward
        glPushMatrix();
        glTranslatef(finalArmX, 0, 0);
        glRotatef(-90, 1, 0, 0); // Point down (cone apex down)
        
        // Transparent Yellow Cone
        glColor4f(1.0f, 0.9f, 0.4f, 0.15f); 
        glutSolidCone(1.8f, lightHeight, 16, 1); 
        glPopMatrix();
        
        // 4.2 Light Spot on Ground (Track illumination)
        glPushMatrix();
        glTranslatef(finalArmX, 0.02f, 0); // Just above ground
        
        // Bright center spot
        glColor4f(1.0f, 0.9f, 0.5f, 0.35f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for(int i=0; i<=16; i++) {
            float a = i * 2.0f * PI / 16.0f;
            glVertex3f(cos(a)*1.2f, 0, sin(a)*1.2f);
        }
        glEnd();
        
        // Softer outer glow (Yellow)
        glColor4f(1.0f, 0.85f, 0.3f, 0.2f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);
        for(int i=0; i<=16; i++) {
            float a = i * 2.0f * PI / 16.0f;
            glVertex3f(cos(a)*3.0f, 0, sin(a)*3.0f);
        }
        glEnd();
        
        glPopMatrix();
        
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        
        // Restore standard blending if needed elsewhere
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
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

// Draw ornamental iron fence section (decorative metal fence with arched top)
void drawOrnamentalFence(float x, float z, float rotation, float sectionWidth = 2.5f) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(rotation, 0, 1, 0);
    
    // === Parameters ===
    float fenceHeight = 1.8f;        // Total height of fence
    float postWidth = 0.15f;         // Width of main posts
    float postHeight = 2.2f;         // Height of posts (taller than fence)
    float barThickness = 0.03f;      // Thickness of vertical bars
    int numBars = 12;                // Number of vertical bars
    
    // Black metal color (dark gray for realistic metal)
    glColor3f(0.15f, 0.15f, 0.15f);
    
    // === DRAW TWO MAIN POSTS (left and right) ===
    for (int side = 0; side < 2; side++) {
        float postX = (side == 0) ? -sectionWidth/2 : sectionWidth/2;
        
        glPushMatrix();
        glTranslatef(postX, 0, 0);
        
        // Main post (square)
        glPushMatrix();
        glTranslatef(0, postHeight/2, 0);
        glScalef(postWidth, postHeight, postWidth);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Post cap (decorative top)
        glPushMatrix();
        glTranslatef(0, postHeight, 0);
        
        // Small pyramid/cone top
        glColor3f(0.2f, 0.2f, 0.2f);
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        GLUquadric* capCone = gluNewQuadric();
        gluCylinder(capCone, postWidth * 0.7f, 0.0f, 0.3f, 8, 1);
        gluDeleteQuadric(capCone);
        glPopMatrix();
        
        // Decorative ball/sphere finial on top
        glTranslatef(0, 0.4f, 0);
        glColor3f(0.18f, 0.18f, 0.18f);
        glutSolidSphere(0.12f, 12, 12);
        
        glPopMatrix();
        glPopMatrix();
        
        glColor3f(0.15f, 0.15f, 0.15f); // Reset color
    }
    
    // === DRAW HORIZONTAL RAILS ===
    // Bottom rail
    glPushMatrix();
    glTranslatef(0, 0.3f, 0);
    glScalef(sectionWidth, 0.04f, 0.04f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Top rail (follows the arch - we'll approximate with straight section)
    glPushMatrix();
    glTranslatef(0, fenceHeight - 0.2f, 0);
    glScalef(sectionWidth, 0.04f, 0.04f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === DRAW VERTICAL BARS WITH ARCHED TOP ===
    for (int i = 0; i < numBars; i++) {
        float barX = -sectionWidth/2 + (sectionWidth / (numBars - 1)) * i;
        
        // Skip bars where posts are
        if (fabs(barX - (-sectionWidth/2)) < 0.1f || fabs(barX - (sectionWidth/2)) < 0.1f) {
            continue;
        }
        
        // Calculate height for arched effect
        // Parabolic arch: height increases towards center
        float normalizedX = (barX) / (sectionWidth/2); // -1 to 1
        float archBoost = 0.3f * (1.0f - normalizedX * normalizedX); // Parabola
        float barHeight = fenceHeight + archBoost;
        
        glPushMatrix();
        glTranslatef(barX, 0, 0);
        
        // Main vertical bar
        glPushMatrix();
        glTranslatef(0, barHeight/2, 0);
        glScalef(barThickness, barHeight, barThickness);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Pointed spike top
        glPushMatrix();
        glTranslatef(0, barHeight, 0);
        glRotatef(-90, 1, 0, 0);
        GLUquadric* spike = gluNewQuadric();
        gluCylinder(spike, barThickness, 0.0f, 0.15f, 6, 1);
        gluDeleteQuadric(spike);
        glPopMatrix();
        
        glPopMatrix();
    }
    
    // === DECORATIVE ELEMENTS (optional patterns) ===
    // Middle decorative rail with pattern
    glPushMatrix();
    glTranslatef(0, fenceHeight/2, 0);
    
    // Small decorative circles/diamonds along middle
    for (int i = 1; i < numBars - 1; i += 2) {
        float decorX = -sectionWidth/2 + (sectionWidth / (numBars - 1)) * i;
        glPushMatrix();
        glTranslatef(decorX, 0, 0);
        glScalef(0.08f, 0.08f, 0.03f);
        glutSolidSphere(1.0f, 8, 8);
        glPopMatrix();
    }
    glPopMatrix();
    
    glPopMatrix();
}

// Draw perimeter fence around entire map
void drawPerimeterFence() {
    // Fence parameters
    float fenceDistance = 15.0f;  // Distance from court center to fence
    float sectionWidth = 2.5f;    // Width of each fence section
    
    // Calculate fence boundaries (outside the running track)
    float fenceLeft = -COURT_LENGTH/2 - fenceDistance;
    float fenceRight = COURT_LENGTH/2 + fenceDistance;
    float fenceBottom = -COURT_WIDTH/2 - fenceDistance;
    float fenceTop = COURT_WIDTH/2 + fenceDistance;
    
    // Gate parameters (arch gate is at x=0, z=-COURT_WIDTH/2 - 15.0f)
    float gateX = 0.0f;
    float gateZ = -COURT_WIDTH/2 - 15.0f;  // Same as arch gate position
    float gateWidth = 14.0f;  // Width of opening to leave clear (scaled gate + wings)
    
    // Calculate total lengths for perfect alignment
    float totalWidth = fenceRight - fenceLeft;
    float totalHeight = fenceTop - fenceBottom;
    
    // === DRAW 4 CORNER SECTIONS FIRST (for perfect corners) ===
    // Bottom-left corner
    drawOrnamentalFence(fenceLeft + sectionWidth/2, fenceBottom + sectionWidth/2, 45, sectionWidth);
    // Bottom-right corner  
    drawOrnamentalFence(fenceRight - sectionWidth/2, fenceBottom + sectionWidth/2, -45, sectionWidth);
    // Top-left corner
    drawOrnamentalFence(fenceLeft + sectionWidth/2, fenceTop - sectionWidth/2, 135, sectionWidth);
    // Top-right corner
    drawOrnamentalFence(fenceRight - sectionWidth/2, fenceTop - sectionWidth/2, -135, sectionWidth);
    
    // === BOTTOM FENCE (horizontal sections) - SKIP NEAR GATE and CORNERS ===
    for (float x = fenceLeft + sectionWidth * 1.5f; x < fenceRight - sectionWidth; x += sectionWidth) {
        // Skip sections near the gate entrance
        if (fabs(x - gateX) < gateWidth/2 + 2.2) {
            continue;  // Don't draw fence here - leave opening for gate
        }
        
        drawOrnamentalFence(x, fenceBottom, 0, sectionWidth);
    }
    
    // === TOP FENCE (horizontal sections) - SKIP CORNERS ===
    for (float x = fenceLeft + sectionWidth * 1.5f; x < fenceRight - sectionWidth; x += sectionWidth) {
        drawOrnamentalFence(x, fenceTop, 0, sectionWidth);
    }
    
    // === LEFT FENCE (vertical sections) - SKIP CORNERS ===
    for (float z = fenceBottom + sectionWidth * 1.5f; z < fenceTop - sectionWidth; z += sectionWidth) {
        drawOrnamentalFence(fenceLeft, z, 90, sectionWidth);
    }
    
    // === RIGHT FENCE (vertical sections) - SKIP CORNERS ===
    for (float z = fenceBottom + sectionWidth * 1.5f; z < fenceTop - sectionWidth; z += sectionWidth) {
        drawOrnamentalFence(fenceRight, z, 90, sectionWidth);
    }
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

// Draw Parabolic Arch Gate (White entrance gate)
void drawArchGate(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    glScalef(1.56f, 1.56f, 1.56f); // Scale entire gate by 1.5x
    
    // Gate parameters
    float gateWidth = 8.0f;        // Total width of the arch
    float gateHeight = 5.0f;       // Height of the arch
    float pillarWidth = 0.6f;      // Width of each pillar
    float pillarDepth = 0.6f;      // Depth of each pillar
    float archThickness = 0.5f;    // Thickness of the arch
    
    // White color for the gate
    glColor3f(0.95f, 0.95f, 0.95f);
    
    // // === LEFT PILLAR ===
    // glPushMatrix();
    // glTranslatef(-gateWidth/2 + pillarWidth/2, gateHeight/2, 0);
    // glScalef(pillarWidth, gateHeight, pillarDepth);
    // glutSolidCube(1.0f);
    // glPopMatrix();
    
    // // === RIGHT PILLAR ===
    // glPushMatrix();
    // glTranslatef(gateWidth/2 - pillarWidth/2, gateHeight/2, 0);
    // glScalef(pillarWidth, gateHeight, pillarDepth);
    // glutSolidCube(1.0f);
    // glPopMatrix();
    
    // === PARABOLIC ARCH (Smooth Seamless Version) ===
    int numSegments = 100;
    float archWidth = gateWidth - pillarWidth * 2;
    float r = archWidth / 2;
    float h = gateHeight;
    float halfDepth = pillarDepth / 2.0f;
    float halfThick = archThickness / 2.0f;

    // We actully need to draw a 3D curving shape. 
    // We can draw it as a series of connected quads for front, back, top, bottom faces.
    
    // Draw TOP surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= numSegments; i++) {
        float x = -archWidth/2 + ((float)i / numSegments) * archWidth;
        float y = h - (h / (r * r)) * (x * x);
        
        // Calculate normal/angle for thickness offset
        // Derivative dy/dx = -2*h*x/r^2
        float slope = -2.0f * h * x / (r * r);
        float angle = atan(slope); // Angle of tangent
        float dx = -sin(angle) * halfThick; // Perpendicular offset x
        float dy = cos(angle) * halfThick;  // Perpendicular offset y
        
        // Top outer edge points
        glNormal3f(-sin(angle), cos(angle), 0.0f);
        glVertex3f(x + dx, y + dy, halfDepth);
        glVertex3f(x + dx, y + dy, -halfDepth);
    }
    glEnd();
    
    // Draw BOTTOM surface
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= numSegments; i++) {
        float x = -archWidth/2 + ((float)i / numSegments) * archWidth;
        float y = h - (h / (r * r)) * (x * x);
        
        float slope = -2.0f * h * x / (r * r);
        float angle = atan(slope);
        float dx = -sin(angle) * halfThick;
        float dy = cos(angle) * halfThick;
        
        // Bottom inner edge points
        glNormal3f(sin(angle), -cos(angle), 0.0f);
        glVertex3f(x - dx, y - dy, -halfDepth);
        glVertex3f(x - dx, y - dy, halfDepth);
    }
    glEnd();
    
    // Draw FRONT face
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i <= numSegments; i++) {
        float x = -archWidth/2 + ((float)i / numSegments) * archWidth;
        float y = h - (h / (r * r)) * (x * x);
        
        float slope = -2.0f * h * x / (r * r);
        float angle = atan(slope);
        float dx = -sin(angle) * halfThick;
        float dy = cos(angle) * halfThick;
        
        glVertex3f(x + dx, y + dy, halfDepth);
        glVertex3f(x - dx, y - dy, halfDepth);
    }
    glEnd();

    // Draw BACK face
    glBegin(GL_QUAD_STRIP);
    glNormal3f(0.0f, 0.0f, -1.0f);
    for (int i = 0; i <= numSegments; i++) {
        float x = -archWidth/2 + ((float)i / numSegments) * archWidth;
        float y = h - (h / (r * r)) * (x * x);
        
        float slope = -2.0f * h * x / (r * r);
        float angle = atan(slope);
        float dx = -sin(angle) * halfThick;
        float dy = cos(angle) * halfThick;
        
        glVertex3f(x - dx, y - dy, -halfDepth);
        glVertex3f(x + dx, y + dy, -halfDepth);
    }
    glEnd();
    
    // === BASE PLATFORM (optional decorative base) ===
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0, 0.05f, 0);
    glScalef(gateWidth + 0.5f, 0.1f, pillarDepth + 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === SIDE RECTANGULAR GATES WITH OPENINGS ===
    glColor3f(0.95f, 0.95f, 0.95f);  // Same white as pillars
    
    float wingLength = 3.0f;       // How far the gate extends
    float wingHeight = 2.5f;       // Height of the side gate
    float wingThickness = 0.3f;    // Thickness of the gate wall
    float openingWidth = 1.5f;     // Width of the walkway opening
    float openingHeight = 2.0f;    // Height of the walkway opening
    float overlap = 0.7f;         // Move closer to main gate
    
    // === LEFT RECTANGULAR GATE (with opening) ===
    
    // Left gate - Left pillar
    glPushMatrix();
    glTranslatef(-gateWidth/2 - wingLength + pillarWidth/2 + overlap, wingHeight/2, 0);
    glScalef(pillarWidth, wingHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Left gate - Right pillar (next to main gate)
    glPushMatrix();
    glTranslatef(-gateWidth/2 - pillarWidth/2 + overlap, wingHeight/2, 0);
    glScalef(pillarWidth, wingHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Left gate - Top lintel (above opening)
    float lintelHeight = wingHeight - openingHeight;
    glPushMatrix();
    glTranslatef(-gateWidth/2 - wingLength/2 + overlap, wingHeight - lintelHeight/2, 0);
    glScalef(wingLength - pillarWidth, lintelHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Left gate - Bottom base (below opening)
    float baseHeight = 0.3f;
    glPushMatrix();
    glTranslatef(-gateWidth/2 - wingLength/2 + overlap, baseHeight/2, 0);
    glScalef(wingLength - pillarWidth, baseHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === RIGHT RECTANGULAR GATE (with opening) ===
    
    // Right gate - Left pillar (next to main gate)
    glPushMatrix();
    glTranslatef(gateWidth/2 + pillarWidth/2 - overlap, wingHeight/2, 0);
    glScalef(pillarWidth, wingHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Right gate - Right pillar
    glPushMatrix();
    glTranslatef(gateWidth/2 + wingLength - pillarWidth/2 - overlap, wingHeight/2, 0);
    glScalef(pillarWidth, wingHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Right gate - Top lintel (above opening)
    glPushMatrix();
    glTranslatef(gateWidth/2 + wingLength/2 - overlap, wingHeight - lintelHeight/2, 0);
    glScalef(wingLength - pillarWidth, lintelHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Right gate - Bottom base (below opening)
    glPushMatrix();
    glTranslatef(gateWidth/2 + wingLength/2 - overlap, baseHeight/2, 0);
    glScalef(wingLength - pillarWidth, baseHeight, wingThickness);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === DECORATIVE TOP CAPS on gates ===
    glColor3f(0.92f, 0.92f, 0.92f);  // Slightly darker for accent
    
    // Left gate cap
    glPushMatrix();
    glTranslatef(-gateWidth/2 - wingLength/2 + overlap, wingHeight + 0.1f, 0);
    glScalef(wingLength + 0.2f, 0.2f, wingThickness + 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Right gate cap
    glPushMatrix();
    glTranslatef(gateWidth/2 + wingLength/2 - overlap, wingHeight + 0.1f, 0);
    glScalef(wingLength + 0.2f, 0.2f, wingThickness + 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
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
    // Draw shadow FIRST (before transforming player)
    drawEllipticalShadow(x, z, 0.4f, 0.35f, 0.4f);
    
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

// Draw a person walking/jogging on the track (similar to drawPlayer but without paddle)
void drawWalker(float x, float z, WalkerState& state, bool isMale, bool isJogging) {
    // Draw shadow FIRST
    drawEllipticalShadow(x, z, 0.4f, 0.35f, 0.4f);
    
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Face the direction of movement
    glRotatef(state.angle, 0, 1, 0);
    
    // === LEGS ===
    float legSwing = isJogging ? 35.0f : 20.0f; // Joggers have more leg movement
    
    // Shorts/pants color (vary by person)
    if (isMale) {
        glColor3f(0.2f, 0.3f, 0.5f); // Blue shorts
    } else {
        glColor3f(0.5f, 0.2f, 0.4f); // Purple/magenta
    }
    
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
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0, -0.72f, 0.08f);
    glScalef(0.18f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix(); // End left leg
    
    // Right leg (similar structure)
    if (isMale) {
        glColor3f(0.2f, 0.3f, 0.5f);
    } else {
        glColor3f(0.5f, 0.2f, 0.4f);
    }
    
    glPushMatrix();
    glTranslatef(0.15f, 0.7f, 0);
    glRotatef(state.legAngle2, 1, 0, 0);
    glTranslatef(0, -0.35f, 0);
    
    glPushMatrix();
    glScalef(0.18f, 0.4f, 0.18f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glColor3f(0.85f, 0.7f, 0.6f);
    glPushMatrix();
    glTranslatef(0, -0.2f, 0);
    glutSolidSphere(0.12f, 12, 12);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, -0.5f, 0);
    glScalef(0.15f, 0.35f, 0.15f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0, -0.72f, 0.08f);
    glScalef(0.18f, 0.1f, 0.25f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix(); // End right leg
    
    // === TORSO ===
    // Shirt color (vary by person)
    if (isMale) {
        glColor3f(0.3f, 0.7f, 0.3f); // Green shirt
    } else {
        glColor3f(0.9f, 0.6f, 0.2f); // Orange shirt
    }
    
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
    glColor3f(0.1f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(-0.08f, 0.05f, 0.18f);
    glutSolidSphere(0.03f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.08f, 0.05f, 0.18f);
    glutSolidSphere(0.03f, 8, 8);
    glPopMatrix();
    
    // Hair
    glColor3f(0.15f, 0.1f, 0.05f);
    glPushMatrix();
    glTranslatef(0, 0.15f, 0);
    glScalef(1.1f, 0.8f, 1.0f);
    glutSolidSphere(0.22f, 12, 12);
    glPopMatrix();
    
    glPopMatrix(); // End head
    
    // === ARMS (swinging naturally while walking/jogging) ===
    glColor3f(0.85f, 0.7f, 0.6f);
    
    // Left arm
    glPushMatrix();
    glTranslatef(-0.35f, 1.5f, 0);
    glRotatef(state.armSwing1, 1, 0, 0); // Swing forward/back
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
    
    // Right arm
    glPushMatrix();
    glTranslatef(0.35f, 1.5f, 0);
    glRotatef(state.armSwing2, 1, 0, 0); // Swing opposite to left arm
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
    
    glPopMatrix(); // End right arm
    
    glPopMatrix();
}

// Draw a dog (simple but recognizable)
void drawDog(float x, float z, float angle) {
    // Draw shadow
    drawEllipticalShadow(x, z, 0.3f, 0.25f, 0.3f);
    
    glPushMatrix();
    glTranslatef(x, 0, z);
    glRotatef(angle, 0, 1, 0);
    
    // Dog color (brown)
    glColor3f(0.55f, 0.35f, 0.2f);
    
    // === BODY ===
    glPushMatrix();
    glTranslatef(0, 0.35f, 0);
    glScalef(0.5f, 0.3f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === HEAD ===
    glPushMatrix();
    glTranslatef(0.3f, 0.4f, 0);
    glutSolidSphere(0.15f, 12, 12);
    
    // Snout
    glColor3f(0.5f, 0.3f, 0.15f);
    glPushMatrix();
    glTranslatef(0.12f, -0.02f, 0);
    glScalef(0.8f, 0.6f, 0.6f);
    glutSolidSphere(0.1f, 8, 8);
    glPopMatrix();
    
    // Ears
    glColor3f(0.5f, 0.3f, 0.18f);
    glPushMatrix();
    glTranslatef(-0.05f, 0.12f, -0.1f);
    glScalef(0.6f, 1.2f, 0.4f);
    glutSolidSphere(0.08f, 8, 8);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.05f, 0.12f, 0.1f);
    glScalef(0.6f, 1.2f, 0.4f);
    glutSolidSphere(0.08f, 8, 8);
    glPopMatrix();
    
    glPopMatrix(); // End head
    
    // === TAIL ===
    glColor3f(0.55f, 0.35f, 0.2f);
    glPushMatrix();
    glTranslatef(-0.28f, 0.45f, 0);
    glRotatef(45, 0, 0, 1);
    glScalef(0.08f, 0.25f, 0.08f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === LEGS (4 legs) ===
    float legPositions[][2] = {
        {0.15f, -0.12f},  // Front-left
        {0.15f, 0.12f},   // Front-right
        {-0.15f, -0.12f}, // Back-left
        {-0.15f, 0.12f}   // Back-right
    };
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(legPositions[i][0], 0.15f, legPositions[i][1]);
        glScalef(0.08f, 0.3f, 0.08f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Draw improved ground with varied park terrain
void drawGrassField() {
    // === DRAW BASE GROUND PLANE (similar to park terrain) ===
    glPushMatrix();
    
    // Draw ground in sections with different colors for variety
    for (float x = -COURT_LENGTH - 12; x < COURT_LENGTH + 12; x += 2.0f) {
        for (float z = -COURT_WIDTH - 12; z < COURT_WIDTH + 12; z += 2.0f) {
            // Skip the court area - AUTOMATICALLY SYNCED with margins!
            // We subtract a small amount (0.2f) to ensure slight overlap (no gaps)
            // if (x > -COURT_LENGTH/2 - (MARGIN_X - 0.2f) && x < COURT_LENGTH/2 + (MARGIN_X - 0.2f) &&
            //     z > -COURT_WIDTH/2 - (MARGIN_Z - 0.2f) && z < COURT_WIDTH/2 + (MARGIN_Z - 0.2f)) {
            //     continue;
            // }
            
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
    
    // === DECORATIVE GRASS BLADES REMOVED AS REQUESTED ===
    // (This creates a cleaner, flatter park ground appearance)
}

// Draw running track around the court (outside the fence)
void drawRunningTrack() {
    glPushMatrix();
    
    // === Track Parameters (in meters) ===
    // trackWidth: Chiều rộng đường chạy (width of running track)
    float trackWidth = 3.0f;  // 3 mét - đủ cho 2 làn chạy
    
    // trackOffset: Khoảng cách từ trung tâm sân đến mép TRONG của đường chạy
    //              (distance from court center to INNER edge of track)
    float trackOffset = 9.0f; // 8 mét
    // => Mép NGOÀI của đường chạy sẽ cách trung tâm sân: 8 + 3 = 11 mét
    
    // Running track surface (reddish-brown athletic track color)
    glColor3f(0.7f, 0.3f, 0.25f); // Red brick/athletic track color
    
    // Draw track as 4 rectangular segments forming a loop
    // Bottom segment
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glEnd();
    
    // Top segment
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Left segment
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Right segment
    glBegin(GL_QUADS);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // === CORNER PIECES - Fill the 4 corners ===
    // Top-left corner
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glEnd();
    
    // Top-right corner
    glBegin(GL_QUADS);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, COURT_WIDTH/2 + trackOffset + trackWidth);
    glEnd();
    
    // Bottom-left corner
    glBegin(GL_QUADS);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glEnd();
    
    // Bottom-right corner
    glBegin(GL_QUADS);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset - trackWidth);
    glVertex3f(COURT_LENGTH/2 + trackOffset + trackWidth, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.005f, -COURT_WIDTH/2 - trackOffset);
    glEnd();
    
    // White lane markings
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    // Inner lane line (bottom)
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glEnd();
    
    // Inner lane line (top)
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Inner lane line (left)
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Inner lane line (right)
    glBegin(GL_LINES);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Center lane divider (dashed line on bottom segment)
    glLineStipple(4, 0xAAAA); // Dotted line pattern
    glEnable(GL_LINE_STIPPLE);
    
    float centerLaneOffset = trackWidth / 2.0f;
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glEnd();
    
    // Center lane on top
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glEnd();
    
    // Center lane on left
    glBegin(GL_LINES);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glEnd();
    
    // Center lane on right
    glBegin(GL_LINES);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    
    // === Center lane on CORNERS (right-angle / L-shaped lines) ===
    // Top-left corner (L-shape) - meet at junction point
    glBegin(GL_LINES);
    // Horizontal segment: from left edge to junction
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glEnd();
    glBegin(GL_LINES);
    // Vertical segment: from junction to right edge
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glEnd();
    
    // Top-right corner (L-shape) - meet at junction point
    glBegin(GL_LINES);
    // Horizontal segment: from right edge to junction
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glEnd();
    glBegin(GL_LINES);
    // Vertical segment: from left edge to junction
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, COURT_WIDTH/2 + trackOffset + centerLaneOffset);
    glEnd();
    
    // Bottom-left corner (L-shape) - meet at junction point
    glBegin(GL_LINES);
    // Horizontal segment: from left edge to junction
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glEnd();
    glBegin(GL_LINES);
    // Vertical segment: from junction to right edge
    glVertex3f(-COURT_LENGTH/2 - trackOffset - centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glVertex3f(-COURT_LENGTH/2 - trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glEnd();
    
    // Bottom-right corner (L-shape) - meet at junction point
    glBegin(GL_LINES);
    // Horizontal segment: from right edge to junction
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset);
    glEnd();
    glBegin(GL_LINES);
    // Vertical segment: from left edge to junction
    glVertex3f(COURT_LENGTH/2 + trackOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glVertex3f(COURT_LENGTH/2 + trackOffset + centerLaneOffset, 0.01f, -COURT_WIDTH/2 - trackOffset - centerLaneOffset);
    glEnd();
    
    glDisable(GL_LINE_STIPPLE);
    
    // === Outer White Boundary Line ===
    // Draw a solid white line at the outermost edge of the track
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    float outerX = COURT_LENGTH/2 + trackOffset + trackWidth;
    float outerZ = COURT_WIDTH/2 + trackOffset + trackWidth;
    
    glBegin(GL_LINE_LOOP);
    // Four outer corners
    glVertex3f(-outerX, 0.01f, -outerZ); // Bottom-left
    glVertex3f(outerX, 0.01f, -outerZ);  // Bottom-right
    glVertex3f(outerX, 0.01f, outerZ);   // Top-right
    glVertex3f(-outerX, 0.01f, outerZ);  // Top-left
    glEnd();
    
    
    // === STREET LAMPS along the track ===
    // Place lamps close to the track edge
    float lampMargin = trackOffset + trackWidth + 0.3f;  // Very close to track
    
    // Bottom side (-Z) - 4 lamps
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset, -COURT_WIDTH/2 - lampMargin, -90);
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset + 10.0f, -COURT_WIDTH/2 - lampMargin, -90);
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset + 28.0f, -COURT_WIDTH/2 - lampMargin, -90);
    drawStreetLamp(COURT_LENGTH/2 + trackOffset, -COURT_WIDTH/2 - lampMargin, -90);
    
    // Top side (+Z) - 4 lamps
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset, COURT_WIDTH/2 + lampMargin, 90);
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset + 10.0f, COURT_WIDTH/2 + lampMargin, 90);
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset + 19.0f, COURT_WIDTH/2 + lampMargin, 90);
    drawStreetLamp(-COURT_LENGTH/2 - trackOffset + 28.0f, COURT_WIDTH/2 + lampMargin, 90);
    drawStreetLamp(COURT_LENGTH/2 + trackOffset, COURT_WIDTH/2 + lampMargin, 90);
    
    // Left side (-X) - 2 lamps
    drawStreetLamp(-COURT_LENGTH/2 - lampMargin, -COURT_WIDTH/2 - trackOffset + 8.0f, 0);
    drawStreetLamp(-COURT_LENGTH/2 - lampMargin, -COURT_WIDTH/2 - trackOffset + 20.0f, 0);
    
    // Right side (+X) - 2 lamps
    drawStreetLamp(COURT_LENGTH/2 + lampMargin, -COURT_WIDTH/2 - trackOffset + 8.0f, 180);
    drawStreetLamp(COURT_LENGTH/2 + lampMargin, -COURT_WIDTH/2 - trackOffset + 20.0f, 180);
    
    
    glPopMatrix();
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

// Update walker positions and animations along the running track
void updateWalkers() {
    if (isPaused) return;
    
    // Track parameters (matching drawRunningTrack)
    float trackOffset = 9.0f;
    float trackWidth = 3.0f;
    float trackMiddle = trackOffset + trackWidth / 2.0f; // Middle of track = 10.5f
    
    // Track dimensions
    float trackHalfLength = COURT_LENGTH/2 + trackMiddle;
    float trackHalfWidth = COURT_WIDTH/2 + trackMiddle;
    
    // Update each walker
    WalkerState* walkers[] = {&walker1, &walker2, &walker3, &walker4};
    bool isJogging[] = {false, false, false, false}; // All walkers walk at same pace
    
    for (int i = 0; i < 4; i++) {
        WalkerState* w = walkers[i];
        float walkSpeed = w->speed;
        
        // Update leg and arm animations (walking/jogging motion)
        float animSpeed = isJogging[i] ? 15.0f : 10.0f;
        float legAngleMax = isJogging[i] ? 40.0f : 25.0f;
        float armAngleMax = isJogging[i] ? 30.0f : 20.0f;
        
        w->legAngle1 = sin(animationTime * animSpeed) * legAngleMax;
        w->legAngle2 = -w->legAngle1; // Opposite leg
        w->armSwing1 = sin(animationTime * animSpeed) * armAngleMax;
        w->armSwing2 = -w->armSwing1; // Opposite arm
        
        // Move walker along track
        // Track segments: 0=bottom, 1=right, 2=top, 3=left
        switch(w->pathSegment) {
            case 0: // Bottom segment (moving right, +X direction)
                w->posX += walkSpeed;
                w->angle = 90;
                if (w->posX >= trackHalfLength) {
                    w->pathSegment = 1;
                    w->posX = trackHalfLength;
                }
                break;
                
            case 1: // Right segment (moving up, +Z direction)
                w->posZ += walkSpeed;
                w->angle = 0;
                if (w->posZ >= trackHalfWidth) {
                    w->pathSegment = 2;
                    w->posZ = trackHalfWidth;
                }
                break;
                
            case 2: // Top segment (moving left, -X direction)
                w->posX -= walkSpeed;
                w->angle = 270;
                if (w->posX <= -trackHalfLength) {
                    w->pathSegment = 3;
                    w->posX = -trackHalfLength;
                }
                break;
                
            case 3: // Left segment (moving down, -Z direction)
                w->posZ -= walkSpeed;
                w->angle = 180;
                if (w->posZ <= -trackHalfWidth) {
                    w->pathSegment = 0;
                    w->posZ = -trackHalfWidth;
                }
                break;
        }
    }
    
    // Keep walker2 and walker3 walking side-by-side (parallel formation)
    // Walker2 moves freely, walker3 follows with perpendicular offset
    float coupleOffset = 0.5f; // Distance between the two people
    
    // Sync their segment and angle
    walker3.pathSegment = walker2.pathSegment;
    walker3.angle = walker2.angle;
    
    // Position walker3 relative to walker2 with perpendicular offset
    switch(walker2.pathSegment) {
        case 0: // Bottom segment - walking along +X direction
            walker3.posX = walker2.posX; // Same X (forward) position
            walker3.posZ = walker2.posZ + coupleOffset; // Offset to the side
            break;
            
        case 1: // Right segment - walking along +Z direction
            walker3.posZ = walker2.posZ; // Same Z (forward) position
            walker3.posX = walker2.posX + coupleOffset; // Offset to the side
            break;
            
        case 2: // Top segment - walking along -X direction
            walker3.posX = walker2.posX; // Same X (forward) position
            walker3.posZ = walker2.posZ - coupleOffset; // Offset to the side (opposite)
            break;
            
        case 3: // Left segment - walking along -Z direction
            walker3.posZ = walker2.posZ; // Same Z (forward) position
            walker3.posX = walker2.posX - coupleOffset; // Offset to the side (opposite)
            break;
    }
    
    // Update dog position to follow walker1 (person with dog)
    // Dog walks slightly behind and to the side
    float dogOffsetDistance = 1.5f;
    float angleRad = (walker1.angle - 45) * PI / 180.0f; // 45 degrees offset
    dogPosX = walker1.posX - cos(angleRad) * dogOffsetDistance;
    dogPosZ = walker1.posZ - sin(angleRad) * dogOffsetDistance;
    dogAngle = walker1.angle; // Dog faces same direction as walker1
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
    drawRunningTrack(); // Draw running track around the court
    drawPerimeterFence(); // Draw ornamental iron fence around entire map
    drawSun();         // Draw sun with rays
    
    // === CLOUDS - Floating in the sky ===
    // Daytime: All clouds visible
    // Nighttime: Fewer clouds (only high and some medium clouds)
    bool isDaytime = (timeOfDay >= 0.3f && timeOfDay <= 0.7f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // === HIGH CLOUDS (25-30m) - Always visible (day and night) ===
    drawCloud(0.0f, 28.0f, -15.0f, 1.8f);
    drawCloud(-5.0f, 30.0f, 15.0f, 1.0f);
    drawCloud(5.0f, 29.0f, 8.0f, 1.6f);
    drawCloud(-18.0f, 27.0f, -12.0f, 1.3f);
    drawCloud(22.0f, 30.0f, 5.0f, 1.1f);
    drawCloud(-28.0f, 28.0f, 18.0f, 1.4f);
    drawCloud(12.0f, 29.0f, -18.0f, 1.2f);
    
    if (isDaytime) {
        // === DAYTIME: Show all medium and low clouds ===
        
        // MEDIUM CLOUDS (18-24m)
        drawCloud(-15.0f, 20.0f, -10.0f, 1.2f);
        drawCloud(10.0f, 22.0f, -5.0f, 1.5f);
        drawCloud(20.0f, 18.0f, 10.0f, 1.3f);
        drawCloud(-20.0f, 21.0f, 5.0f, 1.1f);
        drawCloud(15.0f, 24.0f, -20.0f, 1.4f);
        drawCloud(-10.0f, 19.0f, 20.0f, 1.0f);
        drawCloud(-25.0f, 23.0f, -8.0f, 1.2f);
        drawCloud(8.0f, 20.0f, 12.0f, 1.7f);
        drawCloud(-12.0f, 22.0f, -15.0f, 1.3f);
        drawCloud(25.0f, 21.0f, -3.0f, 1.5f);
        drawCloud(-8.0f, 24.0f, 22.0f, 1.0f);
        
        // LOW CLOUDS (12-17m)
        drawCloud(-18.0f, 14.0f, -8.0f, 2.0f);
        drawCloud(14.0f, 13.0f, -12.0f, 1.8f);
        drawCloud(-6.0f, 15.0f, 18.0f, 1.9f);
        drawCloud(18.0f, 12.0f, 6.0f, 2.2f);
        drawCloud(-22.0f, 16.0f, 12.0f, 1.7f);
        drawCloud(6.0f, 14.0f, -18.0f, 2.1f);
        drawCloud(-14.0f, 17.0f, -5.0f, 1.6f);
        drawCloud(22.0f, 15.0f, -15.0f, 1.9f);
        drawCloud(-3.0f, 13.0f, 10.0f, 2.3f);
        drawCloud(10.0f, 16.0f, 16.0f, 1.8f);
        drawCloud(-26.0f, 14.0f, -18.0f, 2.0f);
        drawCloud(26.0f, 13.0f, 8.0f, 1.7f);
        drawCloud(0.0f, 15.0f, -22.0f, 2.4f);
        drawCloud(-10.0f, 12.0f, -12.0f, 2.1f);
    } else {
        // === NIGHTTIME: Show only some medium clouds (fewer) ===
        drawCloud(-15.0f, 20.0f, -10.0f, 1.2f);
        drawCloud(20.0f, 18.0f, 10.0f, 1.3f);
        drawCloud(15.0f, 24.0f, -20.0f, 1.4f);
        drawCloud(-25.0f, 23.0f, -8.0f, 1.2f);
        drawCloud(25.0f, 21.0f, -3.0f, 1.5f);
    }
    
    glDisable(GL_BLEND);
    
    drawCourt();
    drawNet();
    drawBall();

    
    // Draw players using DYNAMIC POSITIONS
    drawPlayer(player1State.posX, player1State.posZ, player1State, true);   
    drawPlayer(player2State.posX, player2State.posZ, player2State, false);
    
    // === WALKERS ON RUNNING TRACK - People enjoying the park ===
    // Walker 1: Person with dog (male, walking)
    drawWalker(walker1.posX, walker1.posZ, walker1, true, false);
    drawDog(dogPosX, dogPosZ, dogAngle);
    
    // Walker 2 & 3: Walking couple (close together)
    drawWalker(walker2.posX, walker2.posZ, walker2, true, false);   // Male
    drawWalker(walker3.posX, walker3.posZ, walker3, false, false);  // Female
    
    // Walker 4: Walker (male, walking at same speed)
    drawWalker(walker4.posX, walker4.posZ, walker4, true, false);
    
    // Draw park scenery - BEAUTIFUL ENHANCED PARK ATMOSPHERE! 🌳🌸
    
    // === PATHWAYS - REMOVED AS REQUESTED ===
    /*
    // Main path around the court
    drawPath(0, -COURT_WIDTH/2 - 2.5f, 2.0f, 20.0f, 0);  // Bottom path
    drawPath(0, COURT_WIDTH/2 + 2.5f, 2.0f, 20.0f, 0);   // Top path
    drawPath(-COURT_LENGTH/2 - 2.5f, 0, 2.0f, 15.0f, 90); // Left path
    drawPath(COURT_LENGTH/2 + 2.5f, 0, 2.0f, 15.0f, 90);  // Right path
    
    // Diagonal decorative paths
    drawPath(-COURT_LENGTH/2 - 5, -COURT_WIDTH/2 - 5, 1.5f, 8.0f, 45);
    drawPath(COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 5, 1.5f, 8.0f, 45);
    */
    
    // === FENCES - Beautiful wooden fencing (FIXED alignment) ===
    
    // CORNER POSTS (explicitly placed for perfect alignment)
    drawFence(-COURT_LENGTH/2 - 8, -COURT_WIDTH/2 - 5, 0);  // Bottom-left corner
    drawFence(COURT_LENGTH/2 + 8, -COURT_WIDTH/2 - 5, 0);   // Bottom-right corner
    drawFence(-COURT_LENGTH/2 - 8, COURT_WIDTH/2 + 5, 0);   // Top-left corner
    drawFence(COURT_LENGTH/2 + 8, COURT_WIDTH/2 + 5, 0);    // Top-right corner
    
    // Bottom fence line (horizontal) - excluding corners
    for (float x = -COURT_LENGTH/2 - 8 + 1.2f; x < COURT_LENGTH/2 + 8; x += 1.2f) {
        drawFence(x, -COURT_WIDTH/2 - 5, 0);
    }
    // Top fence line (horizontal) - excluding corners
    for (float x = -COURT_LENGTH/2 - 8 + 1.2f; x < COURT_LENGTH/2 + 8; x += 1.2f) {
        drawFence(x, COURT_WIDTH/2 + 5, 0);
    }
    // Left fence line (vertical) - excluding corners
    for (float z = -COURT_WIDTH/2 - 5 + 1.2f; z < COURT_WIDTH/2 + 5; z += 1.2f) {
        drawFence(-COURT_LENGTH/2 - 8, z, 90);
    }
    // Right fence line (vertical) - excluding corners
    for (float z = -COURT_WIDTH/2 - 5 + 1.2f; z < COURT_WIDTH/2 + 5; z += 1.2f) {
        drawFence(COURT_LENGTH/2 + 8, z, 90);
    }
    
    // === TREES - Lush forest-like environment ===
    // Corner trees (large, prominent)
    // drawTree(-COURT_LENGTH/2 - 4, -COURT_WIDTH/2 - 4);
    // drawTree(-COURT_LENGTH/2 - 4, COURT_WIDTH/2 + 4);
    // drawTree(COURT_LENGTH/2 + 4, -COURT_WIDTH/2 - 4);
    // drawTree(COURT_LENGTH/2 + 4, COURT_WIDTH/2 + 4);
    
    // Perimeter trees (creating a natural border)
    drawTree(-COURT_LENGTH/2 - 6, 0);
    drawTree(COURT_LENGTH/2 + 6, 0);
    // drawTree(0, -COURT_WIDTH/2 - 6);
    // drawTree(0, COURT_WIDTH/2 + 6);
    
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
    // drawTree(-COURT_LENGTH/2 - 9, -COURT_WIDTH/2 - 6);
    // drawTree(-COURT_LENGTH/2 - 9, COURT_WIDTH/2 + 6);
    // drawTree(COURT_LENGTH/2 + 9, -COURT_WIDTH/2 - 6);
    // drawTree(COURT_LENGTH/2 + 9, COURT_WIDTH/2 + 6);
    
    // Clustered trees for natural look
    drawTree(-COURT_LENGTH/2 - 8, -COURT_WIDTH/2);
    drawTree(COURT_LENGTH/2 + 8, COURT_WIDTH/2);
    // drawTree(-3, -COURT_WIDTH/2 - 7);
    // drawTree(3, COURT_WIDTH/2 + 7);
    
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
    // drawBush(-2.5f, -COURT_WIDTH/2 - 6);
    // drawBush(2.5f, COURT_WIDTH/2 + 6);
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
    
    // === COURT FLOODLIGHTS - Professional stadium lighting ===
    // 4 tall floodlights at corners (auto ON at night, OFF during day)
    drawCourtFloodlight(-COURT_LENGTH/2 - 2, -COURT_WIDTH/2 - 2);  // Bottom-left
    drawCourtFloodlight(COURT_LENGTH/2 + 2, -COURT_WIDTH/2 - 2);   // Bottom-right
    drawCourtFloodlight(-COURT_LENGTH/2 - 2, COURT_WIDTH/2 + 2);   // Top-left
    drawCourtFloodlight(COURT_LENGTH/2 + 2, COURT_WIDTH/2 + 2);    // Top-right
    
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
    
    // === FOUNTAINS REMOVED AS REQUESTED ===
    // drawFountain(0, COURT_WIDTH/2 + 7);
    // drawFountain(-COURT_LENGTH/2 - 8, 0);
    // drawFountain(COURT_LENGTH/2 + 8, 0);
    
    // === LARGE GARDEN BEDS - Beautiful flower beds! ===
    // // Corner garden beds (large, prominent)
    // drawGardenBed(-COURT_LENGTH/2 - 5.5f, -COURT_WIDTH/2 - 4, 2.5f, 2.0f);
    // drawGardenBed(COURT_LENGTH/2 + 5.5f, COURT_WIDTH/2 + 4, 2.5f, 2.0f);
    // drawGardenBed(-COURT_LENGTH/2 - 5.5f, COURT_WIDTH/2 + 4, 2.5f, 2.0f);
    // drawGardenBed(COURT_LENGTH/2 + 5.5f, -COURT_WIDTH/2 - 4, 2.5f, 2.0f);
    
    // // Linear garden beds along paths
    // drawGardenBed(-COURT_LENGTH/2 + 1, -COURT_WIDTH/2 - 5.5f, 4.0f, 1.5f);
    // drawGardenBed(COURT_LENGTH/2 - 1, COURT_WIDTH/2 + 5.5f, 4.0f, 1.5f);
    
    // // Side garden beds
    // drawGardenBed(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2, 1.8f, 3.0f);
    // drawGardenBed(COURT_LENGTH/2 + 7, COURT_WIDTH/2, 1.8f, 3.0f);
    
    // // Entrance garden beds
    // drawGardenBed(0, -COURT_WIDTH/2 - 7.5f, 3.5f, 1.2f);
    // drawGardenBed(0, COURT_WIDTH/2 + 8.5f, 5.0f, 1.5f);
    
    // === CIRCULAR FLOWER PATCHES - Round decorative gardens ===
    // // Main circular patches (highly visible)
    // drawCircularFlowerPatch(-COURT_LENGTH/2 - 8.5f, -COURT_WIDTH/2 - 5.5f, 0.8f);
    // drawCircularFlowerPatch(COURT_LENGTH/2 + 8.5f, COURT_WIDTH/2 + 5.5f, 0.8f);
    
    // // Medium circular patches
    // drawCircularFlowerPatch(-COURT_LENGTH/2 - 6, -COURT_WIDTH/2 + 1, 0.6f);
    // drawCircularFlowerPatch(COURT_LENGTH/2 + 6, COURT_WIDTH/2 - 1, 0.6f);
    // drawCircularFlowerPatch(-COURT_LENGTH/2 - 3.5f, -COURT_WIDTH/2 - 5, 0.5f);
    // drawCircularFlowerPatch(COURT_LENGTH/2 + 3.5f, COURT_WIDTH/2 + 5, 0.5f);
    
    // // Small accent patches
    // drawCircularFlowerPatch(-COURT_LENGTH/2 - 9, COURT_WIDTH/2 + 2, 0.5f);
    // drawCircularFlowerPatch(COURT_LENGTH/2 + 9, -COURT_WIDTH/2 - 2, 0.5f);
    // drawCircularFlowerPatch(-2, -COURT_WIDTH/2 - 8.5f, 0.4f);
    // drawCircularFlowerPatch(2, COURT_WIDTH/2 + 9, 0.4f);
    
    // // Additional scattered patches for natural look
    // drawCircularFlowerPatch(-COURT_LENGTH/2 - 5, COURT_WIDTH/2 + 6, 0.45f);
    // drawCircularFlowerPatch(COURT_LENGTH/2 + 5, -COURT_WIDTH/2 - 6, 0.45f);
    
    // === PICNIC TABLES - Park seating and gathering areas ===
    // Main picnic area (back of court)
    drawPicnicTable(-4, COURT_WIDTH/2 + 6, 0);
    drawPicnicTable(4, COURT_WIDTH/2 + 6, 0);
    
    // Side picnic areas
    // drawPicnicTable(-COURT_LENGTH/2 - 7, -COURT_WIDTH/2 - 4.5f, 45);
    // drawPicnicTable(COURT_LENGTH/2 + 7, COURT_WIDTH/2 + 4.5f, -45);
    
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
    
    // === TREES OUTSIDE PERIMETER FENCE - Natural forest border ===
    // Hàng rào đen ở vị trí ±15.0f từ tâm sân
    // Đặt cây bên ngoài (xa hơn 15.0f) với các kích thước khác nhau
    
    // Bottom fence line (horizontal) - Outside trees
    drawLargeTree(-COURT_LENGTH/2 - 17, -COURT_WIDTH/2 - 15.5f);
    drawMediumTree(-COURT_LENGTH/2 - 19, -COURT_WIDTH/2 - 15.2f);
    drawSmallTree(-COURT_LENGTH/2 - 21, -COURT_WIDTH/2 - 15.8f);
    drawMediumTree(-COURT_LENGTH/2 - 10, -COURT_WIDTH/2 - 16.0f);
    drawLargeTree(-COURT_LENGTH/2 - 5, -COURT_WIDTH/2 - 15.5f);
    drawSmallTree(-COURT_LENGTH/2 - 2, -COURT_WIDTH/2 - 16.2f);
    // Skip center area for gate
    drawMediumTree(COURT_LENGTH/2 + 2, -COURT_WIDTH/2 - 15.7f);
    drawLargeTree(COURT_LENGTH/2 + 5, -COURT_WIDTH/2 - 16.0f);
    drawSmallTree(COURT_LENGTH/2 + 10, -COURT_WIDTH/2 - 15.5f);
    drawMediumTree(COURT_LENGTH/2 + 17, -COURT_WIDTH/2 - 15.8f);
    drawLargeTree(COURT_LENGTH/2 + 19, -COURT_WIDTH/2 - 16.2f);
    drawSmallTree(COURT_LENGTH/2 + 21, -COURT_WIDTH/2 - 15.6f);
    
    // Top fence line (horizontal) - Outside trees
    drawMediumTree(-COURT_LENGTH/2 - 17, COURT_WIDTH/2 + 15.5f);
    drawLargeTree(-COURT_LENGTH/2 - 19, COURT_WIDTH/2 + 16.0f);
    drawSmallTree(-COURT_LENGTH/2 - 21, COURT_WIDTH/2 + 15.7f);
    drawLargeTree(-COURT_LENGTH/2 - 10, COURT_WIDTH/2 + 15.8f);
    drawMediumTree(-COURT_LENGTH/2 - 5, COURT_WIDTH/2 + 16.2f);
    drawSmallTree(-COURT_LENGTH/2 - 2, COURT_WIDTH/2 + 15.5f);

    drawMediumTree(-COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 15.5f);
    drawSmallTree(-COURT_LENGTH/2 + 6.8, COURT_WIDTH/2 + 15.5f);
    drawLargeTree(-COURT_LENGTH/2 + 9, COURT_WIDTH/2 + 15.5f);
    drawMediumTree(-COURT_LENGTH/2 + 12, COURT_WIDTH/2 + 15.5f);

    drawLargeTree(COURT_LENGTH/2 + 2, COURT_WIDTH/2 + 15.9f);
    drawMediumTree(COURT_LENGTH/2 + 5, COURT_WIDTH/2 + 16.1f);
    drawSmallTree(COURT_LENGTH/2 + 10, COURT_WIDTH/2 + 15.6f);
    drawLargeTree(COURT_LENGTH/2 + 17, COURT_WIDTH/2 + 15.8f);
    drawMediumTree(COURT_LENGTH/2 + 19, COURT_WIDTH/2 + 16.3f);
    drawSmallTree(COURT_LENGTH/2 + 21, COURT_WIDTH/2 + 15.4f);
    
    // Left fence line (vertical) - Outside trees
    drawLargeTree(-COURT_LENGTH/2 - 16.0f, -COURT_WIDTH/2 - 10);
    drawMediumTree(-COURT_LENGTH/2 - 15.5f, -COURT_WIDTH/2 - 5);
    drawSmallTree(-COURT_LENGTH/2 - 16.2f, -COURT_WIDTH/2 - 2);
    drawMediumTree(-COURT_LENGTH/2 - 15.8f, COURT_WIDTH/2 + 2);
    drawLargeTree(-COURT_LENGTH/2 - 16.1f, COURT_WIDTH/2 + 5);
    drawSmallTree(-COURT_LENGTH/2 - 15.6f, COURT_WIDTH/2 + 10);
    
    // Right fence line (vertical) - Outside trees
    drawMediumTree(COURT_LENGTH/2 + 15.7f, -COURT_WIDTH/2 - 10);
    drawLargeTree(COURT_LENGTH/2 + 16.2f, -COURT_WIDTH/2 - 5);
    drawSmallTree(COURT_LENGTH/2 + 15.5f, -COURT_WIDTH/2 - 2);
    drawLargeTree(COURT_LENGTH/2 + 15.9f, COURT_WIDTH/2 + 2);
    drawMediumTree(COURT_LENGTH/2 + 16.3f, COURT_WIDTH/2 + 5);
    drawSmallTree(COURT_LENGTH/2 + 15.4f, COURT_WIDTH/2 + 10);
    
    // Corner accent trees (extra large for emphasis)
    drawLargeTree(-COURT_LENGTH/2 - 18, -COURT_WIDTH/2 - 17);
    drawLargeTree(COURT_LENGTH/2 + 18, -COURT_WIDTH/2 - 17);
    drawLargeTree(-COURT_LENGTH/2 - 18, COURT_WIDTH/2 + 17);
    drawLargeTree(COURT_LENGTH/2 + 18, COURT_WIDTH/2 + 17);
    
    // === ENTRANCE GATE - Parabolic arch at park entrance ===
    // Position: Front center, outside the running track
    drawArchGate(0, -COURT_WIDTH/2 - 15.0f);
    
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
    updateWalkers();  // Update people walking/jogging on track
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
    
    // === ENHANCED LIGHTING SETUP (Phong/Blinn-Phong) ===
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Smooth shading for better gradients
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);  // Normalize normals for correct lighting
    
    // Set up light properties (will be updated in setupLighting)
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};   // Ambient light
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.7f, 1.0f};   // Main light color
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};  // Specular highlights
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    // Set default material specularity (shiny objects)
    GLfloat matSpecular[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat matShininess[] = {32.0f};
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    
    // === ANTI-ALIASING ===
    #ifndef GL_MULTISAMPLE
    #define GL_MULTISAMPLE 0x809D  // OpenGL 1.3+ extension
    #endif
    glEnable(GL_MULTISAMPLE);  // MSAA for smooth edges
    glEnable(GL_LINE_SMOOTH);  // Smooth lines
    glEnable(GL_POINT_SMOOTH); // Smooth points
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
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
    // Enable MSAA (Anti-aliasing) for smooth edges
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Pickleball Playground Scene - Enhanced Graphics");
    
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
