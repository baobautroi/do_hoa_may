/*
 * Enhanced Graphics Utilities for Pickleball Scene
 * Provides advanced rendering functions: shadows, skybox, etc.
 */

#ifndef GRAPHICS_UTILS_H
#define GRAPHICS_UTILS_H

#include <GL/glut.h>
#include <cmath>

// ============================================================================
// SIMPLE SHADOW RENDERING (Fake shadows under objects)
// ============================================================================

/**
 * Draw a simple circular shadow on the ground
 * @param x, z: Center position on ground
 * @param radius: Shadow size
 * @param opacity: Shadow darkness (0.0 = transparent, 1.0 = solid black)
 */
void drawSimpleShadow(float x, float z, float radius, float opacity = 0.4f) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, opacity); // Black shadow with alpha
    
    glPushMatrix();
    glTranslatef(x, 0.02f, z); // Slightly above ground to avoid z-fighting
    glRotatef(-90, 1, 0, 0);  // Rotate to horizontal
    
    // Draw filled circle using triangle fan
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Center
    for (int i = 0; i <= 32; i++) {
        float angle = (i * 6.28318f) / 32.0f;
        glVertex2f(cosf(angle) * radius, sinf(angle) * radius);
    }
    glEnd();
    
    glPopMatrix();
    glPopAttrib();
}

/**
 * Draw an elliptical shadow (for objects viewed at angle)
 * @param x, z: Center position
 * @param radiusX, radiusZ: Ellipse radii
 */
void drawEllipticalShadow(float x, float z, float radiusX, float radiusZ, float opacity = 0.4f) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, opacity);
    
    glPushMatrix();
    glTranslatef(x, 0.02f, z);
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
// SKYBOX / ENHANCED BACKGROUND
// ============================================================================

/**
 * Draw a gradient sky dome (simple skybox alternative)
 * @param topColor: RGB color at zenith
 * @param horizonColor: RGB color at horizon
 */
void drawGradientSkyDome(float topR, float topG, float topB,
                         float horizonR, float horizonG, float horizonB) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glPushMatrix();
    
    // Draw hemisphere with gradient
    int slices = 32;
    int stacks = 16;
    float radius = 100.0f; // Large enough to enclose scene
    
    for (int i = 0; i < stacks; i++) {
        float lat0 = (i * 1.57079f) / stacks; // 0 to PI/2
        float lat1 = ((i + 1) * 1.57079f) / stacks;
        
        float z0 = cosf(lat0);
        float r0 = sinf(lat0);
        float z1 = cosf(lat1);
        float r1 = sinf(lat1);
        
        // Interpolate color based on height
        float t0 = (float)i / stacks;
        float t1 = (float)(i + 1) / stacks;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++) {
            float lng = (j * 6.28318f) / slices;
            float x = cosf(lng);
            float y = sinf(lng);
            
            // Top vertex color (interpolated)
            glColor3f(topR + (horizonR - topR) * t0,
                     topG + (horizonG - topG) * t0,
                     topB + (horizonB - topB) * t0);
            glVertex3f(r0 * x * radius, z0 * radius, r0 * y * radius);
            
            // Bottom vertex color
            glColor3f(topR + (horizonR - topR) * t1,
                     topG + (horizonG - topG) * t1,
                     topB + (horizonB - topB) * t1);
            glVertex3f(r1 * x * radius, z1 * radius, r1 * y * radius);
        }
        glEnd();
    }
    
    glPopMatrix();
    glPopAttrib();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/**
 * Draw clouds in the sky (simple billboards)
 */
void drawSimpleClouds(float timeOfDay) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Cloud positions
    float cloudPositions[][3] = {
        {-30, 40, -50},
        {20, 45, -60},
        {-10, 42, -55},
        {40, 38, -45}
    };
    
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
        glTranslatef(cloudPositions[i][0], cloudPositions[i][1], cloudPositions[i][2]);
        
        // Billboard technique: always face camera
        glRotatef(0, 0, 1, 0);
        
        // Cloud color (white with alpha)
        glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
        
        // Draw cloud as overlapping circles
        for (int j = 0; j < 3; j++) {
            glPushMatrix();
            glTranslatef((j - 1) * 3.0f, 0, 0);
            
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0, 0);
            for (int k = 0; k <= 16; k++) {
                float angle = (k * 6.28318f) / 16.0f;
                glVertex3f(cosf(angle) * 5.0f, sinf(angle) * 2.5f, 0);
            }
            glEnd();
            
            glPopMatrix();
        }
        
        glPopMatrix();
    }
    
    glPopAttrib();
}

#endif // GRAPHICS_UTILS_H
