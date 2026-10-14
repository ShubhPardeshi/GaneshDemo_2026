// Header files
#include <Windows.h>
#include <stdlib.h> // for exit
#include <stdio.h>  // For file IO

// opengl related header files
#include <GL/GL.h>
#include <GL/GLU.h>

// link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")

// globale variable declaration
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL bFullScreen = FALSE;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
FILE *gpFile = NULL;

BOOL bActiveWindow = FALSE;
BOOL bEscapeKeyIsPressed = FALSE;

#define NUMBER_OF_POINTS 1000

struct network
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

//Scene 1 
struct network points[NUMBER_OF_POINTS];
static int nextPointVal;
float sweepXLeft = -1.0f; 
float sweepXRight = 1.0f; 
float sweepDelta = 0.00005f; 
int delay; 
static int slowSpeed;
BOOL bBlankScreen = FALSE; 

//Scene 2 
BOOL bScene2 = FALSE;
GLuint fontBase = 0;

//Scene 3
BOOL bScene3 = FALSE;

enum stateMachine
{
    GreenPoints, 
    GoldenPoints, 
    LineConnect, 
    BlankScreen
}state ; 

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// entry point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // func declarations
    int initialize(void);
    void unintialize(void);
    void render(void);
    void update(void);

    // create log file
    gpFile = fopen("Log.txt", "w");

    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("Log file creation failed"), TEXT("ERROR"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Program started successfully\n");
    }

    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("RTR7_SSH");
    BOOL bDone = FALSE;

    // WNDCLASSEX initialization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    // Register above wndClass
    RegisterClassEx(&wndclass);

    // centering
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    // create the window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          szAppName,
                          TEXT("RTR7-027:Shubhangi Pardeshi"),
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                          ScreenWidth / 2 - WIN_WIDTH / 2,
                          ScreenHeight / 2 - WIN_HEIGHT / 2,
                          WIN_WIDTH,
                          WIN_HEIGHT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    // set global window handle
    ghwnd = hwnd;

    int iResult = initialize();
    if (iResult != 0)
    {
        fprintf(gpFile, "SSH: WinMain: initialize() failed");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "SSH: WinMain: initialize() successed");
    }

    // Show Window
    ShowWindow(hwnd, iCmdShow);

    // Update the window to paint its background
    UpdateWindow(hwnd);

    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // Game loop
    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
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
            if (bActiveWindow == TRUE)
            {
                if (bEscapeKeyIsPressed == TRUE)
                {
                    bDone = TRUE;
                }
                // render
                render();
                // update
                update();
            }
        }
    }

    // unintialize
    unintialize();

    return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // func declarations
    void toggleFullScreen(void);
    void resize(int, int);
    void unintialize(void);

    // code
    switch (iMsg)
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
        switch (wParam)
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
            if (bFullScreen == FALSE)
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
        case 'S':
        case 's': 
            bScene2 = TRUE;
            break;
        case 'B':
        case 'b': 
            bScene3 = TRUE;
            break;
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
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullScreen(void)
{
    // variable decln
    MONITORINFO mi;

    // code
    if (bFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            memset(&mi, 0, sizeof(MONITORINFO));
            mi.cbSize = sizeof(MONITORINFO);
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
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
    // Func declarations
    void resize(int, int);

    // variable declarations
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
    if (ghdc == NULL)
    {
        return -1;
    }

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        return -2;
    }

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        return -3;
    }

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        return -4;
    }

    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -5;
    }

    // Enable depth
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // choose screen clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Network point genration 
    srand(54);
    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        points[i].x = -1.0f + (rand() % 1000) / 500.0f;
        points[i].y = -1.0f + (rand() % 1000) / 500.0f;
        points[i].z = 0.0f;
    }
    
    HFONT hFont;
    HFONT hOldFont;

    fontBase = glGenLists(96);
    hFont = CreateFont(100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                       ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
                       TEXT("Arial"));
    hOldFont = (HFONT)SelectObject(ghdc, hFont);
    wglUseFontBitmaps(ghdc, 32, 96, fontBase);
    SelectObject(ghdc, hOldFont);
    DeleteObject(hFont);

    // warm up resize
    resize(WIN_WIDTH, WIN_HEIGHT);

    return 0;
}

void resize(int width, int height)
{
    // code
    if (height <= 0)
    {
        height = 1;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    // Perspective projection
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void render(void)
{
    // code
    void networkPoints(int);
    void connectNetwork(int);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glPushMatrix(); //1

    glTranslatef(0.0f, 0.0f, -1.0f);
    glScalef(1.1f, 0.8f, 0.0f);
    
    networkPoints(NUMBER_OF_POINTS);
    connectNetwork(NUMBER_OF_POINTS); 

    glPopMatrix(); //1
    
    if(bBlankScreen)
        //code 

    if(bScene2 == TRUE)
    {
        glPushMatrix();

        glTranslatef(0.0f, 0.6f, -2.0f);
        glScalef(1.7f, 0.5f, 0.0f);
        
        networkPoints(NUMBER_OF_POINTS);
        //connectNetwork(NUMBER_OF_POINTS);
        
        char text[] = "Happy Birthday!!!";

        glColor3f(0.85f, 0.70f, 0.20f);
        glRasterPos3f(-0.5f, -1.5f, 0.0f);

        glListBase(fontBase - 32);
        glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
    }
    else if(bScene3 == TRUE)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glColor4f(0.0f, 0.0f, 0.0f, 0.0f);

            glBegin(GL_QUADS);
            glVertex3f(-1.0f, -1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 0.0f);
            glVertex3f(1.0f, 1.0f, 0.0f);
            glVertex3f(-1.0f, 1.0f, 0.0f);
            glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }


    // do double buffering
    SwapBuffers(ghdc);
}

void update(void)
{
    if(state == GreenPoints)
    {
       delay++; 
       if(delay > 120)
       {
            state = GoldenPoints; 
            delay = 0;
       }
        
    }
    else if (state == GoldenPoints)  // gold left → right
    {
         if(sweepXLeft < sweepXRight)
            sweepXLeft += sweepDelta; //speed
        else 
            state = LineConnect; 
    }
    else if (state == LineConnect)
    {
       if(nextPointVal < (NUMBER_OF_POINTS - 1))
        {
         if(slowSpeed++ == 30)
         {
            nextPointVal = nextPointVal + 10;
            slowSpeed = 0;
         }
        }
        else 
            state = BlankScreen;
    }
    else if (state == BlankScreen)  
    {
        bBlankScreen = TRUE;
         if(sweepXLeft < sweepXRight)
         {
            sweepXLeft += 0.05f; //speed
         } 
            //state = LineConnect; 
    }
}

void unintialize(void)
{
    // code
    // if exiting in full screen first restore and then proceed
    if (bFullScreen == TRUE)
    {
        toggleFullScreen();
        bFullScreen = FALSE;
    }

    // first check the current context and if it is then unmake it
    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
    }

    // now destroy
    if (ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    // release device context
    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // destroy window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // Close log file
    if (gpFile)
    {
        fprintf(gpFile, "Program termionated successfully\n");
        fclose(gpFile);
        gpFile = NULL;
    }
}

void networkPoints(int networkSize)
{
    //stars
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.5f, 0.0f); //green 
    for (int i = 0; i < (networkSize-1); i++)
    {
        if (points[i].x <= sweepXLeft)
            glColor3f(0.85f, 0.70f, 0.20f);  // gold
        else
            glColor3f(0.0f, 0.5f, 0.0f);     // green

        glVertex3f(points[i].x, points[i].y, points[i].z);
    }
    glEnd();
}

void connectNetwork(int networkSize)
{
    glBegin(GL_LINES);
    glColor3f(0.85f, 0.70f, 0.20f); //gold 
    for(int i = 0; i < nextPointVal; i++)
    {
        glVertex3f(points[i].x, points[i].y, points[0].z);
        glVertex3f(points[i + 1].x, points[i + 1].y, points[0].z);

        if(bScene2)
            break; 
    }
    glEnd();
}
