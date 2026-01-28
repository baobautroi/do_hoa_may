/*
 * Enhanced Graphics Utilities v2 (FIXED)
 * Fixes: Skybox duplication, Shadow artifacts
 */

#ifndef GRAPHICS_UTILS_V2_H
#define GRAPHICS_UTILS_V2_H

#include <GL/glut.h>
#include <cmath>

// ============================================================================
// SIMPLE SHADOW RENDERING - FIXED VERSION
// ============================================================================

/**
 * Draw a simple circular shadow on the ground (SAFE VERSION)
 * @param x, z: Center position on ground
 * @param radius: Shadow size
 * @param opacity: Shadow darkness (0.0 = transparent, 1.0 = solid black)
 */
void drawSimpleShadow(float x, float z, float radius, float opacity = 0.3f) {
    // Save current state
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Disable lighting for shadow (pure alpha blend)
    glDisable(GL_LIGHTING);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // CRITICAL: Don't write to depth buffer (prevents duplication)
    glDepthMask(GL_FALSE);
    
    // Black shadow with alpha
    glColor4f(0.0f, 0.0f, 0.0f, opacity);
    
    glPushMatrix();
    glTranslatef(x, 0.015f, z); // Slightly above ground to avoid z-fighting
    glRotatef(-90, 1, 0, 0);  // Rotate to horizontal
    
    // Draw filled circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Center
    for (int i = 0; i <= 32; i++) {
        float angle = (i * 6.28318f) / 32.0f;
        glVertex2f(cosf(angle) * radius, sinf(angle) * radius);
    }
    glEnd();
    
    glPopMatrix();
    
    // Restore state
    glPopAttrib();
}

/**
 * Draw an elliptical shadow (for objects at angle)
 */
void drawEllipticalShadow(float x, float z, float radiusX, float radiusZ, float opacity = 0.3f) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // CRITICAL FIX
    
    glColor4f(0.0f, 0.0f, 0.0f, opacity);
    
    glPushMatrix();
    glTranslatef(x, 0.015f, z);
    glRotatef(-90, 1, 0, 0);
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0);
    for (int i = 0; i <= 32; i++) {
        float angle = (i * 6.28318f) / 32.0f;
        glVertex2f(cosf(angle) * radiusX, sinf(angle) * radiusZ);
    }
    glEnd();
    
    glPopMatrix();
    glPopAttrib();
}

// ============================================================================
// SKYBOX - SIMPLIFIED & SAFE VERSION
// ============================================================================

/**
 * Draw a simple gradient sky (NO GEOMETRY, just clear color)
 * Call this BEFORE clearing buffers
 */
void setupSkyGradient(float topR, float topG, float topB) {
    // Just set clear color - simplest, safest
    glClearColor(topR, topG, topB, 1.0f);
}

/**
 * Draw simple clouds (SAFE VERSION)
 * Uses billboards without depth write
 */
void drawSimpleClouds(float timeOfDay) {
    // Only draw during day
    if (timeOfDay < 0.25f || timeOfDay > 0.75f) return;
    
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // Don't write depth
    
    // Cloud positions (far away)
    float cloudPositions[][3] = {
        {-30, 35, -40},
        {20, 38, -45},
        {-10, 36, -42},
        {35, 34, -38}
    };
    
    glColor4f(1.0f, 1.0f, 1.0f, 0.6f); // White with alpha
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(cloudPositions[i][0], cloudPositions[i][1], cloudPositions[i][2]);
        
        // Draw as overlapping circles
        for (int j = 0; j < 3; j++) {
            glPushMatrix();
            glTranslatef((j - 1) * 2.5f, 0, 0);
            glutSolidSphere(3.0f, 12, 12);
            glPopMatrix();
        }
        
        glPopMatrix();
    }
    
    glPopAttrib();
}

#endif // GRAPHICS_UTILS_V2_H
