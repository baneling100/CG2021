
#include <iostream>
#include "math.h"
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

static unsigned int width = 1000;
static unsigned int height = 1000;

static bool mouseRotatePressed = false;
static bool mouseMovePressed   = false;
static bool mouseZoomPressed   = false;
static bool mouseDollyPressed  = false;
static bool seekKeyPressed     = false;

static float originX = 0.0f, originY = 0.0f, originZ = 0.0f;
static float orientX = 1.0f, orientY = 0.0f, orientZ = 0.0f;
static float distance = 3.0f;
static float fovy = 45.0f; // field of view angle
static float upX = 0.0f, upY = 1.0f, upZ = 0.0f;
static int	lastX = 0, lastY = 0;
static float angularSpeed = 0.003f;

static bool fullScreen = false;

static float scale = 0.001f, dist = 0.8f, thickness = 0.02f;
struct Data {
    float x;
    float y;
    float pivot_x;
    float pivot_y;
    float fold_angle;
} joint[5][5] = {{{-91, -29, 0, 46, 80.f}, {-106, 127, 61, 26, 45.f}, {-44, 95, 46, 20, 45.f},
                  {-34, 67, 42, 21, 0.f}, {0, 0, 1.f, 0.f, 0.f}},
                 {{-39, -2, 54, 13, 0.f}, {-44, 222, 53, 7, 90.f}, {-9, 141, 41, 3, 110.f},
                  {-2, 82, 42, -1, 90.f}, {0, 59, 48, -2, 0.f}},
                 {{0, 0, 46, 2, 0.f}, {-4, 222, 32, 0, 90.f}, {6, 156, 34, -1, 110.f},
                  {4, 98, 35, -2, 90.f}, {4, 58, 43, -5, 0.f}},
                 {{38, -4, 32, -9, 0.f}, {28, 199, 49, -2, 90.f}, {7, 145, 35, -4, 110.f},
                  {11, 97, 36, -3, 90.f}, {5, 60, 34, -5, 0.f}},
                 {{65, -20, 34, 24, 0.f}, {65, 183, 46, -9, 90.f}, {17, 113, 35, -9, 110.f},
                  {11, 63, 32, -8, 90.f}, {6, 52, 38, -5, 0.f}}};

void init()
{
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++) {
            joint[i][j].x *= scale;
            joint[i][j].y *= scale;
            double size = sqrt(joint[i][j].pivot_x * joint[i][j].pivot_x +
                               joint[i][j].pivot_y * joint[i][j].pivot_y);
            joint[i][j].pivot_x /= size;
            joint[i][j].pivot_y /= size;
        }
}


void draw_hexahedron(glm::vec4 u1, glm::vec4 u2, glm::vec4 u3, glm::vec4 u4,
                     glm::vec4 v1, glm::vec4 v2, glm::vec4 v3, glm::vec4 v4)
{
    // u1 u2 u3 u4
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(u1.x, u1.y, u1.z);
    glVertex3f(u2.x, u2.y, u2.z);
    glVertex3f(u3.x, u3.y, u3.z);
    glVertex3f(u4.x, u4.y, u4.z);
    glEnd();
    
    // v1 v2 v3 v4
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glEnd();

    // u1 u2 v2 v1
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(u1.x, u1.y, u1.z);
    glVertex3f(u2.x, u2.y, u2.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v1.x, v1.y, v1.z);
    glEnd();

    // u2 u3 v3 v2
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(u2.x, u2.y, u2.z);
    glVertex3f(u3.x, u3.y, u3.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glEnd();

    // u3 u4 v4 v3
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(u3.x, u3.y, u3.z);
    glVertex3f(u4.x, u4.y, u4.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glEnd();

    // u4 u1 v1 v4
    glBegin(GL_POLYGON);
    glColor3f(224.f / 255.f, 225.f / 255.f, 215.f / 255.f);
    glVertex3f(u4.x, u4.y, u4.z);
    glVertex3f(u1.x, u1.y, u1.z);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v4.x, v4.y, v4.z);
    glEnd();
}

void draw_line(glm::vec4 u1, glm::vec4 u2)
{
    glBegin(GL_LINES);
    glColor3f(0.f, 0.f, 0.f);
    glVertex3f(u1.x, u1.y, u1.z);
    glVertex3f(u2.x, u2.y, u2.z);
    glEnd();
}

void draw_hand(bool enable[])
{
    for (int i = 0; i < 5; i++) {
        for (int j = 0; i ? j < 4 : j < 3; j++) {
            glPushMatrix();
                // draw skeleton
                glTranslatef(joint[i][j].x, joint[i][j].y, 0.f);
                float angle = enable[i] ? 0.0f : joint[i][j].fold_angle;
                glRotatef(angle, joint[i][j].pivot_x, joint[i][j].pivot_y, 0.f);
                glBegin(GL_LINES);
                glColor3f(1.f, 1.f, 1.f);
                glVertex3f(0.f, 0.f, 0.f);
                glVertex3f(joint[i][j + 1].x, joint[i][j + 1].y, 0.f);
                glEnd();

                // draw finger flesh
                glm::mat4 mat1 = glm::rotate(glm::mat4(1.f),
                                             glm::radians(-angle / 2.f),
                                             glm::vec3(joint[i][j].pivot_x,
                                                        joint[i][j].pivot_y,
                                                       0.f));
                glm::vec4 u1 = mat1 * glm::vec4(-thickness * joint[i][j].pivot_x,
                                                -thickness * joint[i][j].pivot_y,
                                                thickness,
                                                1.f);
                glm::vec4 u2 = mat1 * glm::vec4(thickness * joint[i][j].pivot_x,
                                                thickness * joint[i][j].pivot_y,
                                                thickness,
                                                1.f);
                glm::vec4 u3 = mat1 * glm::vec4(thickness * joint[i][j].pivot_x,
                                                thickness * joint[i][j].pivot_y,
                                                -thickness,
                                                1.f);
                glm::vec4 u4 = mat1 * glm::vec4(-thickness * joint[i][j].pivot_x,
                                                -thickness * joint[i][j].pivot_y,
                                                -thickness,
                                                1.f);
                glm::mat4 mat2 = glm::translate(glm::mat4(1.f),
                                                glm::vec3(joint[i][j + 1].x,
                                                          joint[i][j + 1].y,
                                                          0.f));
                float next_angle = enable[i] ? 0.0f : joint[i][j + 1].fold_angle;
                mat2 = glm::rotate(mat2, glm::radians(next_angle / 2.f),
                                   glm::vec3(joint[i][j + 1].pivot_x,
                                                joint[i][j + 1].pivot_y,
                                             0.f));
                glm::vec4 v1 = mat2 * glm::vec4(-thickness * joint[i][j + 1].pivot_x,
                                                -thickness * joint[i][j + 1].pivot_y,
                                                thickness,
                                                1.f);
                glm::vec4 v2 = mat2 * glm::vec4(thickness * joint[i][j + 1].pivot_x,
                                                thickness * joint[i][j + 1].pivot_y,
                                                thickness,
                                                1.f);
                glm::vec4 v3 = mat2 * glm::vec4(thickness * joint[i][j + 1].pivot_x,
                                                thickness * joint[i][j + 1].pivot_y,
                                                -thickness,
                                                1.f);
                glm::vec4 v4 = mat2 * glm::vec4(-thickness * joint[i][j + 1].pivot_x,
                                                -thickness * joint[i][j + 1].pivot_y,
                                                -thickness,
                                                1.f);
                draw_hexahedron(u1, u2, u3, u4, v1, v2, v3, v4);
                draw_line(v1, v2);
                draw_line(v2, v3);
                draw_line(v3, v4);
                draw_line(v4, v1);
                if (j > 0) {
                    draw_line(u1, v1);
                    draw_line(u2, v2);
                    draw_line(u3, v3);
                    draw_line(u4, v4);
                }
                else if (i == 0) {
                    draw_line(u1, v1);
                    draw_line(u4, v4);
                }
                else if (i == 4) {
                    draw_line(u2, v2);
                    draw_line(u3, v3);
                }
        }
        for (int j = 0; i ? j < 4 : j < 3; j++)
            glPopMatrix();
    }

    // draw palm flesh
    for (int i = 0; i < 4; i++) { // i ~ i + 1
        GLfloat angle1 = enable[i] ? 0.0f : joint[i][0].fold_angle;
        glm::mat4 mat1 = glm::translate(glm::mat4(1.f),
                                        glm::vec3(joint[i][0].x,
                                                  joint[i][0].y,
                                                  0.f));
        glm::mat4 mat2 = glm::rotate(mat1, glm::radians(angle1),
                                     glm::vec3(joint[i][0].pivot_x,
                                                joint[i][0].pivot_y,
                                               0.f));
        mat1 = glm::rotate(mat1, glm::radians(angle1 / 2.f),
                           glm::vec3(joint[i][0].pivot_x,
                                     joint[i][0].pivot_y,
                                     0.f));
        mat2 = glm::translate(mat2, glm::vec3(joint[i][1].x, joint[i][1].y, 0.f));
        GLfloat angle2 = enable[i] ? 0.0f : joint[i][1].fold_angle;
        mat2 = glm::rotate(mat2, glm::radians(angle2 / 2.f),
                           glm::vec3(joint[i][1].pivot_x,
                                        joint[i][1].pivot_y,
                                     0.f));
        glm::vec4 u1 = mat1 * glm::vec4(thickness * joint[i][0].pivot_x,
                                        thickness * joint[i][0].pivot_y,
                                        thickness,
                                        1.f);
        glm::vec4 u4 = mat1 * glm::vec4(thickness * joint[i][0].pivot_x,
                                        thickness * joint[i][0].pivot_y,
                                        -thickness,
                                        1.f);
        glm::vec4 v1 = mat2 * glm::vec4(thickness * joint[i][1].pivot_x,
                                        thickness * joint[i][1].pivot_y,
                                        thickness,
                                        1.f);
        glm::vec4 v4 = mat2 * glm::vec4(thickness * joint[i][1].pivot_x,
                                        thickness * joint[i][1].pivot_y,
                                        -thickness,
                                        1.f);
        angle1 = enable[i + 1] ? 0.0f : joint[i + 1][0].fold_angle;
        mat1 = glm::translate(glm::mat4(1.f),
                              glm::vec3(joint[i + 1][0].x,
                              joint[i + 1][0].y,
                              0.f));
        mat2 = glm::rotate(mat1, glm::radians(angle1),
                           glm::vec3(joint[i + 1][0].pivot_x,
                           joint[i + 1][0].pivot_y,
                           0.f));
        mat1 = glm::rotate(mat1, glm::radians(angle1 / 2.f),
                           glm::vec3(joint[i + 1][0].pivot_x,
                                     joint[i + 1][0].pivot_y,
                                     0.f));
        mat2 = glm::translate(mat2, glm::vec3(joint[i + 1][1].x,
                                              joint[i + 1][1].y,
                                              0.f));
        angle2 = enable[i + 1] ? 0.0f : joint[i + 1][1].fold_angle;
        mat2 = glm::rotate(mat2, glm::radians(angle2 / 2.f),
                           glm::vec3(joint[i + 1][1].pivot_x,
                                        joint[i + 1][1].pivot_y,
                                     0.f));
        glm::vec4 u2 = mat1 * glm::vec4(-thickness * joint[i + 1][0].pivot_x,
                                        -thickness * joint[i + 1][0].pivot_y,
                                        thickness,
                                        1.f);
        glm::vec4 u3 = mat1 * glm::vec4(-thickness * joint[i + 1][0].pivot_x,
                                        -thickness * joint[i + 1][0].pivot_y,
                                        -thickness,
                                        1.f);
        glm::vec4 v2 = mat2 * glm::vec4(-thickness * joint[i + 1][1].pivot_x,
                                        -thickness * joint[i + 1][1].pivot_y,
                                        thickness,
                                        1.f);
        glm::vec4 v3 = mat2 * glm::vec4(-thickness * joint[i + 1][1].pivot_x,
                                        -thickness * joint[i + 1][1].pivot_y,
                                        -thickness,
                                        1.f);
        draw_hexahedron(u1, u2, u3, u4, v1, v2, v3, v4);
        draw_line(v1, v2);
        draw_line(v2, v3);
        draw_line(v3, v4);
        draw_line(v4, v1);
        if (i == 0) {
            draw_line(u1, v1);
            draw_line(u2, v2);
            draw_line(u3, v3);
            draw_line(u4, v4);
        }
    }
}

void drawSomething()
{
    // Draw something Here
    // rock
    glPushMatrix();
    glRotatef(-90.f, 0.f, 0.f, 1.f);
    glRotatef(180.f, 1.f, 0.f, 0.f);
    glTranslatef(0.f, -dist, 0.f);
    bool rock[5] = {false, false, false, false, false};
    draw_hand(rock);
    glPopMatrix();

    // paper
    glPushMatrix();
    glRotatef(-90.f, 0.f, 0.f, 1.f);
    glRotatef(-60.f, 1.f, 0.f, 0.f);
    glTranslatef(0.f, -dist, 0.f);
    bool paper[5] = {true, true, true, true, true};
    draw_hand(paper);
    glPopMatrix();

    // scissors
    glPushMatrix();
    glRotatef(-90.f, 0.f, 0.f, 1.f);
    glRotatef(60.f, 1.f, 0.f, 0.f);
    glTranslatef(0.f, -dist, 0.f);
    bool scissors[5] = {false, true, true, false, false};
    draw_hand(scissors);
    glPopMatrix();
}

void reshape(int w, int h)
{
    width = glutGet(GLUT_WINDOW_WIDTH);
    height = glutGet(GLUT_WINDOW_HEIGHT);

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = (float)w / (float)h;
    gluPerspective(fovy /*field of view angle*/,
                   aspectRatio,
                   0.1 /*near clipping plane*/,
                   1000.0 /* far clipping plane */);

    float eyeX = originX + distance * orientX;
    float eyeY = originY + distance * orientY;
    float eyeZ = originZ + distance * orientZ;
    
    gluLookAt(eyeX, eyeY, eyeZ, originX, originY, originZ, upX, upY, upZ);
    glMatrixMode(GL_MODELVIEW);
}

void display()
{
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawSomething();
    glFlush();
    glutSwapBuffers();
}

void keyboardCB(unsigned char keyPressed, int x, int y)
{
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow(width, height);
            fullScreen = false;
        } else {
            glutFullScreen();
            fullScreen = true;
        }
        break;
    case 'q':
        exit(0);
        break;
    case 'a': // show all
        originX = 0.f;
        originY = 0.f;
        originZ = 0.f;
        distance = 3.f;
        fovy = 45.f;

        reshape(width, height);
        break;
    }
    glutPostRedisplay();
}

void mouseCB(int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed   = false;
        mouseRotatePressed = false;
        mouseZoomPressed   = false;
        mouseDollyPressed  = false;
    } else {
        if (button == GLUT_LEFT_BUTTON && GLUT_ACTIVE_SHIFT == glutGetModifiers())
        {
            // do something here
            lastX = x;
            lastY = y;

            //std::cout << "translate click" << std::endl;

            mouseMovePressed   = true;
            mouseRotatePressed = false;
            mouseZoomPressed   = false;
            mouseDollyPressed  = false;
        }
        else if (button == GLUT_RIGHT_BUTTON && GLUT_ACTIVE_SHIFT == glutGetModifiers())
        {
            // do something here
            glReadBuffer(GL_FRONT);
            float z;
            glReadPixels(x, height - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
            
            if (z < 1.0) {
                float eyeX = originX + distance * orientX;
                float eyeY = originY + distance * orientY;
                float eyeZ = originZ + distance * orientZ;
    
                glm::vec3 win = glm::vec3(x, height - y, z);
                glm::mat4 view = glm::lookAt(glm::vec3(eyeX, eyeY, eyeZ),
                                             glm::vec3(originX, originY, originZ),
                                             glm::vec3(upX, upY, upZ));
                glm::mat4 proj = glm::perspective(glm::radians(fovy), (float)width / (float)height,
                                                  0.1f, 1000.f);
                glm::vec4 viewport = glm::vec4(0, 0, width, height);

                glm::vec3 coord = glm::unProject(win, view, proj, viewport);
                originX = coord.x;
                originY = coord.y;
                originZ = coord.z;

                reshape(width, height);
            }

            // std::cout << "seek click" << std::endl;
        }
        else if (button == GLUT_LEFT_BUTTON && GLUT_ACTIVE_CTRL == glutGetModifiers())
        {
            // do something here
            lastY = y;

            //std::cout << "zoom click" << std::endl;

            mouseMovePressed   = false;
            mouseRotatePressed = false;
            mouseZoomPressed   = true;
            mouseDollyPressed  = false;
        }
        else if (button == GLUT_RIGHT_BUTTON && GLUT_ACTIVE_CTRL == glutGetModifiers())
        {
            // do something here
            lastY = y;

            //std::cout << "dolly click" << std::endl;

            mouseMovePressed   = false;
            mouseRotatePressed = false;
            mouseZoomPressed   = false;
            mouseDollyPressed  = true;
        }
        else if (button == GLUT_LEFT_BUTTON)
        {
            // do something here
            lastX = x;
            lastY = y;

            //std::cout << "rotate click" << std::endl;

            mouseMovePressed   = false;
            mouseRotatePressed = true;
            mouseZoomPressed   = false;
            mouseDollyPressed  = false;
        }
    }
    glutPostRedisplay();
}

void motionCB(int x, int y) {
    if (mouseRotatePressed == true)
    {
        // do something here
        float radius = std::min(width, height) / 2.0;
        float centerX = width / 2.0;
        float centerY = height / 2.0;
        float deltaX = x - centerX;
        float deltaY = centerY - y;
        float deltaLastX = lastX - centerX;
        float deltaLastY = centerY - lastY;
        float dist1Square = deltaX * deltaX + deltaY * deltaY;
        float dist2Square = deltaLastX * deltaLastX + deltaLastY * deltaLastY;
        int distSquare = (x - lastX) * (x - lastX) + (y - lastY) * (y - lastY);
        if (dist1Square <= radius * radius && dist2Square <= radius * radius && distSquare) {
            glm::vec3 axisX = glm::cross(glm::vec3(upX, upY, upZ),
                                         glm::vec3(orientX, orientY, orientZ));
            axisX = glm::normalize(axisX);
            glm::vec3 axisY = glm::cross(glm::vec3(orientX, orientY, orientZ), axisX);
            axisY = glm::normalize(axisY);

            float deltaZ = sqrt(radius * radius - dist1Square);
            float deltaLastZ = sqrt(radius * radius - dist2Square);

            float sphereX = deltaX * axisX.x + deltaY * axisY.x + deltaZ * orientX;
            float sphereY = deltaX * axisX.y + deltaY * axisY.y + deltaZ * orientY;
            float sphereZ = deltaX * axisX.z + deltaY * axisY.z + deltaZ * orientZ;
            float sphereLastX = deltaLastX * axisX.x + deltaLastY * axisY.x + deltaLastZ * orientX;
            float sphereLastY = deltaLastX * axisX.y + deltaLastY * axisY.y + deltaLastZ * orientY;
            float sphereLastZ = deltaLastX * axisX.z + deltaLastY * axisY.z + deltaLastZ * orientZ;

            glm::vec3 axis = glm::cross(glm::vec3(sphereX, sphereY, sphereZ),
                                        glm::vec3(sphereLastX, sphereLastY, sphereLastZ));
            float theta = angularSpeed * sqrt(distSquare);
            
            glm::quat rotateMatrix = glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), theta, axis);
            glm::vec3 orient = rotateMatrix * glm::vec3(orientX, orientY, orientZ);
            orient = glm::normalize(orient);
            orientX = orient.x;
            orientY = orient.y;
            orientZ = orient.z;

            glm::vec3 up = rotateMatrix * glm::vec3(upX, upY, upZ);
            up = glm::normalize(up);
            upX = up.x;
            upY = up.y;
            upZ = up.z;

            reshape(width, height);
        }

        lastX = x;
        lastY = y;

        //std::cout << "rotate drag" << std::endl;
    }
    else if (mouseMovePressed == true)
    {
        // do something here
        glm::vec3 axisX = glm::cross(glm::vec3(upX, upY, upZ), glm::vec3(orientX, orientY, orientZ));
        axisX = glm::normalize(axisX);
        glm::vec3 axisY = glm::cross(glm::vec3(orientX, orientY, orientZ), axisX);
        axisY = glm::normalize(axisY);

        float distX = (lastX - x) * 0.001;
        float distY = (y - lastY) * 0.001;

        originX += distX * axisX.x + distY * axisY.x;
        originY += distX * axisX.y + distY * axisY.y;
        originZ += distX * axisX.z + distY * axisY.z;
        
        reshape(width, height);

        lastX = x;
        lastY = y;

        //std::cout << "translate drag" << std::endl;
    }
    else if (mouseZoomPressed == true)
    {
        // do something here
        fovy += (y - lastY) * 0.1;
        if (fovy < 0.1) fovy = 0.1;
        else if (fovy > 179) fovy = 179;

        reshape(width, height);

        lastY = y;

        //std::cout << "zoom drag" << std::endl;
    }
    else if (mouseDollyPressed == true)
    {
        // do something here
        distance += (y - lastY) * 0.01;
        if (distance < 0.1) distance = 0.1;

        reshape(width, height);

        lastY = y;

        //std::cout << "dolly drag" << std::endl;
    }
    glutPostRedisplay();
}

void idle() { glutPostRedisplay(); }

int main(int argc, char** argv) {
    init();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow("Rock-Paper-Scissors 3D viewer");

    reshape(width, height);
    glClearColor(0.0, 0.1, 0.3, 1.0);

    glClearDepth(1.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardCB);
    glutReshapeFunc(reshape);
    glutMotionFunc(motionCB);
    glutMouseFunc(mouseCB);

    glutMainLoop();
    return 0;
}
