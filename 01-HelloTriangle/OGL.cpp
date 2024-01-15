// Header Files
#include <windows.h>
#include "OGL.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vmath.h"
using namespace vmath;

// OpenGL Header Files
#include <GL/glew.h>
#include <GL/gl.h>

#define WINWIDTH 800
#define WINHEIGHT 600

// OpenGL Libraries
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "openGL32.lib")

// Global Function Declarations / Signatures
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable Declarations
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

BOOL gbFullscreen = FALSE;
FILE *gpFile = NULL;
FILE *vshaderFile = NULL;
FILE *fshaderFile = NULL;
BOOL gbActiveWindow = FALSE;
BOOL bDone = FALSE;
int iRetVal = 0;
enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTURE0
};

GLuint vao1;
GLuint vbo1;
GLuint vao2;
GLuint vbo2;
GLuint vbo_color1;
GLuint vbo_color2;
GLuint mvpMatrixUniform;

mat4 perspectiveProjectionMatrix;

// Programmable pipeline related global variables
GLuint shaderProgramObject;

// Entery Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // function declarations

    int initialise(void);
    void display(void);
    void update(void);
    void uninitialise(void);

    // variable declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyWindow");

    int xscreen = GetSystemMetrics(SM_CXSCREEN);
    int yscreen = GetSystemMetrics(SM_CYSCREEN);

    // code

    if (fopen_s(&gpFile, "Log.txt", "w") != 0)
    {
        MessageBox(NULL, TEXT("Creation of Log File Failed. Exiting..."), TEXT("File I/O Error"), MB_OK);
        exit(0);
    }
    else
    {
        fprintf(gpFile, "Log File is successfully created\n");
    }

    // Initialization of WNDCLASSEX structure
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;

    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // Registering above wndclassex
    RegisterClassEx(&wndclass);

    // Create the window
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                        szAppName,
                        TEXT("OpenGL Window"),
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                        (xscreen - WINWIDTH) / 2,
                        (yscreen - WINHEIGHT) / 2,
                        WINWIDTH,
                        WINHEIGHT,
                        NULL,
                        NULL,
                        hInstance,
                        NULL);
    ghwnd = hwnd;

    // Initialise
    iRetVal = initialise();

    if (iRetVal == -1)
    {
        fprintf(gpFile, "Choose Pixel Format Failed\n");
        uninitialise();
    }
    else if(iRetVal == -2)
    {
        fprintf(gpFile, "Set Pixel Format Failed\n");
        uninitialise();
    }
    else if (iRetVal == -3)
    {
        fprintf(gpFile, "Create OpenGL Context Failed\n");
        uninitialise();
    }
    else if (iRetVal == -4)
    {
        fprintf(gpFile, "Making openGL context as current context failed\n");
        uninitialise();
    }
    else if (iRetVal == -5)
    {
        fprintf(gpFile, "");
        uninitialise();
    }
    else if (iRetVal == -6)
    {
        fprintf(gpFile, "");
        uninitialise();
    }
    else
    {
        fprintf(gpFile, "Success\n");
    }

    // Show Window
    ShowWindow(hwnd, iCmdShow);

    // Foregrounding and Focusing the window
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                bDone = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActiveWindow == TRUE)
            {
                // render the scene
                display();

                // update the scene
                update();
            }
        }
    }

    uninitialise();

    return (int)msg.wParam;
}

// CALLBACK FUNCTION
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Function Declarations
    void ToggleFullScreen(void);
    void resize(int, int);

    // code
    switch(iMsg)
    {
        case WM_KEYDOWN:
            switch(wParam)
            {
                case 27:
                    DestroyWindow(hwnd);
                    break;
                default:
                    break;
            }
            break;
        case WM_SETFOCUS:
            gbActiveWindow = TRUE;
            break;
        case WM_KILLFOCUS:
            gbActiveWindow = FALSE;
            break;
        case WM_ERASEBKGND:
            return 0;
        case WM_CHAR:
            switch (wParam)
            {
                case 'F':
                case 'f':
                    ToggleFullScreen();
                    break;
                default:
                    break;
            }
            break;
        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


void ToggleFullScreen(void)
{
	// Variable Declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;
	
	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullscreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
			gbFullscreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullscreen = FALSE;
	}
}

int initialise(void)
{
    // Function declarations
    void resize(int, int);
    void printGLinfo(void);
    void uninitialise(void);
    GLchar *LoadShader(FILE *, char *);

    // Variable Declarations

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    // Code

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    // GetDC

    ghdc = GetDC(ghwnd);

    // Choose Pixel Format

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        return -1;
    }

    // Set the chosen pixel format

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
        return -2;

    ghrc = wglCreateContext(ghdc);
    if (ghrc == NULL)
    {
        return -3;
    }

    // Make the rendering context as current context

    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        return -4;
    }

    // GLEW initialisation

    if (glewInit() != GLEW_OK)
    {
        return -5;
    }

    // vertex shader
    // const GLchar* vertexShaderSourceCode=
    // "#version 450 core" \
    // "\n"  \
    // "void main(void)" \
    // "{" \
    // "}";

    const GLchar *vertexShaderSourceCode = LoadShader(vshaderFile, "hellotriangle.vert");

    // fprintf(gpFile, "%s\n", buffer);

    // strcpy(buffer, vertexShaderSourceCode);


    GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
    glCompileShader(vertexShaderObject);

    // error checking for shader
    GLint status;
    GLint infoLogLength;
    char *log = NULL;

    glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char *)malloc(infoLogLength);
            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(vertexShaderObject, infoLogLength, &written, log);
                fprintf(gpFile, "Vertex Shader Compilation Log: %s\n", log);
                free(log);
                uninitialise();
            }
        }
    }

    // fragment shader

    const GLchar* fragmentShaderSourceCode = LoadShader(fshaderFile, "hellotriangle.frag");

    GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
    glCompileShader(fragmentShaderObject);

    // pre linked binding
    
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION, "a_position");
    glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_COLOR, "a_color");


    // check errors

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
    if (status==GL_FALSE)
    {
        glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char *)malloc(infoLogLength);
            if (log != NULL)
            {
                GLsizei written;
                glGetShaderInfoLog(fragmentShaderObject, infoLogLength, &written, log);
                fprintf(gpFile, "Fragment Shader Compilation Log: %s\n", log);
                free(log);
                uninitialise();
            }
        }
    }

    // shader program object

    shaderProgramObject = glCreateProgram();
    glAttachShader(shaderProgramObject, vertexShaderObject);
    glAttachShader(shaderProgramObject, fragmentShaderObject);

    glLinkProgram(shaderProgramObject);

    status = 0;
    infoLogLength = 0;
    log = NULL;

    glGetProgramiv(shaderProgramObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        glGetProgramiv(shaderProgramObject, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 0)
        {
            log = (char *)malloc(infoLogLength);
            if (log != NULL)
            {
                GLsizei written;
                glGetProgramInfoLog(shaderProgramObject, infoLogLength, &written, log);
                fprintf(gpFile, "Shader Program Linking Log: %s\n", log);
                free(log);
                uninitialise();
            }
        }
    }

    mvpMatrixUniform = glGetUniformLocation(shaderProgramObject, "u_mvpMatrix");

    // declaration of vertex data arrays
    const GLfloat trianglePosition[] = 
	{
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};

    const GLfloat triangleColor[] =
    {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    const GLfloat otherTriangleColor[] =
    {
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };


    // vao for triangle
    glGenVertexArrays(1, &vao1);
    glBindVertexArray(vao1);
        glGenBuffers(1, &vbo1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePosition), trianglePosition, GL_STATIC_DRAW);
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_color1);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleColor), triangleColor, GL_STATIC_DRAW);
        glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(vao1);

    // vao for triangle
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
        glGenBuffers(1, &vbo2);
        glBindBuffer(GL_ARRAY_BUFFER, vbo2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePosition), trianglePosition, GL_STATIC_DRAW);
        glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_color2);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_color2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(otherTriangleColor), otherTriangleColor, GL_STATIC_DRAW);
        glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(vao2);


    // Here starts OpenGL

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Clear the screen 

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjectionMatrix = mat4::identity();

    resize(WINWIDTH, WINHEIGHT);

    return 0;
}

void resize(int width, int height)
{
    if (height == 0)
    {
        height = 1;
    }

    glViewport(0, 0, width, height);

    perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);
}

void display(void)
{
    // code

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use shader program object
    glUseProgram(shaderProgramObject);

    // here there be dragons
    mat4 translationMatrix = mat4::identity();
    mat4 modelViewMatrix = mat4::identity();
    mat4 modelViewProjectionMatrix = mat4::identity();

    translationMatrix = vmath::translate(-1.5f, 0.0f, -4.0f);
    modelViewMatrix = translationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    glBindVertexArray(vao1);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);

    translationMatrix = vmath::translate(1.5f, 0.0f, -4.0f);
    modelViewMatrix = translationMatrix;

    modelViewProjectionMatrix = perspectiveProjectionMatrix * modelViewMatrix;
    glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, modelViewProjectionMatrix);

    glBindVertexArray(vao2);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
    // unuse shader program object
    glUseProgram(0);
    SwapBuffers(ghdc);
}

void update(void)
{
    // code
}

void uninitialise(void)
{
    // Function Declarations
    void ToggleFullScreen(void);

    // code

    if (gpFile)
    {
        fprintf(gpFile, "Logfile successfully closed\n");
        fclose(gpFile);
        gpFile = NULL;
    }

    if (gbFullscreen)
    {
        ToggleFullScreen();
    }

    if (shaderProgramObject)
    {
        glUseProgram(shaderProgramObject);
        GLsizei numAttachedShaders;
        glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
        GLuint *shaderObjects = NULL;

        // allocate enough memory to this buffer according to the number of attached shader objects
        shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));

        glGetAttachedShaders(shaderProgramObject, numAttachedShaders, &numAttachedShaders, shaderObjects);

        for (GLsizei i = 0; i < numAttachedShaders; i++)
        {
            glDetachShader(shaderProgramObject, shaderObjects[i]);
            glDeleteShader(shaderObjects[i]);
            shaderObjects[i] = 0;
        }

        free(shaderObjects);
        shaderObjects = NULL;

        glUseProgram(0);
        glDeleteProgram(shaderProgramObject);
        shaderProgramObject = 0;
    }

    if (wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL, NULL);
        if (ghrc)
        {
            wglDeleteContext(ghrc);
            ghrc = NULL;
        }
    }

    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if (ghwnd)
    {
        DestroyWindow(ghwnd);
    }
}

GLchar *LoadShader(FILE *shaderFile, char *shaderFileName)
{
    long fileLength;

    if (fopen_s(&shaderFile, shaderFileName, "r") != 0)
    {
        fprintf(gpFile, "%s could not be read\n", shaderFileName);
    }
    else
    {
        fprintf(gpFile, "%s read successfully\n", shaderFileName);
    }

    // Get the file length
    fseek(shaderFile, 0, SEEK_END);
    fileLength = ftell(shaderFile);
    fseek(shaderFile, 0, SEEK_SET);

    // allocate memory for buffer
    GLchar *shaderSourceCode = (char *)malloc(fileLength + 1);

    if (shaderSourceCode == NULL)
    {
        fprintf(gpFile, "Error allocating memory for vertex shader source code\n");
        fclose(shaderFile);
        return "Error";
    }

    // read the file into the buffer
    fread(shaderSourceCode, fileLength, 1, shaderFile);

    shaderSourceCode[fileLength] = '\0';

    // close the file
    fclose(shaderFile);

    return shaderSourceCode;
    
}




