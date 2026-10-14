// Header files
#include <Windows.h>
#include <stdlib.h> // for exit
#include <stdio.h>  // For file IO
#include <string.h> // for strlen

// opengl related header files
#include <GL/GL.h>
#include <GL/GLU.h>

// link with opengl import library
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLU32.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define NUMBER_OF_POINTS 700
#define FADE_AT_SECONDS 120.0f

// Scene IDs
#define SCENE_1 1
#define SCENE_2 2
#define SCENE_3 3

// Scene 1 sub-phases
#define PHASE_GREEN 0
#define PHASE_GOLD 1
#define PHASE_CONNECT 2

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

struct network
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct network points[NUMBER_OF_POINTS];

int currentScene = SCENE_1;
int scene1Phase = PHASE_GREEN;
int delayCounter = 0;
int connectSlowSpeed = 0;
int connectCount = 0;

float goldSweepX = -1.0f;
float goldSweepSpeed = 0.004f;
float fadeAlpha = 0.0f;

DWORD startTick = 0;
float totalTimeSec = 0.0f;

GLuint fontBase = 0;

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void drawNetworkPoints(BOOL useUpperBand);
void drawNetworkConnections(BOOL useUpperBand);
void drawHappyBirthday(void);
void drawECG(void);
void drawFadeToBlack(void);
float mapPointY(float y, BOOL useUpperBand);
void buildBitmapFont(void);

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
                          TEXT("RTR7 Member7 - Point Network"),
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
        fprintf(gpFile, "SSH: WinMain: initialize() failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "SSH: WinMain: initialize() successed\n");
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

void buildBitmapFont(void)
{
    HFONT hFont;
    HFONT hOldFont;

    fontBase = glGenLists(96);
    hFont = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                       ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
                       TEXT("Arial"));
    hOldFont = (HFONT)SelectObject(ghdc, hFont);
    wglUseFontBitmaps(ghdc, 32, 96, fontBase);
    SelectObject(ghdc, hOldFont);
    DeleteObject(hFont);
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

    // Deterministic random point cloud (full screen range ~[-1, 1])
    srand(67);
    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        points[i].x = -1.0f + (rand() % 1000) / 500.0f;
        points[i].y = -1.0f + (rand() % 1000) / 500.0f;
        points[i].z = 0.0f;
    }

    currentScene = SCENE_1;
    scene1Phase = PHASE_GREEN;
    delayCounter = 0;
    connectCount = 0;
    goldSweepX = -1.0f;
    fadeAlpha = 0.0f;

    buildBitmapFont();

    startTick = GetTickCount();

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

float mapPointY(float y, BOOL useUpperBand)
{
    if (useUpperBand == TRUE)
    {
        // Compress original y [-1,1] into upper third ~[0.25, 0.95]
        return 0.25f + ((y + 1.0f) * 0.5f) * 0.70f;
    }
    return y;
}

void drawNetworkPoints(BOOL useUpperBand)
{
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUMBER_OF_POINTS; i++)
    {
        if (points[i].x <= goldSweepX)
            glColor3f(0.85f, 0.70f, 0.20f); // gold
        else
            glColor3f(0.0f, 0.55f, 0.0f); // green

        glVertex3f(points[i].x, mapPointY(points[i].y, useUpperBand), points[i].z);
    }
    glEnd();
}

void drawNetworkConnections(BOOL useUpperBand)
{
    int maxLinks = connectCount;
    if (maxLinks > NUMBER_OF_POINTS - 1)
        maxLinks = NUMBER_OF_POINTS - 1;

    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glColor3f(0.85f, 0.70f, 0.20f); // gold links
    for (int i = 0; i < maxLinks; i++)
    {
        // Only connect if both ends are already gold
        if (points[i].x <= goldSweepX && points[i + 1].x <= goldSweepX)
        {
            glVertex3f(points[i].x, mapPointY(points[i].y, useUpperBand), points[i].z);
            glVertex3f(points[i + 1].x, mapPointY(points[i + 1].y, useUpperBand), points[i + 1].z);
        }
    }
    glEnd();
}

void drawHappyBirthday(void)
{
    char text[] = "Happy Birthday";

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos3f(-0.55f, 0.0f, 0.0f);

    glListBase(fontBase - 32);
    glCallLists((GLsizei)strlen(text), GL_UNSIGNED_BYTE, text);
}

void drawECG(void)
{
    // Reserved lower band — ECG not delivered yet
}

void drawFadeToBlack(void)
{
    if (fadeAlpha <= 0.0f)
        return;

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

    glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
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

void render(void)
{
    // code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2.2f);
    glScalef(1.0f, 0.85f, 1.0f);

    if (currentScene == SCENE_1)
    {
        // Full-screen network
        drawNetworkPoints(FALSE);
        if (scene1Phase == PHASE_CONNECT)
            drawNetworkConnections(FALSE);
    }
    else if (currentScene == SCENE_2 || currentScene == SCENE_3)
    {
        // Split composition:
        // UPPER  - golden network
        // MIDDLE - Happy Birthday
        // LOWER  - ECG (empty for now)
        drawNetworkPoints(TRUE);
        drawNetworkConnections(TRUE);
        drawHappyBirthday();
        drawECG();
    }

    if (currentScene == SCENE_3)
        drawFadeToBlack();

    // do double buffering
    SwapBuffers(ghdc);
}

void update(void)
{
    totalTimeSec = (float)(GetTickCount() - startTick) / 1000.0f;

    // Global: at exactly 02:00 enter fade scene
    if (totalTimeSec >= FADE_AT_SECONDS && currentScene != SCENE_3)
    {
        currentScene = SCENE_3;
    }

    if (currentScene == SCENE_1)
    {
        if (scene1Phase == PHASE_GREEN)
        {
            // Hold green points, then start gold sweep
            delayCounter++;
            if (delayCounter > 120) // ~2 sec at 60 FPS
            {
                scene1Phase = PHASE_GOLD;
                delayCounter = 0;
            }
        }
        else if (scene1Phase == PHASE_GOLD)
        {
            if (goldSweepX < 1.0f)
            {
                goldSweepX += goldSweepSpeed;
                if (goldSweepX > 1.0f)
                    goldSweepX = 1.0f;
            }
            else
            {
                // Short hold all-gold, then connect
                delayCounter++;
                if (delayCounter > 60)
                {
                    scene1Phase = PHASE_CONNECT;
                    delayCounter = 0;
                    connectCount = 0;
                }
            }
        }
        else if (scene1Phase == PHASE_CONNECT)
        {
            if (connectCount < NUMBER_OF_POINTS - 1)
            {
                if (connectSlowSpeed++ >= 2)
                {
                    connectSlowSpeed = 0;
                    connectCount++;
                }
            }
            else
            {
                // Network complete -> Scene 2 split layout
                delayCounter++;
                if (delayCounter > 90)
                {
                    currentScene = SCENE_2;
                    delayCounter = 0;
                    goldSweepX = 1.0f; // keep all gold
                    connectCount = NUMBER_OF_POINTS - 1;
                }
            }
        }
    }
    else if (currentScene == SCENE_2)
    {
        // Hold split screen until 02:00 (handled above)
    }
    else if (currentScene == SCENE_3)
    {
        // Fade to black
        if (fadeAlpha < 1.0f)
        {
            fadeAlpha += 0.008f;
            if (fadeAlpha > 1.0f)
                fadeAlpha = 1.0f;
        }
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

    if (fontBase != 0)
    {
        glDeleteLists(fontBase, 96);
        fontBase = 0;
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
