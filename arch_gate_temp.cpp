// Draw Parabolic Arch Gate (White entrance gate)
void drawArchGate(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Gate parameters
    float gateWidth = 8.0f;        // Total width of the arch
    float gateHeight = 5.0f;       // Height of the arch
    float pillarWidth = 0.6f;      // Width of each pillar
    float pillarDepth = 0.6f;      // Depth of each pillar
    float archThickness = 0.5f;    // Thickness of the arch
    
    // White color for the gate
    glColor3f(0.95f, 0.95f, 0.95f);
    
    // === LEFT PILLAR ===
    glPushMatrix();
    glTranslatef(-gateWidth/2 + pillarWidth/2, gateHeight/2, 0);
    glScalef(pillarWidth, gateHeight, pillarDepth);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === RIGHT PILLAR ===
    glPushMatrix();
    glTranslatef(gateWidth/2 - pillarWidth/2, gateHeight/2, 0);
    glScalef(pillarWidth, gateHeight, pillarDepth);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // === PARABOLIC ARCH ===
    // Draw arch as series of small boxes forming a parabola
    int numSegments = 40;
    float archWidth = gateWidth - pillarWidth * 2;  // Width between pillars
    
    for (int i = 0; i <= numSegments; i++) {
        float t = (float)i / numSegments;  // 0 to 1
        float xPos = -archWidth/2 + t * archWidth;
        
        // Parabolic equation: y = -a*x^2 + h
        // Where h is max height, positioned at x=0
        float normalizedX = (xPos / (archWidth/2));  // -1 to 1
        float yPos = gateHeight * (1.0f - normalizedX * normalizedX * 0.3f);
        
        glPushMatrix();
        glTranslatef(xPos, yPos, 0);
        
        // Calculate rotation angle for smooth curve
        float angle = 0;
        if (i < numSegments) {
            float nextT = (float)(i + 1) / numSegments;
            float nextX = -archWidth/2 + nextT * archWidth;
            float nextNormX = (nextX / (archWidth/2));
            float nextY = gateHeight * (1.0f - nextNormX * nextNormX * 0.3f);
            angle = atan2(nextY - yPos, nextX - xPos) * 180.0f / PI;
        }
        
        glRotatef(angle, 0, 0, 1);
        glScalef(archWidth/numSegments * 1.2f, archThickness, pillarDepth);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    
    // === BASE PLATFORM (optional decorative base) ===
    glColor3f(0.9f, 0.9f, 0.9f);
    glPushMatrix();
    glTranslatef(0, 0.05f, 0);
    glScalef(gateWidth + 0.5f, 0.1f, pillarDepth + 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPopMatrix();
}
