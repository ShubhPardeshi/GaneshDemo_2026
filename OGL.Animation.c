// Header files
#include<Windows.h>
#include<stdlib.h> // for exit
#include<stdio.h> // For file IO
#include "OGL.h"

//opengl related header files
#include<GL/GL.h>
#include<GL/GLU.h>

#define NUMBER_OF_POINTS 100

//link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")
void road(void);
void tree(void);
void networkPoints(int);
void connectNetwork(int);


float anglePyramid =  0.0f;
float angleCube = 0.0f;

struct network
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct network points[NUMBER_OF_POINTS];
int numberOfPointsShow = NUMBER_OF_POINTS;
int nextPointVal;


//globale variab;le declaration
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL bFullScreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
FILE *gpFile = NULL;

BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;

//Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global fuction delcarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//entry point function

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE  hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    //func declarations
    int initialize(void);
    void unintialize(void);
    void render(void);
    void update(void);
    
    //create log file 
    gpFile = fopen("Log.txt", "w");
    
    if(gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log file creation failed"), TEXT("ERROR"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program started successfully\n");
    }

    //variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_SSH");
    BOOL bDone = FALSE;

    //code
    //WNDCLASSEX initialization

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style =  CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    //Register above wndClass
    RegisterClassEx(&wndclass);

    //centering
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    //create the window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW, szAppName, TEXT("RTR7 Siddhesh Sharad Hande"),WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, ScreenWidth/2 - WIN_WIDTH/2, ScreenHeight/2 - WIN_HEIGHT/2, WIN_WIDTH, WIN_HEIGHT, NULL,NULL, hInstance, NULL);

    //set global window handle
    ghwnd = hwnd;

    int iResult = initialize();
    if(iResult != 0)
    {
        fprintf(gpFile, "SSH: WinMain: initialize() failed");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "SSH: WinMain: initialize() successed");
    }

    //Show Window
    ShowWindow(hwnd, iCmdShow);

    //Update the window to paint its background

    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //Game loop
    while(bDone == FALSE)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(bActiveWindow == TRUE)
            {
                if(bEscapeKeyIsPressed == TRUE)
                {
                    bDone = TRUE;
                }
                //render
                render();
                //update
                update();
            }
        }
    }
    
    //unintialize
    unintialize();

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    //func declarations
    void toggleFullScreen(void);
    void resize(int, int);
    void unintialize(void);

    //code
    switch(iMsg)
    {
        case WM_CREATE:
            memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
            wpPrev.length = sizeof(WINDOWPLACEMENT);
            break;

        case WM_SETFOCUS:
            bActiveWindow = TRUE;
            break;

        case WM_KILLFOCUS:
            bActiveWindow = FALSE;
            break;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
            switch(wParam)
            {
                case VK_ESCAPE:
                    bEscapeKeyIsPressed = TRUE;
                    break;

                default:
                    break;
            }
            break;
        case WM_CHAR:
            switch (wParam)
            {
            case 'F':
            case 'f':
                if(bFullScreen == FALSE)
                {
                    toggleFullScreen();
                    bFullScreen = TRUE;
                }
                else
                {
                    toggleFullScreen();
                    bFullScreen = FALSE;
                }
                break;
            case 'P':
            case 'p':
                numberOfPointsShow = numberOfPointsShow + 20;
                if(numberOfPointsShow == NUMBER_OF_POINTS)
                {
                    numberOfPointsShow = 100;
                }

            default:
                break;
            }
            break;

        case WM_CLOSE:
            unintialize();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        
        default:
            break;
    }
    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    //variable decln
    MONITORINFO mi;

    //code
    if(bFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED); //WM_NCCALCSIZE            
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

int initialize(void)
{
    //code
    //Func declarations
    void resize(int, int);

    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;
    
    ghdc = GetDC(ghwnd);
    if(ghdc == NULL)
    {
        return -1;
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0)
    {
        return -2;
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return -3;
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL)
    {
        return -4;
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -5;
    }

    //Enable depth
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


    //Filling STAR points
    srand(67);
    //stars
    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        points[i].x = -1.0f + (rand() % 1000) / 500.0f;
        points[i].y = -1.0f + (rand() % 1000) / 500.0f;
        fprintf(gpFile, "points[%d].x : %f, points[%d].y : %f\n", i, points[i].x, i, points[i].y);
    }

    //choose screen clear color as blue
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    //code
    if(height <= 0)
    {
        height = 1;
    }

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    //Perspective projection
    gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
}

void render(void)
{
    //code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -2.0f);

    // networkPoints(numberOfPointsShow);
    
    for(int i = 0; i < nextPointVal; i++)
    {
    networkPoints(i);
    connectNetwork(i);
    }
    //do double buffring
    SwapBuffers(ghdc);
}

void update(void)
{
    static int slowSpeed;
    //code
    // anglePyramid = anglePyramid + 0.250f;
    // if(anglePyramid == 360.0f)
    // {
    //     anglePyramid = 0.0f;
    // }

    // angleCube = angleCube + 0.250f;
    // if(angleCube == 360.0f)
    // {
    //     angleCube = 0.0f;
    // }

    
    if(nextPointVal < (numberOfPointsShow - 1))
    {
        if(slowSpeed++ == 30)
        {
            nextPointVal = nextPointVal + 1;
            slowSpeed = 0;
        }
    }
}

void unintialize(void)
{
    //code
    // if exiting in full screen first restore and then proceed
    if(bFullScreen == TRUE)
    {
        toggleFullScreen();
        bFullScreen = FALSE;
    }

    //first cehck the current context and if it is then make it unmake it
    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // now destroy
    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    //release device context
    if(ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    //destroy window
    if(ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    //Close log file
    if(gpFile)
    {
        fprintf(gpFile,"Program termionated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

void cube(void)
{
    glTranslatef(-1.50f, -0.50f, -6.0f);
    glScalef(0.75f, 0.75f, 0.75f);
    glRotatef(angleCube, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	//red, right green, back blue, left cyn, top magenta, bot yellow

    // front
    glColor3f(1.0f, 0.0f, 0.0f);
	
    glVertex3f(1.0f, 1.0f, 1.0f);

    glVertex3f(-1.0f, 1.0f, 1.0f);
	
	glVertex3f(-1.0f, -1.0f, 1.0f);
	
	glVertex3f(1.0f, -1.0f, 1.0f);

    //right
    glColor3f(0.0f, 1.0f, 0.0f);
	
    glVertex3f(1.0f, 1.0f, -1.0f);

    glVertex3f(1.0f,  1.0f,  1.0f);
	
	glVertex3f(1.0f, -1.0f,  1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);

    //back
    glColor3f(0.0f, 0.0f, 1.0f);
	
    glVertex3f(1.0f,  1.0f, -1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);

    //left
    glColor3f(1.0f, 0.0f, 1.0f);
	
    glVertex3f(-1.0f,  1.0f,  1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f,  1.0f);
	
    //top
    glColor3f(0.0f, 1.0f, 1.0f);
	
    glVertex3f(1.0f,  1.0f, -1.0f);

    glVertex3f(-1.0f,  1.0f, -1.0f);
	
	glVertex3f(-1.0f,  1.0f,  1.0f);
	
	glVertex3f(1.0f,  1.0f,  1.0f);

    //bottom
    glColor3f(1.0f, 1.0f, 0.0f);
	
    glVertex3f(1.0f, -1.0f,  1.0f);

    glVertex3f(-1.0f, -1.0f,  1.0f);
	
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(1.0f, -1.0f, -1.0f);
    glEnd();
}

void triangle()
{

}

void road(void)
{
    float k = 0.002f;        
    float halfWidth = 0.80f;
    int slices = 50;
    float offset = -0.90f;
    float z, left_edge;
    float centerX;

    glColor3f(0.4f, 0.4f, 0.4f);   // gray road
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++)
    {
        z = -1.0f - (float)i;
        centerX = offset + k * z * z;

        glVertex3f(centerX - halfWidth, -0.499f, z);
        glVertex3f(centerX + halfWidth, -0.499f, z);
    }
    glEnd();

    //left line
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++)
    {
        z = -1.0f - (float)i;
        centerX = offset + k * z * z;
        left_edge = centerX - halfWidth;

        glVertex3f(left_edge + 0.05f, -0.498f, z);
        glVertex3f(left_edge + 0.1f, -0.498f, z);
    }
    glEnd();

    //right line
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++)
    {
        z = -1.0f - (float)i;
        centerX = offset + k * z * z;
        left_edge = centerX + halfWidth;

        glVertex3f(left_edge - 0.01f, -0.498f, z);
        glVertex3f(left_edge - 0.035f, -0.498f, z);
    }
    glEnd();

    //centre line
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    for (int i = 0; i <= slices; i = i + 2)
    {
        z = -1.0f - (float)i;
        float zNext = z - 1.0f;
        float centerX1 = offset + k * z * z;
        float centerX2 = offset + k * zNext * zNext;

        glVertex3f(centerX1 - 0.05f, -0.498f, z);
        glVertex3f(centerX1 + 0.05f, -0.498f, z);
        glVertex3f(centerX2 + 0.05f, -0.498f, zNext);
        glVertex3f(centerX2 - 0.05f, -0.498f, zNext);
    }
    glEnd();

#ifdef SID
    glBegin(GL_QUADS);
    {
        //ground
        // glColor3f(0.0f, 1.0f, 0.0f);
        glColor3f(36.0f/255.0f, 77.0f/255.0f, 50.0f/255.0f);
        glVertex3f(-50.0f, -0.5f, -1.0f);
        glVertex3f(50.0f, -0.5f, -1.0f);
        glVertex3f(50.0f, -0.5f, -50.0f);
        glVertex3f(-50.0f, -0.5f, -50.0f);
    }
    glEnd();

    road();

    glTranslatef(-1.0f, 0.50f,-5.0f);
    tree();
    glTranslatef(-1.0f, -0.1f,-5.0f);
    tree();
    glTranslatef(-1.80f, -0.1f,-5.0f);
    tree();
#endif

}

void tree(void)
{
	//--------------------------------------------------------------------------
	//Tree

	// { 0.541f, 0.306f, 0.298f } // #8A4E4C

	glBegin(GL_TRIANGLE_FAN);

	glColor3f(0.541f, 0.306f, 0.298f);
	glVertex3f(-0.7f, -1.f, 0.0f);

	glColor3f(0.541f, 0.306f, 0.298f);
	glVertex3f(-0.68f, -0.6f, 0.0f);

	glColor3f(0.541f, 0.306f, 0.298f);
	glVertex3f(-0.62f, -.6f, 0.0f);

	glColor3f(0.541f, 0.306f, 0.298f);
	glVertex3f(-0.6f, -1.0f, 0.0f);

	glEnd();

	//------{ 0.522f, 0.988f, 0.000f } // #85FC00
	//=----{ 0.302f, 0.569f, 0.000f } // #4D9100
	glBegin(GL_TRIANGLES);
	
	glColor4f(.522f, 0.988f, 0.000f,0.0);
	glVertex3f(-0.675f, -0.5f, 0.0f);

	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.5f, -.7f, 0.0f);
	
	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.8f, -.7f, 0.0f);
	
	glEnd();

	glBegin(GL_TRIANGLES);
	
	glColor4f(.522f, 0.988f, 0.000f,0.0);
	glVertex3f(-0.65f, -0.4f, 0.0f);

	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.52f, -0.6f, 0.0f);
	
	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.78f, -0.6f, 0.0f);
	
	glEnd();

	glBegin(GL_TRIANGLES);
	
	glColor4f(.522f, 0.988f, 0.000f,0.0);
	glVertex3f(-.65f, -0.32f, 0.0f);

	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.55f, -.5f, 0.0f);
	
	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.75f, -.5f, 0.0f);
	
	glEnd();

	glBegin(GL_TRIANGLES);
	
	glColor4f(.522f, 0.988f, 0.000f,0.0);
	glVertex3f(-0.65f, -0.25f, 0.0f);

	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-.58f, -0.4f, 0.0f);
	
	glColor4f(0.302f, 0.569f, 0.0f,0.0);
	glVertex3f(-0.72f, -0.4f, 0.0f);
	
	glEnd();

	//--------------------------------------------------------------------------
}

void networkPoints(int networkSize)
{
    //stars
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.5f, 0.0f); //green 
    for (int i = 0; i < networkSize; i++)
    {
        glVertex3f(points[i].x, points[i].y, points[i].z);
    }
    glEnd();
}

void connectNetwork(int networkSize)
{
    // glBegin(GL_LINES);
    // glColor3f(0.0f, 0.5f, 0.0f); //green 
    // for (int i = 0; i < (networkSize - 1); i = i + 1)
    // {
    //     glVertex3f(points[i].x, points[i].y, points[i].z);
    //     glVertex3f(points[i + 1].x, points[i + 1].y, points[i].z);
    // }
    // glEnd();

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.5f, 0.0f); //green 

    glVertex3f(points[networkSize].x, points[networkSize].y, points[0].z);
    glVertex3f(points[networkSize + 1].x, points[networkSize + 1].y, points[0].z);

    glEnd();
}