#include <iostream>
#include <windows.h>
#include <fcntl.h>
#include <vector>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <gl/gl.h>
#include "game.h"

const float _time_dif=0.01;//60 fps

using namespace std;

int _w_width=603;
int _w_height=600;
bool _keys[256];
vector< vector<int> > g_vec_board;
vector< vector<int> > g_vec_board_next;

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    string command_line=lpCmdLine;
    bool enable_sound=true;
    if(command_line=="nosound")
    {
        enable_sound=false;
    }

    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "Break-in";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    //debug console
    if(false)
    {
        AllocConsole();

        HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
        FILE* hf_out = _fdopen(hCrt, "w");
        setvbuf(hf_out, NULL, _IONBF, 1);
        *stdout = *hf_out;

        HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
        hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
        FILE* hf_in = _fdopen(hCrt, "r");
        setvbuf(hf_in, NULL, _IONBF, 128);
        *stdin = *hf_in;

    }

    //if fullscreen
    if(false)
    {
        //Detect screen resolution
        RECT desktop;
        // Get a handle to the desktop window
        const HWND hDesktop = GetDesktopWindow();
        // Get the size of screen to the variable desktop
        GetWindowRect(hDesktop, &desktop);
        // The top left corner will have coordinates (0,0)
        // and the bottom right corner will have coordinates
        // (horizontal, vertical)
        _w_width  = desktop.right;
        _w_height = desktop.bottom;
    }

    hwnd = CreateWindowEx(0,
                          "Break-in",
                          "Break-in",
                          WS_BORDER,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          _w_width,
                          _w_height,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    EnableOpenGL(hwnd, &hDC, &hRC);

    //startup
    if(enable_sound) Beep(400, 200);
    game Game;
    if(!Game.init(_w_width,_w_height,_keys,enable_sound))
    {
        //cout<<"ERROR: Could not init\n";
        return 1;
    }

    clock_t time_now=clock();
    clock_t time_last=time_now;

    while (!bQuit)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /*Game.update();
            Game.draw();
            SwapBuffers(hDC);*/

            bool state_updated=false;
            time_now=clock();
            while( time_last+_time_dif*1000 <= time_now )
            {
                time_last+=_time_dif*1000;
                Game.update();
                state_updated=true;
            }
            if(state_updated)
            {
                Game.draw();
                SwapBuffers(hDC);
            }
        }
    }

    DisableOpenGL(hwnd, hDC, hRC);
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
		{
			_keys[wParam]=true;

			cout<<"Button pressed: "<<wParam<<endl;

			if(wParam==VK_ESCAPE) PostQuitMessage(0);
		}
		break;

		case WM_KEYUP:
		{
			_keys[wParam]=false;
		}
		break;

        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    *hDC = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    //set 2D mode
    glClearColor(0.0,0.0,0.0,0.0);  //Set the cleared screen colour to black
    glViewport(0,0,_w_width,_w_height);   //This sets up the viewport so that the coordinates (0, 0) are at the top left of the window

    //Set up the orthographic projection so that coordinates (0, 0) are in the top left
    //and the minimum and maximum depth is -10 and 10. To enable depth just put in
    //glEnable(GL_DEPTH_TEST)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,_w_width,_w_height,0,-1,1);

    //Back to the modelview so we can draw stuff
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /*//Enable antialiasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearStencil(0);*/
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}






