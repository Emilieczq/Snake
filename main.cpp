#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

#include <cstdlib>
#include <unistd.h>

#include <math.h>
#include <string>
#include <time.h>
#include <vector>
#include <algorithm>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>

const int SIZE_CELL = 10; // size of one cell of map is 10 * 10
const int SIZE_MAP = 20;  // size of map is 20 cells * 20 cells

std::vector<int> map; // index of vecter is index of a cell, int is type of this cell (1: grass; 2: pond; 3: stone)
std::vector<int> indicesStone;
std::vector<int> indicesPond;
int indexFruit = 0;
int indexCoin = 0;

std::vector<int> snake;

bool isStop = true; // at defaut, snake stops
bool lose = false;
int currentDirection = 1; // 1: right; 2: down; 3: left; 4:up
int currentLevel = 1;     // obstacles 1: X5; 2: X10; 3: X15
int moveDelayTime = 200;
int score = 0;
int highscore = 0; // keep track of score
int currency = 0;  // keep track of currency when coins are picked up

/* Textures */
GLubyte *grass_tex, *pond_tex, *stone_tex, *coin_tex;
int width, height, max;
GLuint textures[4]; // change this number to add more textures

/* Lighting */
bool lightOn = true;                      // default light is on
float light_pos[] = {150, -500, 150, 1}; //light's position
float lightSpeed = 100.0f;

/* Eye */
float eyeRotate[] = {0, 0};
float eye[] = {100, -100, 300}; // eye's position
float eyeSpeed = 2.0f;


/**
 * Check if an int x is in a vector v
 * 
 * must sort the vector at first
 */
bool findIndex(std::vector<int> v, int x)
{
    std::sort(v.begin(), v.end());
    if (std::binary_search(v.begin(), v.end(), x))
        return true;
    else
        return false;
}

/** 
 * Create a map with random ponds and random stones
 * 
 * level => 1: easy; 2: medium; 3: hard
 */
void createMap(int level)
{
    int numStones, numPonds;
    if (level == 1) // easy
    {
        numStones = 5;
        numPonds = 5;
    }
    else if (level == 2) // medium
    {
        numStones = 10;
        numPonds = 10;
    }
    else // hard
    {
        numStones = 15;
        numPonds = 15;
    }

    /* Initial all cases of the map to grass */
    for (int i = 0; i < SIZE_MAP * SIZE_MAP; i++)
    {
        map.push_back(1);
    }

    /* Generate a map with random stones and ponds */
    srand(time(NULL));
    for (int i = 0; i < numPonds; i++)
    {
        int r = rand() % (SIZE_MAP * SIZE_MAP);
        // do this loop to avoid the fruit is created in a pond or under snake
        // or duplicate pond at the same place
        while (true)
        {
            if (r == indexFruit || r == 0 || r == 1 || findIndex(indicesPond, r) || r == indexCoin)
                r = rand() % (SIZE_MAP * SIZE_MAP);
            else
                break;
        }
        map.at(r) = 2;
        indicesPond.push_back(r);
    }
    for (int i = 0; i < numStones; i++)
    {
        int r = rand() % (SIZE_MAP * SIZE_MAP);
        // do this loop to avoid the fruit is created in a stone or under snake
        // or duplicate stone at the same place
        // and make sure a stone is not created in a pond
        while (true)
        {
            if (r == indexFruit || r == 0 || r == 1 || findIndex(indicesPond, r) || findIndex(indicesStone, r))
                r = rand() % (SIZE_MAP * SIZE_MAP);
            else
                break;
        }
        map.at(r) = 3;
        indicesStone.push_back(r);
    }
}

// Set the fruit's index a random number
void newFruit(void)
{
    srand(time(NULL));
    int r = rand() % (SIZE_MAP * SIZE_MAP);
    // make sure that the fruit is not in pond, in stone or in snake, or in coin
    while (true)
    {
        if (findIndex(indicesPond, r) || findIndex(indicesStone, r) || findIndex(snake, r) || indexCoin == r)
            r = rand() % (SIZE_MAP * SIZE_MAP);
        else
            break;
    }
    indexFruit = r;
}

// set coin's index a random number
void newCoin(void)
{
    srand(time(NULL));
    int r = rand() % (SIZE_MAP * SIZE_MAP);
    // make sure that the fruit is not in pond, in stone or in snake, or in fruit
    while (true)
    {
        if (findIndex(indicesPond, r) || findIndex(indicesStone, r) || findIndex(snake, r) || indexFruit == r)
            r = rand() % (SIZE_MAP * SIZE_MAP);
        else
            break;
    }
    indexCoin = r;
}

void newGame(void)
{
    score = 0;
    indicesPond.clear();
    indicesStone.clear();
    snake.clear();
    snake.push_back(1);
    snake.push_back(0);
    map.clear();
    isStop = true;
    currentDirection = 1;
    newFruit();
    newCoin();

    createMap(currentLevel);
}

void grass(int i)
{
    // glColor3ub(0, 102, 0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    int x = (i % SIZE_MAP) * SIZE_CELL;
    int y = (i / SIZE_MAP) * SIZE_CELL;
    int z = 0;
    glTexCoord2f(0, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y, z);
    glTexCoord2f(0, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y + SIZE_CELL, z);
    glTexCoord2f(1, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, z);
    glTexCoord2f(1, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y, z);
    glEnd();
}

void pond(int i)
{
    // glColor3ub(0, 204, 255);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    int x = (i % SIZE_MAP) * SIZE_CELL;
    int y = (i / SIZE_MAP) * SIZE_CELL;
    int z = 0;
    glTexCoord2f(0, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y, z);
    glTexCoord2f(0, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y + SIZE_CELL, z);
    glTexCoord2f(1, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, z);
    glTexCoord2f(1, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y, z);
    glEnd();
}

// Now we use cube to represent a stone
void stone(int i)
{
    // glColor3ub(153, 153, 153);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glBegin(GL_QUADS);
    int x = (i % SIZE_MAP) * SIZE_CELL;
    int y = (i / SIZE_MAP) * SIZE_CELL;
    // top
    glTexCoord2f(0, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y, SIZE_CELL);
    glTexCoord2f(0, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 1);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 0);
    glNormal3f(0, 0, 1);
    glVertex3i(x + SIZE_CELL, y, SIZE_CELL);
    // front
    glTexCoord2f(0, 0);
    glNormal3f(0, -1, 0);
    glVertex3i(x, y, 0);
    glTexCoord2f(0, 1);
    glNormal3f(0, -1, 0);
    glVertex3i(x, y, SIZE_CELL);
    glTexCoord2f(1, 1);
    glNormal3f(0, -1, 0);
    glVertex3i(x + SIZE_CELL, y, SIZE_CELL);
    glTexCoord2f(1, 0);
    glNormal3f(0, -1, 0);
    glVertex3i(x + SIZE_CELL, y, 0);
    // left
    glTexCoord2f(0, 0);
    glNormal3f(-1, 0, 0);
    glVertex3i(x, y, 0);
    glTexCoord2f(0, 1);
    glNormal3f(-1, 0, 0);
    glVertex3i(x, y + SIZE_CELL, 0);
    glTexCoord2f(1, 1);
    glNormal3f(-1, 0, 0);
    glVertex3i(x, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 0);
    glNormal3f(-1, 0, 0);
    glVertex3i(x, y, SIZE_CELL);
    // right
    glTexCoord2f(0, 0);
    glNormal3f(1, 0, 0);
    glVertex3i(x + SIZE_CELL, y, 0);
    glTexCoord2f(0, 1);
    glNormal3f(1, 0, 0);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, 0);
    glTexCoord2f(1, 1);
    glNormal3f(1, 0, 0);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 0);
    glNormal3f(1, 0, 0);
    glVertex3i(x + SIZE_CELL, y, SIZE_CELL);
    // back
    glTexCoord2f(0, 0);
    glNormal3f(0, 1, 0);
    glVertex3i(x, y + SIZE_CELL, 0);
    glTexCoord2f(0, 1);
    glNormal3f(0, 1, 0);
    glVertex3i(x, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 1);
    glNormal3f(0, 1, 0);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, SIZE_CELL);
    glTexCoord2f(1, 0);
    glNormal3f(0, 1, 0);
    glVertex3i(x + SIZE_CELL, y + SIZE_CELL, 0);
    // don't need to draw bottom as it's hidden
    glEnd();
}

/**
 * Draw the map
 */
void drawMap(void)
{
    for (int i = 0; i < SIZE_MAP * SIZE_MAP; i++)
    {
        int type = map.at(i);
        if (type == 1) // grass
        {
            grass(i);
        }
        else if (type == 2) // pond
        {
            pond(i);
        }
        else // stone
        {
            stone(i);
        }
    }
}

/**
 * The idea of moving is to find the next position of the head and it becomes the head
 * then drop the last one
 * but if the next position has a fruit, don't drop the last block
 * 
 * check if the next postion has a stone or a pond or itself
 */
void move(int direction)
{
    int indexHead = snake.at(0);
    int xHead = indexHead % SIZE_MAP;
    int yHead = indexHead / SIZE_MAP;
    int xNext, yNext, indexNext;
    if (direction == 1) // right
    {
        xNext = xHead == SIZE_MAP - 1 ? 0 : xHead + 1;
        yNext = yHead;
    }
    else if (direction == 2) // down
    {
        xNext = xHead;
        yNext = yHead == 0 ? SIZE_MAP - 1 : yHead - 1;
    }
    else if (direction == 3) // left
    {
        xNext = xHead == 0 ? SIZE_MAP - 1 : xHead - 1;
        yNext = yHead;
    }
    else // up
    {
        xNext = xHead;
        yNext = yHead == SIZE_MAP - 1 ? 0 : yHead + 1;
    }
    indexNext = yNext * SIZE_MAP + xNext;

    // if next index equals to the index of fruit, don't drop the last block so snake becomes longer
    if (indexNext == indexFruit)
    {
        score++;
        printf("Score: %d\n", score);
        snake.insert(snake.begin(), indexNext);
        newFruit();
    }

    else if (indexNext == indexCoin)
    {
        currency++;
        //printf("\nCoin picked up\n");
        printf("    Wallet: $%d\n", currency);
        newCoin();
    }
    // if next position has a pond
    else if (findIndex(indicesPond, indexNext))
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1, 1, 1, 0.3);
// TO DO (For now, it just functions like normal)
#ifdef __APPLE__
        std::this_thread::sleep_for(std::chrono::milliseconds(moveDelayTime)); // control speed
#else
        Sleep(moveDelayTime);
#endif
        snake.insert(snake.begin(), indexNext);
        snake.pop_back();
    }
    // if next position has a stone or its body, the snake dies and reset the map and the snake
    else if (findIndex(indicesStone, indexNext) || findIndex(snake, indexNext))
    {
        lose = true;
        // keep wallet at the same amount? that way the user can continue to collect coins
        // and spend them in future games.
        //currency = 0;
        //newGame();
    }
    // if next position is on grass
    else
    {
#ifdef __APPLE__
        std::this_thread::sleep_for(std::chrono::milliseconds(moveDelayTime)); // control speed
#else
        Sleep(moveDelayTime);
#endif
        snake.insert(snake.begin(), indexNext);
        snake.pop_back();
    }
}

void drawSnake(void)
{
    glMatrixMode(GL_MODELVIEW);

    glColor4f(1, 1, 1, 1);

    if (!isStop)
        move(currentDirection);

    for (int i = 0; i < snake.size(); i++)
    {
        int index = snake.at(i);
        int x = SIZE_CELL / 2 + index % SIZE_MAP * SIZE_CELL;
        int y = SIZE_CELL / 2 + index / SIZE_MAP * SIZE_CELL;
        glPushMatrix();
        glTranslatef(x, y, SIZE_CELL / 2);
        glutSolidSphere(SIZE_CELL / 2, 50, 50);
        glPopMatrix();
    }
    glDisable(GL_BLEND);
    glutPostRedisplay();
}

void drawFruit(void)
{
    glMatrixMode(GL_MODELVIEW);

    // made fruit red
    glColor3f(1, 0, 0);
    int x = SIZE_CELL / 2 + indexFruit % SIZE_MAP * SIZE_CELL;
    int y = SIZE_CELL / 2 + indexFruit / SIZE_MAP * SIZE_CELL;
    glPushMatrix();
    glTranslatef(x, y, SIZE_CELL / 2);
    glutSolidSphere(SIZE_CELL / 2, 50, 50);
    glPopMatrix();

    glutPostRedisplay();
}

float angle = 0;
void drawCoin(void)
{
    int x = SIZE_CELL / 2 + indexCoin % SIZE_MAP * SIZE_CELL;
    int y = SIZE_CELL / 2 + indexCoin / SIZE_MAP * SIZE_CELL;

    glPushMatrix();
    glTranslatef(x, y, SIZE_CELL / 2);
    glRotatef(angle, 0, 1, 1);

    // the rotation is affected when the game is paused or not.
    // thus, changed the speeds of the game to ensure smoother rotation of the objects.
    if (isStop)
    {
        angle += 0.1;
    }
    else
    {
        angle += 40;
    }
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    GLUquadricObj *sphere = NULL;
    sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_FILL);
    gluQuadricTexture(sphere, true);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    //glBindTexture(GL_TEXTURE_2D, textures[3]);
    gluSphere(sphere, SIZE_CELL / 2, 50, 50);
    glPopMatrix();

    glutPostRedisplay();
}

void display(void)
{
    //set material
    float amb[] = {1.0, 0.5, 0.3, 0.3};
    float dif[] = {1.0, 0.5, 0.3, 1.0};
    float spec[] = {0.5, 0.5, 0.5, 1.0};
    float shiny = 50;

    //enable material
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
    glColorMaterial(GL_DIFFUSE, GL_AMBIENT);

    // title
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eye[0], eye[1], eye[2], SIZE_CELL * SIZE_MAP / 2, SIZE_CELL * SIZE_MAP / 2, 0, 0, 1, 0);

    if (!lose)
    {
        glPushMatrix();
        //Scene rotation
        glTranslatef(SIZE_CELL * SIZE_MAP / 2, SIZE_CELL * SIZE_MAP / 2, 0);
        glRotatef(eyeRotate[0], 1, 0, 0);
        glTranslatef(-SIZE_CELL * SIZE_MAP / 2, -SIZE_CELL * SIZE_MAP / 2, 0);

        glTranslatef(SIZE_CELL * SIZE_MAP / 2, SIZE_CELL * SIZE_MAP / 2, 0);
        glRotatef(eyeRotate[1], 0, 1, 0);
        glTranslatef(-SIZE_CELL * SIZE_MAP / 2, -SIZE_CELL * SIZE_MAP / 2, 0);

        // Enable lighting if light's on, light0 is enabled in init so no need to enable again
        if (lightOn)
            glEnable(GL_LIGHTING);
        else
            glDisable(GL_LIGHTING);

        /* display all models here */
        drawMap();
        drawCoin();

        glBindTexture(GL_TEXTURE_2D, 0); // disable texture binding for the following functions (no texture for fruit and snake now, can be added in the future)
        drawFruit();
        drawSnake();
        glPopMatrix();

        /* display all words here */
        glDisable(GL_LIGHTING); // disable lighting on words
        // instruction
        glRasterPos2i(-20, 310);
        std::string instrucString = "w, a, s, d  : Snake Direction Control  u, j, h, k : Light Control";
        for (std::string::iterator i = instrucString.begin(); i != instrucString.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
        glRasterPos2i(-20, 290);
        std::string instrucString2 = " arrow key : Scene Rotation   l : Turn on/off Light  q : Quit";
        for (std::string::iterator i = instrucString2.begin(); i != instrucString2.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
        // title
        glRasterPos2i(60, 250);
        std::string title = "3D SNAKE GAME";
        for (std::string::iterator i = title.begin(); i != title.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
        // score
        std::string scoreString = std::to_string(score);
        glRasterPos2i(45, -13);
        std::string scoreDescription = "SCORE: " + scoreString;
        for (std::string::iterator i = scoreDescription.begin(); i != scoreDescription.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
        // MAX score
        if (highscore < score)
        {
            highscore = score;
        }
        std::string hiscoreString = std::to_string(highscore);
        glRasterPos2i(45, -23);
        std::string hiscoreDescription = "HIGH SCORE: " + hiscoreString;
        for (std::string::iterator i = hiscoreDescription.begin(); i != hiscoreDescription.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
        // wallet
        std::string currencyString = std::to_string(currency);
        glRasterPos2i(115, -13);
        std::string currencyDescription = "WALLET: $" + currencyString;
        for (std::string::iterator i = currencyDescription.begin(); i != currencyDescription.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }
    }
    else
    {
        glPushMatrix();
        glTranslatef(10, 80, 1);
        glScalef(0.08, 0.2, 1);
        std::string titleL = "You lost! \n  Press SPACE to restart ";
        for (std::string::iterator i = titleL.begin(); i != titleL.end(); ++i)
        {
            char c = *i;
            glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
        }
        glPopMatrix();

        /* glRasterPos2i(25, 50);
        std::string titleR = "Press SPACE to restart the game. ";
        for (std::string::iterator i = titleR.begin(); i != titleR.end(); ++i)
        {
            char c = *i;
            glutStrokeCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }*/

        
        glPushMatrix();
        std::string scoreString2 = std::to_string(highscore);
        glRasterPos2i(45, 40);
        std::string scoreDescription2 = "The highest score is: " + scoreString2;
        for (std::string::iterator i = scoreDescription2.begin(); i != scoreDescription2.end(); ++i)
        {
            char c = *i;
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
        glPopMatrix();
    }
    // reshape window (TODO)
    // glTranslatef(-1 * (SIZE_MAP / 2), 0, -1 * (SIZE_MAP / 2));

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{

    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
        break;
    case 'd':
    case 'D':
        if ((currentDirection == 2 || currentDirection == 4) && !isStop)
            currentDirection = 1;
        break;
    case 's':
    case 'S':
        if ((currentDirection == 1 || currentDirection == 3) && !isStop)
            currentDirection = 2;
        break;
    case 'a':
    case 'A':
        if ((currentDirection == 2 || currentDirection == 4) && !isStop)
            currentDirection = 3;
        break;
    case 'w':
    case 'W':
        if ((currentDirection == 1 || currentDirection == 3) && !isStop)
            currentDirection = 4;
        break;
    case ' ':
        if (lose)
        {
            lose = false;
            glutPostRedisplay();
            newGame();
        }
        else
        {
            isStop = !isStop;
        }
        break;
    case 'l':
    case 'L':
        lightOn = !lightOn;
        break;
    case 'j':
    case 'J':
        light_pos[1] -= lightSpeed;
        break;
    case 'u':
    case 'U':
        light_pos[1] += lightSpeed;
        break;
    case 'h':
    case 'H':
        light_pos[2] -= lightSpeed;
        break;
    case 'k':
    case 'K':
        light_pos[2] += lightSpeed;
        break;
    }
    //make sure only light is changed
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glPopMatrix();
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    // arrow key presses control scene
    switch (key)
    {
    case GLUT_KEY_UP:
        eyeRotate[1] += eyeSpeed;
        break;

    case GLUT_KEY_DOWN:
        eyeRotate[1] -= eyeSpeed;
        break;

    case GLUT_KEY_LEFT:
        eyeRotate[0] += eyeSpeed;
        break;

    case GLUT_KEY_RIGHT:
        eyeRotate[0] -= eyeSpeed;
        break;
    }
    glutPostRedisplay();
}

void processObstaclesMenu(int value)
{
    currentLevel = value;
    newGame();
}

void processSpeedMenu(int value)
{
    switch (value)
    {
    case 1:
        moveDelayTime = 200;
        break;
    case 2:
        moveDelayTime = 100;
        break;
    case 3:
        moveDelayTime = 75;
        break;
    }
}

void processMainMenu(int value)
{
    switch (value)
    {
    case 1:
        newGame();
        break;
    case 2:
        isStop = !isStop;
        break;
    case 3:
        exit(0);
        break;
    }
}

void createMenu(void)
{
    int numObstacles = glutCreateMenu(processObstaclesMenu);
    glutAddMenuEntry("X5", 1);
    glutAddMenuEntry("X10", 2);
    glutAddMenuEntry("X15", 3);

    int speed = glutCreateMenu(processSpeedMenu);
    glutAddMenuEntry("slow", 1);
    glutAddMenuEntry("medium", 2);
    glutAddMenuEntry("fast", 3);

    int main_id = glutCreateMenu(processMainMenu);
    glutAddMenuEntry("New Game", 1);
    glutAddSubMenu("Obstacles", numObstacles);
    glutAddSubMenu("Speed", speed);
    glutAddMenuEntry("Pause/Continue", 2);
    glutAddMenuEntry("Quit", 3);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/* LoadPPM -- loads the specified ppm file, and returns the image data as a GLubyte 
 *  (unsigned byte) array. Also returns the width and height of the image, and the
 *  maximum colour value by way of arguments
 */
GLubyte *LoadPPM(char *file, int *width, int *height, int *max)
{
    GLubyte *img;
    FILE *fd;
    int n, m;
    int k, nm;
    char c;
    int i;
    char b[100];
    float s;
    int red, green, blue;

    fd = fopen(file, "r");
    fscanf(fd, "%[^\n] ", b);
    if (b[0] != 'P' || b[1] != '3')
    {
        exit(0);
    }
    fscanf(fd, "%c", &c);
    while (c == '#')
    {
        fscanf(fd, "%[^\n] ", b);
        fscanf(fd, "%c", &c);
    }
    ungetc(c, fd);
    fscanf(fd, "%d %d %d", &n, &m, &k);
    nm = n * m;
    img = (GLubyte *)malloc(3 * sizeof(GLuint) * nm);
    s = 255.0 / k;
    for (i = 0; i < nm; i++)
    {
        fscanf(fd, "%d %d %d", &red, &green, &blue);
        img[3 * nm - 3 * i - 3] = red * s;
        img[3 * nm - 3 * i - 2] = green * s;
        img[3 * nm - 3 * i - 1] = blue * s;
    }
    *width = n;
    *height = m;
    *max = k;
    return img;
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, 1, 1, 1000);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    /* Texture */
    // enable texturing
    glEnable(GL_TEXTURE_2D);
    // generate 4 texture IDs, store them in array "textures"
    glGenTextures(4, textures);
    // load the texture (grass)
    grass_tex = LoadPPM("grass.ppm", &width, &height, &max);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, grass_tex);
    // load the texture (pond)
    pond_tex = LoadPPM("pond.ppm", &width, &height, &max);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pond_tex);
    // load the texture (stone)
    stone_tex = LoadPPM("stone.ppm", &width, &height, &max);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, stone_tex);
    // load coin texture
    coin_tex = LoadPPM("coin.ppm", &width, &height, &max);
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, coin_tex);

    /* New game */
    newGame();
}

void callbackInit()
{
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Snake");

    callbackInit();

    glEnable(GL_DEPTH_TEST);
    init();

    createMenu();

    glutMainLoop();
    return (0);
}
