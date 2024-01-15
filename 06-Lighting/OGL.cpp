// Header filess
#include <windows.h>
#include <windowsx.h>
#include "OGL.h"
#include <stdlib.h>
#include <stdio.h>

// OpenGL Header Files
#include <GL/glew.h>	// This must be above GL.h
#include <GL/wglew.h>
#include <GL/gl.h>
#include "vmath.h"
using namespace vmath; // only works in cpp


#define WINWIDTH 800
#define WINHEIGHT 600

// OpenGL Libraries
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "openGL32.lib")

// Global Function Declarations / Signatures
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MouseProc(int, WPARAM, LPARAM);

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
GLuint shaderProgramObject1;
GLuint shaderProgramObject2;
enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXTURE0
};
GLuint vao_pyramid;
GLuint vbo_pyramid_position;
GLuint vbo_pyramid_color;

GLuint vao_cube;
GLuint vbo_cube_position;
GLuint vbo_cube_color;
GLuint vbo_cube_texcoord;
GLuint vbo_cube_normal;
GLuint lightCubeVAO;
GLuint lightCubeVBO;

FILE *vshaderFile = NULL;
FILE *fshaderFile = NULL;


GLfloat anglepyramid = 0.0f;
GLfloat anglecube = 0.0f;

GLuint mvpMatrixUniform;


GLuint vao1;
GLuint vbo1;
GLuint vao2;
GLuint vbo2;
GLuint vbo_color1;
GLuint vbo_color2;
GLuint lightVAO;
GLuint lightVBO;
GLuint modelMatrixUniform;
GLuint viewMatrixUniform;
GLuint projectionMatrixUniform;
GLuint objectColorUniform;
GLuint lightColorUniform;
GLuint viewPosUniform;

GLuint diffuseColorUniform;
GLuint ambientColorUniform;

GLuint materialAmbientUniform;
GLuint materialDiffuseUniform;
GLuint materialSpecularUniform;
GLuint materialShininessUniform;

GLuint lightAmbientUniform;
GLuint lightDiffuseUniform;
GLuint lightSpecularUniform;

GLuint texture_container;
GLuint textureSamplerUniform;

mat4 perspectiveProjectionMatrix;

double deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Programmable pipeline related global variables

vec3 eyeVector = vec3(0.0f, 0.0f, 3.0f);
vec3 centerVector = vec3(0.0f, 0.0f, 1.0f);
vec3 upVector = vec3(0.0f, 1.0f, 0.0f);

vec3 lightPos = vec3(1.2f, 1.0f, 2.0f);
GLuint lightPositionUniform;

// camera related variables
float cameraSpeed;
vmath::vec3 direction;

// for deltaTime
LARGE_INTEGER frequency, currentTime, previousTime;

// for mouse
float xPos = 0.0f;
float yPos = 0.0f;

// initial mouse position
float lastX = 400;
float lastY = 300;

float yaw = 0.0f;
float pitch = 0.0f;

float xoffset;
float yoffset;

bool firstMouse = true;
BOOL vsyncEnabled = FALSE;


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
		fprintf(gpFile, "GLEW init failed return -5");
		uninitialise();
	}

	else if (iRetVal == -6)
	{
		fprintf(gpFile, "Return -6 here");
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

	// Setting Mouse Hook
	SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);

	
    // Setting up Initial time for deltaTime
    // Get the frequency of the performance counter
    QueryPerformanceFrequency(&frequency);

    // Get the previous time from the performance counter (current time in the update() function)
    QueryPerformanceCounter(&previousTime);

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
        case WM_MOUSEMOVE:
            xPos = GET_X_LPARAM(lParam); 
            yPos = GET_Y_LPARAM(lParam);
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
				case 'V':
				case 'v':
					if(vsyncEnabled == TRUE)
					{
						// wglSwapInterval() call for VSYNC
						wglSwapIntervalEXT(0); // 1 -> Enable VSYNC, 2 -> Disable VSYNC
						vsyncEnabled = FALSE;
					}
					else
					{
						// wglSwapInterval() call for VSYNC
						wglSwapIntervalEXT(1); // 1 -> Enable VSYNC, 2 -> Disable VSYNC
						vsyncEnabled = TRUE;
					}
					break;
                case 'W':
                case 'w':
                    eyeVector += cameraSpeed * centerVector;
                    // eyeVector += 0.5f;
                    break;
                case 'S':
                case 's':
                    eyeVector -= cameraSpeed * centerVector;
                    break;
                case 'A':
                case 'a':
                    eyeVector -= vmath::normalize(vmath::cross(centerVector, upVector)) * cameraSpeed;
                    break;
                case 'D':
                case 'd':
                    eyeVector += vmath::normalize(vmath::cross(centerVector, upVector)) * cameraSpeed;
                    break;
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

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	vec3 mouseLook(float, float);
    xoffset = xPos - lastX;
    yoffset = lastY - yPos;

    const float sensitivity = 0.5f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch =  89.0f;

    if(pitch < -89.0f)
        pitch = -89.0f;

	centerVector = mouseLook(yaw, pitch);


    // mouse position
    xoffset = xPos - lastX;
    yoffset = lastY - yPos; // reversed since y-coordinates range from bottom to top
    lastX = xPos;
    lastY = yPos;


    return CallNextHookEx(0, nCode, wParam, lParam);
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
	GLchar *LoadShader(FILE *, char *);
	BOOL loadGLtexture(GLuint *, TCHAR[]);

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

	// Vertex and Fragment Shaders for first Cube

    const GLchar *vertexShaderSourceCode1 = LoadShader(vshaderFile, "vertexShader1.vert");

	GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode1, NULL);
	glCompileShader(vertexShaderObject);

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
	else
	{
		fprintf(gpFile, "Vertex shader successfully compiled!!!! at %d\n", __LINE__);
	}

	// fragment shader
	
    const GLchar* fragmentShaderSourceCode1 = LoadShader(fshaderFile, "fragmentShader1.frag");

	GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode1, NULL);
	glCompileShader(fragmentShaderObject);

	// pre linked binding

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
	else
	{
		fprintf(gpFile, "Fragment shader successfully compiled!!!! at %d\n", __LINE__);
	}

	// shader program object

	shaderProgramObject1 = glCreateProgram();
	glAttachShader(shaderProgramObject1, vertexShaderObject);
	glAttachShader(shaderProgramObject1, fragmentShaderObject);

	glBindAttribLocation(shaderProgramObject1, AMC_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject1, AMC_ATTRIBUTE_NORMAL, "a_normal");
	glBindAttribLocation(shaderProgramObject1, AMC_ATTRIBUTE_TEXTURE0, "a_texcoord");

	glLinkProgram(shaderProgramObject1);

	status = 0;
	infoLogLength = 0;
	log = NULL;

	glGetProgramiv(shaderProgramObject1, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		glGetProgramiv(shaderProgramObject1, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(shaderProgramObject1, infoLogLength, &written, log);
				fprintf(gpFile, "Shader Program Linking Log: %s\n", log);
				free(log);
				uninitialise();
			}
		}
	}

    modelMatrixUniform = glGetUniformLocation(shaderProgramObject1, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject1, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject1, "u_projectionMatrix");
	objectColorUniform = glGetUniformLocation(shaderProgramObject1, "u_objectColor");
	lightColorUniform = glGetUniformLocation(shaderProgramObject1, "u_lightColor");

	lightPositionUniform = glGetUniformLocation(shaderProgramObject1, "u_lightPos");
	viewPosUniform = glGetUniformLocation(shaderProgramObject1, "u_viewPos");


	materialAmbientUniform = glGetUniformLocation(shaderProgramObject1, "material.ambient");
	materialDiffuseUniform = glGetUniformLocation(shaderProgramObject1, "material.diffuse");
	materialSpecularUniform = glGetUniformLocation(shaderProgramObject1, "material.specular");
	materialShininessUniform = glGetUniformLocation(shaderProgramObject1, "material.shininess");

	lightAmbientUniform = glGetUniformLocation(shaderProgramObject1, "light.ambient");
	lightDiffuseUniform = glGetUniformLocation(shaderProgramObject1, "light.diffuse");
	lightSpecularUniform = glGetUniformLocation(shaderProgramObject1, "light.specular");

	textureSamplerUniform = glGetUniformLocation(shaderProgramObject1, "u_textureSampler");

	
	// Vertex and Fragment Shaders for Second Cube

    const GLchar *vertexShaderSourceCode2 = LoadShader(vshaderFile, "vertexShader2.vert");

	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, (const GLchar**)&vertexShaderSourceCode2, NULL);
	glCompileShader(vertexShaderObject);

	status = 0;
	infoLogLength = 0;
	log = NULL;

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
	else
	{
		fprintf(gpFile, "Vertex shader successfully compiled!!!! at %d\n", __LINE__);
	}

	// fragment shader
	
    const GLchar* fragmentShaderSourceCode2 = LoadShader(fshaderFile, "fragmentShader2.frag");

	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode2, NULL);
	glCompileShader(fragmentShaderObject);

	// pre linked binding

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
	else
	{
		fprintf(gpFile, "Fragment shader successfully compiled!!!! at %d\n", __LINE__);
	}

	// shader program object

	shaderProgramObject2 = glCreateProgram();
	glAttachShader(shaderProgramObject2, vertexShaderObject);
	glAttachShader(shaderProgramObject2, fragmentShaderObject);

	glBindAttribLocation(shaderProgramObject2, AMC_ATTRIBUTE_POSITION, "a_position");
	glBindAttribLocation(shaderProgramObject2, AMC_ATTRIBUTE_NORMAL, "a_normal");

	glLinkProgram(shaderProgramObject2);

	status = 0;
	infoLogLength = 0;
	log = NULL;

	glGetProgramiv(shaderProgramObject2, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		glGetProgramiv(shaderProgramObject2, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			log = (char *)malloc(infoLogLength);
			if (log != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(shaderProgramObject2, infoLogLength, &written, log);
				fprintf(gpFile, "Shader Program Linking Log: %s\n", log);
				free(log);
				uninitialise();
			}
		}
	}

    modelMatrixUniform = glGetUniformLocation(shaderProgramObject2, "u_modelMatrix");
    viewMatrixUniform = glGetUniformLocation(shaderProgramObject2, "u_viewMatrix");
    projectionMatrixUniform = glGetUniformLocation(shaderProgramObject2, "u_projectionMatrix");


	// declaration of vertex data arrays
	const GLfloat cubePosition[] =
	{
		// top
    	1.0f, 1.0f, -1.0f,
    	-1.0f, 1.0f, -1.0f, 
    	-1.0f, 1.0f, 1.0f,
    	1.0f, 1.0f, 1.0f,

	  	// bottom
        1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,

		 // front
        1.0f, 1.0f, 1.0f,
       -1.0f, 1.0f, 1.0f,
       -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

		// back
        1.0f, 1.0f, -1.0f,
       -1.0f, 1.0f, -1.0f,
       -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

		 // left
    	-1.0f, 1.0f, 1.0f,
    	-1.0f, 1.0f, -1.0f, 
    	-1.0f, -1.0f, -1.0f, 
    	-1.0f, -1.0f, 1.0f,

		 // right
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
	};

	const GLfloat cubeTexcoords[] =
	{
		0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

		0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
	};
	
	const GLfloat cubeNormals[] = 
	{
		 // top surface
        0.0f, 1.0f, 0.0f,  // top-right of top
        0.0f, 1.0f, 0.0f, // top-left of top
        0.0f, 1.0f, 0.0f, // bottom-left of top
        0.0f, 1.0f, 0.0f,  // bottom-right of top

        // bottom surface
        0.0f, -1.0f, 0.0f,  // top-right of bottom
        0.0f, -1.0f, 0.0f,  // top-left of bottom
        0.0f, -1.0f, 0.0f,  // bottom-left of bottom
        0.0f, -1.0f, 0.0f,   // bottom-right of bottom

        // front surface
        0.0f, 0.0f, 1.0f,  // top-right of front
        0.0f, 0.0f, 1.0f, // top-left of front
        0.0f, 0.0f, 1.0f, // bottom-left of front
        0.0f, 0.0f, 1.0f,  // bottom-right of front

        // back surface
        0.0f, 0.0f, -1.0f,  // top-right of back
        0.0f, 0.0f, -1.0f, // top-left of back
        0.0f, 0.0f, -1.0f, // bottom-left of back
        0.0f, 0.0f, -1.0f,  // bottom-right of back

		// left surface
        -1.0f, 0.0f, 0.0f, // top-right of left
        -1.0f, 0.0f, 0.0f, // top-left of left
        -1.0f, 0.0f, 0.0f, // bottom-left of left
        -1.0f, 0.0f, 0.0f, // bottom-right of left

		 // right surface
        1.0f, 0.0f, 0.0f,  // top-right of right
        1.0f, 0.0f, 0.0f,  // top-left of right
        1.0f, 0.0f, 0.0f,  // bottom-left of right
        1.0f, 0.0f, 0.0f,  // bottom-right of right
	};


	// vao_cube and vbo related code
	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);
	// vao_pyramid done here

		// vbo for position
		glGenBuffers(1, &vbo_cube_position);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubePosition), cubePosition, GL_STATIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// vao_cube color done here

		// // vbo for position
		// glGenBuffers(1, &vbo_cube_normal);
		// glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_normal);
		// glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
		// glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		// glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);

		// glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_position);
	glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_cube_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo for texture
	glGenBuffers(1, &vbo_cube_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexcoords), cubeTexcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	///////////

	// lightCubeVAO and lightCubeVBO related code
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
		// vao_pyramid done here

		// vbo for position
		// glGenBuffers(1, &lightCubeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubePosition), cubePosition, GL_STATIC_DRAW);
		glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	// vao_cube color done here
	glBindVertexArray(0);

	
	glEnable(GL_TEXTURE_2D);

	
	if (loadGLtexture(&texture_container, MAKEINTRESOURCE(IDBITMAP_TEXTURE))==FALSE)
	{
		fprintf(gpFile, "error in texture");

		return -6;
	}


	// Here starts openGL code

	// depth and clear color related code

	glClearDepth(1.0f); // ?
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// glShadeModel(GL_SMOOTH);
	// glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Clear the screen using Blue Color

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // this function only tells the program what color to use to clear the screen. The actual clearing DOES NOT happen here

	perspectiveProjectionMatrix = mat4::identity();

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

	perspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

}

void display(void)
{
	// function declarations

	if (gbActiveWindow)
	{
		SetCapture(ghwnd);
	}
	else
	{
		ReleaseCapture();
	}

	// Code

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Actual coloring and clearing happens here
	
	// Use the shader program object
	glUseProgram(shaderProgramObject1);

	// pyramid

	// transformations
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix_x = mat4::identity();
	mat4 rotationMatrix_y = mat4::identity();
	mat4 rotationMatrix_z = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 scaleMatrix = mat4::identity();
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();

	// mouse look

	glUniform3fv(viewPosUniform, 1, eyeVector);
    
    viewMatrix = vmath::lookat(eyeVector, eyeVector + centerVector, upVector);

	// uniforms for lighting

	glUniform3fv(objectColorUniform, 1, vec3(1.5f, 0.5f, 0.31f));
	glUniform3fv(lightColorUniform, 1, vec3(1.0f, 1.0f, 1.0f));
	
	lightPos = vec3(-3.0f, 0.0f, -3.0f);

	vec3 lightColor = vec3(0.13f, 0.3f, 0.65f);
	glUniform3fv(lightPositionUniform, 1, lightPos);

	vec3 diffuseColor = lightColor * vec3(1.0f, 0.0f, 0.0f);
	vec3 ambientColor = diffuseColor * vec3(0.2f, 0.2f, 0.2f);

	// glUniform3fv(diffuseColorUniform, 1, diffuseColor);
	glUniform3fv(lightColorUniform, 1, lightColor);

	glUniform3fv(materialAmbientUniform, 1, vec3(1.0f, 0.5f, 0.31f));
	glUniform3fv(materialDiffuseUniform, 1, vec3(1.0f, 0.5f, 0.31f));
	glUniform3fv(materialSpecularUniform, 1, vec3(0.5f, 0.5f, 0.5f));
	glUniform1f(materialShininessUniform, 32.0f);

	glUniform3fv(lightAmbientUniform, 1, ambientColor);
	glUniform3fv(lightDiffuseUniform, 1, diffuseColor);
	glUniform3fv(lightSpecularUniform, 1, vec3(1.0f, 1.0f, 1.0f));

	// cube

	// transformations
	translationMatrix = mat4::identity();
	rotationMatrix = mat4::identity();
	scaleMatrix = mat4::identity();

	translationMatrix = vmath::translate(0.0f, 0.0f, -3.0f);
	rotationMatrix_x = vmath::rotate(anglecube, 1.0f, 0.0f, 0.0f);
	rotationMatrix_y = vmath::rotate(anglecube, 0.0f, 1.0f, 0.0f);
	rotationMatrix_z = vmath::rotate(anglecube, 0.0f, 0.0f, 1.0f);
	// rotationMatrix = rotationMatrix_x * rotationMatrix_y * rotationMatrix_z;
	scaleMatrix = vmath::scale(0.75f, 0.75f, 0.75f);

	modelMatrix = translationMatrix  * scaleMatrix * rotationMatrix;
	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_container);
	glUniform1i(textureSamplerUniform, 0);
	
	glBindVertexArray(lightVAO);

	// here there be dragons (drawing code)

	// glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glBindVertexArray(0);

	// unuse the shader program object
	glUseProgram(0);

	glUseProgram(shaderProgramObject2);

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	translationMatrix = mat4::identity();

	viewMatrix = vmath::lookat(eyeVector, eyeVector + centerVector, upVector);

	translationMatrix = vmath::translate(lightPos);
	scaleMatrix = vmath::scale(0.2f, 0.2f, 0.2f);

	modelMatrix = translationMatrix * scaleMatrix;


	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, perspectiveProjectionMatrix);

	glBindVertexArray(lightCubeVAO);

	// here there be dragons (drawing code)

	// glDrawArrays(GL_TRIANGLES, 0, 3);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

	glBindVertexArray(0);

	glUseProgram(0);
	SwapBuffers(ghdc);

}

void update(void)
{
	// function declarations
	double deltaTimeCalculator(double);

	// Code
	anglepyramid = anglepyramid + 0.5f;

	if (anglepyramid >= 360.0f)
	{
		anglepyramid = 0.0f;
	}

	anglecube = anglecube + 0.5f;

	if (anglecube >= 360.0f)
	{
		anglecube = 0.0f;
	}

    // code

    if (firstMouse) // initially set to true to ensure there are no 'jumps' during first time mouse usage
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

   
    // camera stuff
    cameraSpeed = 0.008f * deltaTimeCalculator(deltaTime);

    /////////////////////////////////////////////////////////////////////////////////////
}

void uninitialise(void)
{
	// Function Declarations
	void ToggleFullScreen(void);
	// Code

	// Unhook the mouse
	// UnhookWindowsHookEx(WH_MOUSE_LL);

	if (gbFullscreen)
	{
		ToggleFullScreen();
	}

	// deletion and uninitialisation of vbo

	if (vbo_cube_color)
	{
		glDeleteBuffers(1, &vbo_cube_color);
		vbo_cube_color = 0;
	}

	if (vbo_cube_position)
	{
		glDeleteBuffers(1, &vbo_cube_position);
		vbo_cube_position = 0;
	}

	if (vbo_pyramid_position)
	{
		glDeleteBuffers(1, &vbo_pyramid_position);
		vbo_pyramid_position = 0;
	}

	if (vbo_pyramid_color)
	{
		glDeleteBuffers(1, &vbo_pyramid_color);
		vbo_pyramid_color = 0;
	}

	// deletion and uninitiaisaion of vao_pyramid

	if (vao_pyramid)
	{
		glDeleteVertexArrays(1, &vao_pyramid);
		vao_pyramid = 0;
	}

	if (vao_cube)
	{
		glDeleteVertexArrays(1, &vao_cube);
		vao_cube = 0;
	}

	// shader uninitialisation
	if (shaderProgramObject1)
	{
		glUseProgram(shaderProgramObject1);
		GLsizei numAttachedShaders;
		glGetProgramiv(shaderProgramObject1, GL_ATTACHED_SHADERS, &numAttachedShaders);
		GLuint *shaderObjects = NULL;

		// allocate enough memory to this buffer according to the number of attached shaders and fill it with the attached shader objects
		shaderObjects = (GLuint *)malloc(numAttachedShaders * sizeof(GLuint));

		glGetAttachedShaders(shaderProgramObject1, numAttachedShaders, &numAttachedShaders, shaderObjects);

		for (GLsizei i = 0; i < numAttachedShaders; i++)
		{
			glDetachShader(shaderProgramObject1, shaderObjects[i]);
			glDeleteShader(shaderObjects[i]);
			shaderObjects[i] = 0;
		}

		free(shaderObjects);
		shaderObjects = NULL;

		glUseProgram(0);
		glDeleteProgram(shaderProgramObject1);
		shaderProgramObject1 = 0;
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

	if (gpFile)
	{
		fprintf(gpFile, "Logfile Successfully Closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

// utilities

double deltaTimeCalculator(double deltaTime)
{
     // deltatime
    // Get the current time from the performance counter
    QueryPerformanceCounter(&currentTime);

    // Calculate the elapsed time since the last frame
    double elapsedTime = static_cast<double>(currentTime.QuadPart - previousTime.QuadPart) / frequency.QuadPart;

	// if (elapsedTime < 0.0001) 
	// {
    //     deltaTime = 0.0;
	// 	return deltaTime;
    // } 
	// else 
	// {
    //     // Save the current time for the next frame
    //     previousTime = currentTime;
    
    //     // Update deltaTime with the elapsed time
    //     deltaTime = elapsedTime;

	// 	return deltaTime;
    // }

    // Save the current time for the next frame
    // previousTime = currentTime;
    
    // float currentFrame = GetTickCount() / 1000.0;
    // deltaTime = currentFrame - lastFrame;
    // lastFrame = currentFrame;

    // deltaTime = elapsedTime;
    return elapsedTime;
}

vec3 mouseLook(float yaw, float pitch)
{
    vmath::vec3 direction;
    direction = vec3((cos(vmath::radians(yaw)) * cos(vmath::radians(pitch))), 
                    (sin(vmath::radians(pitch))),
                    (sin(vmath::radians(yaw)) * cos(vmath::radians(pitch))));

    // // direction.y = sin(vmath::radians(pitch));
    // // direction.z = sin(vmath::radians(yaw)) * cos(vmath::radians(pitch));
    return vmath::normalize(direction);
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

BOOL loadGLtexture(GLuint *texture, TCHAR imageResourceID[])
{
	// Variable declarations
	HBITMAP hbitmap=NULL;

	BITMAP bmp;
	BOOL bResult=FALSE;

	// code
	hbitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL), imageResourceID, IMAGE_BITMAP,0, 0, LR_CREATEDIBSECTION);

	if (hbitmap)
	{
		bResult = TRUE;
		GetObject(hbitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1); /// for better performance

		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//mipmapping

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// create the texture

		// gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits); // TARGET, MIPMAPLEVEL, opengl image format, image width, image height, border width, our image format, bytes
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		DeleteObject(hbitmap);
	}

	return bResult;
}









