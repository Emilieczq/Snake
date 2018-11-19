#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#endif

#include <math.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

const int SIZE_CELL = 10; // size of one cell of map is 10 * 10
const int SIZE_MAP = 20;  // size of map is 20 cells * 20 cells

std::vector<int> map; // index of vecter is index of a cell, int is type of this cell (1: grass; 2: pond; 3: stone)
std::vector<int> indicesStone;
std::vector<int> indicesPond;
int indexFruit;

std::vector<int> snake;

bool isStop = true; // at defaut, snake stops

int currentDirection = 1; // 1: right; 2: down; 3: left; 4:up
int currentLevel = 1;

float camPos[] = {100, -100, 300}; // camera's position

bool findIndex(std::vector<int> v, int x)
{
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
        numStones = 1;
        numPonds = 1;
    }
    else if (level == 2) // medium
    {
        numStones = 5;
        numPonds = 5;
    }
    else // hard
    {
        numStones = 10;
        numPonds = 10;
    }

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
        while (true)
        {
            if (r == indexFruit || r == 0 || r == 1)
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
        // do this loop to avoid the fruit is created in a pond or under snake
        // and make sure a stone is not created in a pond
        while (true)
        {
            if (r == indexFruit || r == 0 || r == 1 || findIndex(indicesPond, r))
                r = rand() % (SIZE_MAP * SIZE_MAP);
            else
                break;
        }
        map.at(r) = 3;
        indicesStone.push_back(r);
    }
}
/**
 * Set the fruit's index a random number
 */
void newFruit(void)
{
    srand(time(NULL));
    int r = rand() % (SIZE_MAP * SIZE_MAP);
    // make sure that the fruit is not in pond, in stone or in snake
    while (true)
    {
        if (findIndex(indicesPond, r) || findIndex(indicesStone, r) || findIndex(snake, r))
            r = rand() % (SIZE_MAP * SIZE_MAP);
        else
            break;
    }
    indexFruit = r;
}

void resetAll(void)
{
    indicesPond.clear();
    indicesStone.clear();
    snake.clear();
    snake.push_back(1);
    snake.push_back(0);
    map.clear();
    isStop = true;
    currentDirection = 1;
    newFruit();
    createMap(currentLevel);
}

void drawMap(void)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    for (int i = 0; i < SIZE_MAP * SIZE_MAP; i++)
    {
        int type = map.at(i);
        if (type == 1)
        {
            glColor3ub(0, 102, 0); // grass: green
        }
        else if (type == 2)
        {
            glColor3ub(0, 204, 255); // pond: blue
        }
        else
        {
            glColor3ub(153, 153, 153); // stone: gray
        }
        int x = (i % SIZE_MAP) * SIZE_CELL;
        int y = (i / SIZE_MAP) * SIZE_CELL;
        int z = 0;

        glVertex3i(x, y, z);
        glVertex3i(x, y + SIZE_CELL, z);
        glVertex3i(x + SIZE_CELL, y + SIZE_CELL, z);
        glVertex3i(x + SIZE_CELL, y, z);
    }
    glEnd();
}

/**
 * The idea of moving is to find the next position of the head and it becomes the head
 * then drop the last one
 * but if the next position has a fruit, don't drop the last block
 * 
 * check if the next postion has a stone or a pond
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
        snake.insert(snake.begin(), indexNext);
        newFruit();
    }
    // if next position has a pond
    else if (findIndex(indicesPond, indexNext))
    {
        // TO DO (For now, it just functions like normal)
        #ifdef __APPLE__
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // control speed
        #else
            Sleep(200);
        #endif
        snake.insert(snake.begin(), indexNext);
        snake.pop_back();
    }
    // if next position has a stone or its body
    else if (findIndex(indicesStone, indexNext) || findIndex(snake, indexNext))
    {
        resetAll();
    }
    // if next position is on grass
    else
    {
        #ifdef __APPLE__
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // control speed
        #else
            Sleep(200);
        #endif
        snake.insert(snake.begin(), indexNext);
        snake.pop_back();
    }
}

void drawSnake(void)
{
    glMatrixMode(GL_MODELVIEW);

    glColor3f(1, 1, 1);

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
    glutPostRedisplay();
}

void drawFruit(void)
{
    glMatrixMode(GL_MODELVIEW);

    glColor3f(1, 1, 0);
    int x = SIZE_CELL / 2 + indexFruit % SIZE_MAP * SIZE_CELL;
    int y = SIZE_CELL / 2 + indexFruit / SIZE_MAP * SIZE_CELL;
    glPushMatrix();
    glTranslatef(x, y, SIZE_CELL / 2);
    glutSolidSphere(SIZE_CELL / 2, 50, 50);
    glPopMatrix();

    glutPostRedisplay();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camPos[0], camPos[1], camPos[2], SIZE_CELL * SIZE_MAP / 2, SIZE_CELL * SIZE_MAP / 2, 0, 0, 1, 0);

    glColor3f(1.0, 1.0, 1.0);
    drawMap();
    drawFruit();
    drawSnake();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 27:
        exit(0);
        break;
    case 'd':
        if ((currentDirection == 2 || currentDirection == 4) && !isStop)
            currentDirection = 1;
        break;
    case 's':
        if ((currentDirection == 1 || currentDirection == 3) && !isStop)
            currentDirection = 2;
        break;
    case 'a':
        if ((currentDirection == 2 || currentDirection == 4) && !isStop)
            currentDirection = 3;
        break;
    case 'w':
        if ((currentDirection == 1 || currentDirection == 3) && !isStop)
            currentDirection = 4;
        break;
    case ' ':
        isStop = !isStop;
        break;
    }
}

void init(void)
{
    glClearColor(0, 0, 0, 0);
    glColor3f(1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45, 1, 1, 1000);

    newFruit();
    createMap(currentLevel); // default level is easy(1)

    snake.push_back(1);
    snake.push_back(0);
}

void callbackInit()
{
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
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
    glutMainLoop();
    return (0);
}
