// Header files
#include <windows.h>
#include "OGL.h"
#include <stdlib.h>
#include <stdio.h>

// OpenGL Header Files
#include <GL/glew.h>	// This must be above GL.h
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
HGLRC ghrc = NULL; // HGLRC -> Handle to opengl rendering context

BOOL gbFullscreen = FALSE;
FILE* gpFile = NULL;
BOOL gbActiveWindow = FALSE;
BOOL bDone = FALSE;
int iRetVal = 0;

// Programmamble pipeline related global variables
GLuint shaderProgramObject;

// Entry Point Function
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
	 
	// INitialization of WNDCLASSEX structure
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
	//wndclass.

	// Registering above wndclassex
	RegisterClassEx(&wndclass);

	// Create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
						szAppName,
						TEXT("OpenGL Window"),
						WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, // My window will clip others, others will not clip me! (clipchildren and siblings), WS_VISIBLE given because there is no WM_PAINT. initially visible irrespective of WM_PAINT
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
		fprintf(gpFile, "Choose Pixel Format Failed");
		uninitialise();
	}

	else if (iRetVal == -2)
	{
		fprintf(gpFile, "Set Pixel Format Failed");
		uninitialise();
	}

	else if (iRetVal == -3)
	{
		fprintf(gpFile, "Create OpenGL Context Failed");
		uninitialise();
	}

	else if (iRetVal == -4)
	{
		fprintf(gpFile, "Making openGL context as current context failed");
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
		fprintf(gpFile, "Success");
	}

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	// Update the Window
	// UpdateWindow(hwnd);

	// Foregrounding and Focusing the Window
	SetForegroundWindow(hwnd); // ghwnd and hwnd both will work - but hwnd is inside winmain while ghwnd was created for functions other than winmain
	SetFocus(hwnd);

	// Message Loop
	//while (GetMessage(&msg, NULL, 0, 0))
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

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
				// Render the Scene
				display();

				// Update the Scene
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
	// void uninitialise(void);

	// code
	switch (iMsg)
	{
	case WM_KEYDOWN:
		switch (wParam)
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
		// break;
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
		// uninitialise();
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
	// Function Declarations
	void resize(int, int);
	void printGLinfo(void);
	void uninitialise(void);

	// Variable Declarations

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// Code

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR)); // Initialisation of pixelformatdescriptor structure :: memset((void *) &pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR))
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR); // n -> short int
	pfd.nVersion = 1; // conventionally it should be 1.0
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; //gimme a pixel that draws on a window | pixel format should support OpenGL | 
	pfd.iPixelType = PFD_TYPE_RGBA; // gimme a pixel that uses RGBA color scheme
	pfd.cColorBits = 32; // 8 bits - green, 8 bits - red, 8 bits - blue, 8 bits - alpha -> 32 bit color bits
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;
	
	// GetDC

	ghdc = GetDC(ghwnd);

	// Choose Pixel Format

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd); // pfd is an in-out parameter
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}

	// Set the chosen pixel format

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) // 
		return -2;

	// Create OpenGL rendering context

	ghrc = wglCreateContext(ghdc); // wgl -> bridging API for windows
	if (ghrc == NULL)
	{
		return -3;
	}

	// Make the rendering context as current context

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) // make ghrc the current context, remove ghdc from current context
	{
		return -4;
	}

	// GLEW initialisation

	if (glewInit() != GLEW_OK)
	{
		return -5;
	}

	// print OpenGL info

	// printGLinfo();

	// vertex shader - check where to put this
	const GLchar* vertexShaderSourceCode = 
	"#version 450 core" \
	"\n" \
	"void main(void)" \
	"{" \
	"}";

	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode, NULL); // give shader to vertex
	glCompileShader(vertexShaderObject);

	// error checking for shader
	GLint status;
	GLint infoLogLength;
	char* log = NULL;

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

	const GLchar* fragmentShaderSourceCode = 
	"#version 450 core" \
	"\n" \
	"void main(void)" \
	"{" \
	"}";

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL); // give shader to fragment
	glCompileShader(fragmentShaderObject);

	// check error for fragment shader

	status = 0;
	infoLogLength = 0;
	log = NULL;

	glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
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

	// Here starts openGL code

	glClearDepth(1.0f); // ?
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// glShadeModel(GL_SMOOTH);
	// glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	// Clear the screen using Blue Color

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // this function only tells the program what color to use to clear the screen. The actual clearing DOES NOT happen here

	resize(WINWIDTH, WINHEIGHT);

	
	return 0;

}

void printGLinfo(void)
{
	// local variable declarations
	GLint numExtensions = 0;


	// code
	fprintf(gpFile, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
	fprintf(gpFile, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(gpFile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
	fprintf(gpFile, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	fprintf(gpFile, "Number of supported extensions: %d\n", numExtensions);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            

	for(int i = 0; i < numExtensions; i++)
	{
		fprintf(gpFile, "%s\n", glGetStringi(GL_EXTENSIONS, i));
	}

	
}

void resize(int width, int height)
{
	// Code
	if (height == 0) // To avoid possible divide by zero in future code
	{
		height = 1;
	}

	glViewport(0, 0, width, height); // screen cha konta bhaag dakhvu?

}

void display(void)
{
	// Code

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Actual coloring and clearing happens here
	
	// Use the shader program object
	glUseProgram(shaderProgramObject);

	// here there be dragons (drawing code)

	// unuse the shader program object
	glUseProgram(0);
	SwapBuffers(ghdc);

}

void update(void)
{
	// Code
}

void uninitialise(void)
{
	// Function Declarations
	void ToggleFullScreen(void);
	// Code
	if (gpFile)
	{
		fprintf(gpFile, "Logfile Successfully Closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}

	if (gbFullscreen)
	{
		ToggleFullScreen();
	}

	// shader uninitialisation
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint *shaderObjects = NULL;

		// allocate enough memory to this buffer according to the number of attached shaders and fill it with the attached shader objects
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

	if(ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
	}

	// if (gpFile)
	// {
	// 	fprintf(gpFile, "Logfile Successfully Closed\n");
	// 	fclose(gpFile);
	// 	gpFile = NULL;
	// }
}





