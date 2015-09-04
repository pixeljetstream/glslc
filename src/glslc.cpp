/*

glslc
=============================================================================
Copyright: 2013-2015 Christoph Kubisch. 
http://glslc.luxinia.de

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

[ MIT license: http://www.opensource.org/licenses/mit-license.php ]

*/

#define GLSLC_VERSION 11

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <fstream>

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define WIN32_LEAN_AND_MEAN 1
#include <direct.h>
#include <windows.h>
#include <io.h>
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

typedef char GLchar;

#include <GL/gl.h>

#ifdef __linux__
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#endif

enum MGLERROR {
  MGL_NO_ERROR                       = GL_NO_ERROR,
  MGL_INVALID_ENUM                   = GL_INVALID_ENUM,
  MGL_INVALID_VALUE                  = GL_INVALID_VALUE,
  MGL_INVALID_OPERATION              = GL_INVALID_OPERATION,
  MGL_STACK_OVERFLOW                 = GL_STACK_OVERFLOW,
  MGL_STACK_UNDERFLOW                = GL_STACK_UNDERFLOW,
  MGL_OUT_OF_MEMORY                  = GL_OUT_OF_MEMORY,
};

#define GL_PROGRAM_SEPARABLE              0x8258

#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE
#define GL_PROGRAM_BINARY_FORMATS         0x87FF

#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_TESS_EVALUATION_SHADER         0x8E87
#define GL_TESS_CONTROL_SHADER            0x8E88
#define GL_COMPUTE_SHADER                 0x91B9

#define GL_SHADER_INCLUDE_ARB             0x8DAE
#define GL_NAMED_STRING_LENGTH_ARB        0x8DE9
#define GL_NAMED_STRING_TYPE_ARB          0x8DEA

typedef void (APIENTRYP PFNGLGETPROGRAMBINARYPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
typedef void (APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);

typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);

typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
#if defined(_WIN32)
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
#endif
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);

typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

typedef void (APIENTRYP PFNGLCOMPILESHADERINCLUDEARBPROC) (GLuint shader, GLsizei count, const GLchar* const *path, const GLint *length);
typedef void (APIENTRYP PFNGLDELETENAMEDSTRINGARBPROC) (GLint namelen, const GLchar* name);
typedef void (APIENTRYP PFNGLGETNAMEDSTRINGARBPROC) (GLint namelen, const GLchar* name, GLsizei bufSize, GLint *stringlen, GLchar *string);
typedef void (APIENTRYP PFNGLGETNAMEDSTRINGIVARBPROC) (GLint namelen, const GLchar* name, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISNAMEDSTRINGARBPROC) (GLint namelen, const GLchar* name);
typedef void (APIENTRYP PFNGLNAMEDSTRINGARBPROC) (GLenum type, GLint namelen, const GLchar* name, GLint stringlen, const GLchar *string);


PFNGLGETPROGRAMBINARYPROC PFNGLGETPROGRAMBINARYPROCvar;
PFNGLPROGRAMPARAMETERIPROC PFNGLPROGRAMPARAMETERIPROCvar;

PFNGLCREATEPROGRAMPROC PFNGLCREATEPROGRAMPROCvar;
PFNGLCREATESHADERPROC PFNGLCREATESHADERPROCvar;

PFNGLLINKPROGRAMPROC PFNGLLINKPROGRAMPROCvar;
PFNGLCOMPILESHADERPROC PFNGLCOMPILESHADERPROCvar;
PFNGLSHADERSOURCEPROC  PFNGLSHADERSOURCEPROCvar;
PFNGLATTACHSHADERPROC PFNGLATTACHSHADERPROCvar;

PFNGLGETPROGRAMIVPROC PFNGLGETPROGRAMIVPROCvar;
PFNGLGETSHADERIVPROC PFNGLGETSHADERIVPROCvar;
PFNGLGETPROGRAMINFOLOGPROC PFNGLGETPROGRAMINFOLOGPROCvar;
PFNGLGETSHADERINFOLOGPROC PFNGLGETSHADERINFOLOGPROCvar;

PFNGLCOMPILESHADERINCLUDEARBPROC PFNGLCOMPILESHADERINCLUDEARBPROCvar;
PFNGLDELETENAMEDSTRINGARBPROC PFNGLDELETENAMEDSTRINGARBPROCvar;
PFNGLGETNAMEDSTRINGARBPROC PFNGLGETNAMEDSTRINGARBPROCvar;
PFNGLGETNAMEDSTRINGIVARBPROC PFNGLGETNAMEDSTRINGIVARBPROCvar;
PFNGLISNAMEDSTRINGARBPROC PFNGLISNAMEDSTRINGARBPROCvar;
PFNGLNAMEDSTRINGARBPROC PFNGLNAMEDSTRINGARBPROCvar;

int SUPPORTS_SEPARATESHADERS = 0;
int SUPPORTS_SHADERINCLUDE   = 1;

void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary){
  PFNGLGETPROGRAMBINARYPROCvar(program,bufSize,length,binaryFormat,binary);
}

void glProgramParameteri(GLuint program, GLenum pname, GLint value){
  PFNGLPROGRAMPARAMETERIPROCvar(program,pname,value);
}

GLuint glCreateProgram()
{
  return PFNGLCREATEPROGRAMPROCvar();
}

GLuint glCreateShader(GLenum type)
{
  return PFNGLCREATESHADERPROCvar(type);
}

void glLinkProgram(GLuint program)
{
  PFNGLLINKPROGRAMPROCvar(program);
}

void glCompileShader(GLuint shader)
{
  PFNGLCOMPILESHADERPROCvar(shader);
}

void glShaderSource(GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length)
{
  PFNGLSHADERSOURCEPROCvar(shader,count,string,length);
}

void glAttachShader (GLuint program, GLuint shader)
{
  PFNGLATTACHSHADERPROCvar(program,shader);
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *params){
  PFNGLGETPROGRAMIVPROCvar(program,pname,params);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params){
  PFNGLGETSHADERIVPROCvar(shader,pname,params);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
  PFNGLGETPROGRAMINFOLOGPROCvar(program,bufSize,length,infoLog);
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
  PFNGLGETSHADERINFOLOGPROCvar(shader,bufSize,length,infoLog);
}

void glCompileShaderIncludeARB(GLuint shader,GLsizei count,const GLchar* const *path,const GLint *length){
  PFNGLCOMPILESHADERINCLUDEARBPROCvar(shader,count,path,length);
}

void glDeleteNamedStringARB(GLint namelen,const GLchar* name){
  PFNGLDELETENAMEDSTRINGARBPROCvar(namelen,name);
}

void glGetNamedStringARB(GLint namelen,const GLchar* name,GLsizei bufSize,GLint *stringlen,GLchar *string){
  PFNGLGETNAMEDSTRINGARBPROCvar(namelen,name,bufSize,stringlen,string);
}

void glGetNamedStringivARB(GLint namelen,const GLchar* name,GLenum pname,GLint *params){
  PFNGLGETNAMEDSTRINGIVARBPROCvar(namelen,name,pname,params);
}

GLboolean glIsNamedStringARB(GLint namelen,const GLchar* name){
  return PFNGLISNAMEDSTRINGARBPROCvar(namelen,name);
}

void glNamedStringARB(GLenum type,GLint namelen,const GLchar* name,GLint stringlen,const GLchar *string){
  PFNGLNAMEDSTRINGARBPROCvar(type,namelen,name,stringlen,string);
}

#ifdef _WIN32
#define INIT_GL_FUNC( type, name )   type##var = (type) wglGetProcAddress(name);  if (! type##var) notfound = true;
#elif __linux__
#define INIT_GL_FUNC( type, name )   type##var = (type) glXGetProcAddressARB((const GLubyte *)name);  if (! type##var) notfound = true;
#endif

bool initGL()
{
  bool notfound = false;

  SUPPORTS_SEPARATESHADERS = strstr((const char*)glGetString(GL_EXTENSIONS),"GL_ARB_separate_shader_objects") != NULL;
  SUPPORTS_SHADERINCLUDE = strstr((const char*)glGetString(GL_EXTENSIONS),"GL_ARB_shading_language_include") != NULL;

  INIT_GL_FUNC(PFNGLGETPROGRAMBINARYPROC,"glGetProgramBinary");
  INIT_GL_FUNC(PFNGLPROGRAMPARAMETERIPROC,"glProgramParameteri");

  INIT_GL_FUNC(PFNGLCOMPILESHADERINCLUDEARBPROC,"glCompileShaderIncludeARB");
  INIT_GL_FUNC(PFNGLDELETENAMEDSTRINGARBPROC,"glDeleteNamedStringARB");
  INIT_GL_FUNC(PFNGLGETNAMEDSTRINGARBPROC,"glGetNamedStringARB");
  INIT_GL_FUNC(PFNGLGETNAMEDSTRINGIVARBPROC,"glGetNamedStringivARB");
  INIT_GL_FUNC(PFNGLISNAMEDSTRINGARBPROC,"glIsNamedStringARB");
  INIT_GL_FUNC(PFNGLNAMEDSTRINGARBPROC,"glNamedStringARB");

  // mandatory
  notfound = false;

  INIT_GL_FUNC(PFNGLCREATEPROGRAMPROC,"glCreateProgram");
  INIT_GL_FUNC(PFNGLCREATESHADERPROC,"glCreateShader");

  INIT_GL_FUNC(PFNGLLINKPROGRAMPROC,"glLinkProgram");
  INIT_GL_FUNC(PFNGLSHADERSOURCEPROC,"glShaderSource");
  INIT_GL_FUNC(PFNGLCOMPILESHADERPROC,"glCompileShader");
  INIT_GL_FUNC(PFNGLATTACHSHADERPROC,"glAttachShader");

  INIT_GL_FUNC(PFNGLGETPROGRAMIVPROC,"glGetProgramiv");
  INIT_GL_FUNC(PFNGLGETSHADERIVPROC,"glGetShaderiv");
  INIT_GL_FUNC(PFNGLGETPROGRAMINFOLOGPROC,"glGetProgramInfoLog");
  INIT_GL_FUNC(PFNGLGETSHADERINFOLOGPROC,"glGetShaderInfoLog");

  MGLERROR error = (MGLERROR)glGetError();

  return error == MGL_NO_ERROR && !notfound;
}

#ifdef _WIN32

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  int x = LOWORD(lParam);
  int y = HIWORD(lParam);
  switch (msg)
  {
   case WM_KEYDOWN:
    {
      switch (wParam)
      {
      case VK_ESCAPE:
        PostQuitMessage(0);
        break;
      case VK_OEM_2: // Question Mark / Forward Slash for US Keyboards
        break;
      }
      break;
    }
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}


bool createContext()
{
  // based on "Pez" by Philip Rideout
  // http://prideout.net/blog/p36/pez.windows.c

  LPCSTR szName = "nvglcc";
  WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(0), 0, 0, 0, 0, szName, 0 };
  DWORD dwStyle = WS_DISABLED;
  DWORD dwExStyle = WS_EX_APPWINDOW;
  RECT rect;
  int windowWidth, windowHeight, windowLeft, windowTop;
  HWND hWnd;
  PIXELFORMATDESCRIPTOR pfd;
  HDC hDC;
  HGLRC hRC;
  int pixelFormat;
  DWORD previousTime = GetTickCount();
  MSG msg = {0};

  wc.hCursor = LoadCursor(0, IDC_ARROW);
  RegisterClassExA(&wc);

  SetRect(&rect, 0, 0, 32, 32);
  AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
  windowWidth = rect.right - rect.left;
  windowHeight = rect.bottom - rect.top;
  windowLeft = GetSystemMetrics(SM_CXSCREEN) / 2 - windowWidth / 2;
  windowTop = GetSystemMetrics(SM_CYSCREEN) / 2 - windowHeight / 2;
  hWnd = CreateWindowExA(0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, 0, 0, 0, 0);

  // Create the GL context.
  ZeroMemory(&pfd, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  hDC = GetDC(hWnd);
  pixelFormat = ChoosePixelFormat(hDC, &pfd);

  SetPixelFormat(hDC, pixelFormat, &pfd);
  hRC = wglCreateContext(hDC);
  wglMakeCurrent(hDC, hRC);
  
  return true;
}

#elif __linux__

static int const bufferAttributes[] =
{
  GLX_DOUBLEBUFFER, False,
  GLX_DRAWABLE_TYPE, GLX_PIXMAP_BIT,
  GLX_BIND_TO_TEXTURE_RGB_EXT, True,
  None
};

static int const pixmapAttributes[] =
{
  GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGB_EXT,
  GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
  None
};

static int const contextAttributes[] =
{
  GLX_RENDER_TYPE, GLX_RGBA_TYPE,
#ifndef NDEBUG
  GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
  None
};

bool createContext()
{
  Display *const pDisplay = XOpenDisplay(NULL);
  if (NULL == pDisplay)
  {
    printf( "Unable to open a connection to the X server\n" );
    return false;
  }
    
  // Request a frame-buffer config.
  int numFbConfigs;
  GLXFBConfig *const pFbConfigs = glXChooseFBConfig(
    pDisplay,
    DefaultScreen(pDisplay),
    bufferAttributes,
    &numFbConfigs);

  // Create a new GL context.
  GLXContext glContext = glXCreateContextAttribsARB(
    pDisplay,
    pFbConfigs[0],
    NULL,
    True,
    contextAttributes);

  // Create a pixmap.
  Pixmap xPixmap = XCreatePixmap(pDisplay, DefaultRootWindow(pDisplay), 128, 128, 24);
  GLXPixmap glPixmap = glXCreatePixmap(pDisplay, pFbConfigs[0], xPixmap, pixmapAttributes);

  // Make the context active.
  glXMakeCurrent(pDisplay, glPixmap, glContext);
  
  return true;
}

#endif

std::string readFile( const char* filename)
{
  std::string result;

  std::ifstream stream(filename, std::ios::in);
  if(!stream.is_open()){
    fprintf(stderr,"error: could not open input file \"%s\"\n",filename);
    exit(1);
    return result;
  }

  stream.seekg(0, std::ios::end);
  result.reserve(stream.tellg());
  stream.seekg(0, std::ios::beg);

  result.assign(
    (std::istreambuf_iterator<char>(stream)),
    std::istreambuf_iterator<char>());

  return result;
}

void printHelp()
{
  printf("glslc v%d\n",GLSLC_VERSION);
  printf("-----\n");
  printf("(c) 2013-2014 Christoph Kubisch: pixeljetstream@luxinia.de\n");
  printf("http://glslc.luxinia.de\n");
  printf("\n");
  printf("Basic offline compiler for GLSL\n");
  printf("Creates a dummy window and evokes the GL driver for compiling.\n");
  printf("Can dump pseudo assembly files for NVIDIA\n");
  printf("Basic #include handling independent of GL_ARB_shading_language_include\n");
  printf("\n");
  printf("Usage:\n");
  printf("\n");
  printf("glslc [options] filename\n");
  printf("Mandatory Options:\n");
  printf("  -profilename\n");
  printf("  -profile profilename\n");
  printf("       profilename can be: vertex, fragment, geometry,\n");
  printf("       tessevaluation, tesscontrol, compute\n");
  printf("       and affects subsequent filenames.\n");
  printf("       all files will be linked to a single program\n");
  printf("Other:\n");
  printf("  -E\n");
  printf("       pre-processes all shaders only. Dumps to stdout unless -P option is used\n");
  printf("  -P filename\n");
  printf("       Stores pre-processed file for next defined shader\n");
  printf("  -separable\n");
  printf("       enables separate shader objects usage (default false)\n");
  printf("  -o outputfilename\n");
  printf("       NVIDIA drivers can output pseudo assembly based on NV_program\n");
  printf("  -DMACRO[=VALUE]\n");
  printf("       prepends '#define MACRO VALUE' to all shaders\n");
  printf("       If VALUE is not specified it defaults to 1.\n");
  printf("  -SDMACRO[=VALUE]\n");
  printf("       prepends '#define MACRO VALUE' only to next shader\n");
  printf("       If VALUE is not specified it defaults to 1.\n");
  printf("  -glslversion \"version string\"\n");
  printf("       prepends version string prior defines, puts // in front of #version directives in shader file\n");
  printf("\n");
}


// manual include derived of Christoph Riccio's g_truc OpenGL SamplePack
// http://ogl-samples.g-truc.net/

std::string parseInclude(std::string const & Line, std::size_t const & Offset)
{
  std::string Result;

  std::string::size_type IncludeFirstQuote = Line.find("\"", Offset);
  std::string::size_type IncludeSecondQuote = Line.find("\"", IncludeFirstQuote + 1);

  return Line.substr(IncludeFirstQuote + 1, IncludeSecondQuote - IncludeFirstQuote - 1);
}

std::vector<std::string>  includeMarkers;

std::string eol("\n");

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
                  ( std::ostringstream() << std::dec << (x) ) ).str()

inline std::string includeMarker(const std::string & filename)
{
  std::string fixedname;
#ifdef _WIN32
  for (size_t i = 0; i < filename.size(); i++){
    char c = filename[i];
    if ( c == '\\'){
      fixedname.append("/");
    }
    else{
      fixedname.append(1,c);
    }
  }
#else
  fixedname = filename;
#endif
  if (!SUPPORTS_SHADERINCLUDE){
    // #line lineNumber markerNumber

    includeMarkers.push_back(fixedname);
    int slot = int( includeMarkers.size() );
    std::string slotstr = SSTR(slot);
    return slotstr;
  }
  else{
    return std::string(" \"") + fixedname + std::string("\" ");
  }
}

std::string manualInclude ( std::string const & filename,
                            std::string const & prepend,
                            std::string const & forcedVersion)
{
  std::string source = readFile(filename.c_str());
  if (source.empty())
    return source;

  std::stringstream stream;
  stream << source;
  std::string Line, Text;

  // Handle command line defines
  Text += prepend;
  Text += std::string("#line 1 ") + includeMarker(filename) + eol;
  int lineCount  = 0;
  while(std::getline(stream, Line))
  {
    std::size_t Offset = 0;
    lineCount++;

    // Version
    Offset = Line.find("#version");
    if(Offset != std::string::npos)
    {
      std::size_t CommentOffset = Line.find("//");
      if(CommentOffset != std::string::npos && CommentOffset < Offset)
        continue;

      // Reorder so that the #version line is always the first of a shader text
      Text = (forcedVersion.empty() ? Line : forcedVersion) + eol + Text + std::string("//") + Line + eol;
      continue;
    }

    // Include
    Offset = Line.find("#include");
    if(Offset != std::string::npos)
    {
      std::size_t CommentOffset = Line.find("//");
      if(CommentOffset != std::string::npos && CommentOffset < Offset)
        continue;

      std::string includeFile   = parseInclude(Line, Offset);
      std::string includeSource = manualInclude(includeFile,std::string(),std::string());
      if(!includeSource.empty())
      {
        // strip null terminator from content
        Text += includeSource;
        Text += eol + std::string("#line ") + SSTR(lineCount + 1) + std::string(" ") + includeMarker(filename) + eol;
      }

      continue;
    }

    Text += Line + eol;
  }

  return Text;
}

void printCorrectedLog(const std::string &log, const char *filename)
{
  std::string output = log;
#ifndef __linux__
  if (!SUPPORTS_SHADERINCLUDE){
    // (0) :
    std::tr1::regex patternFile("^\\(0\\)");
    output = std::tr1::regex_replace( output, patternFile, std::string(filename) );

    // 0(572) : 
    std::tr1::regex patternLine("^0(\\(\\d+\\) : )");
    output = std::tr1::regex_replace( output, patternLine, std::string(filename) + std::string("$1") );

    for (size_t i = 0; i < includeMarkers.size(); i++){
      std::string curfile = includeMarkers[i];
      int slot = int(i+1);

      // (0) :
      std::tr1::regex patternFile(std::string("^\\(") + SSTR(slot) + std::string("\\)"));
      output = std::tr1::regex_replace( output, patternFile, std::string(curfile) );

      // 0(572) : 
      std::tr1::regex patternLine(std::string("^") + SSTR(slot) + std::string("(\\(\\d+\\) : )"));
      output = std::tr1::regex_replace( output, patternLine, std::string(curfile) + std::string("$1") );
    }
  }
#endif
  printf("%s", output.c_str());
}


const char* shaderTypeName(GLenum type)
{
  switch(type){
    case GL_VERTEX_SHADER: return "Vertex";
    case GL_FRAGMENT_SHADER: return "Fragment";
    case GL_GEOMETRY_SHADER: return "Geometry";
    case GL_TESS_EVALUATION_SHADER: return "Tessellation evaluation";
    case GL_TESS_CONTROL_SHADER: return "Tessellation control";
    case GL_COMPUTE_SHADER: return "Compute";
  }
  return "";
}

static const size_t NPOS = std::string::npos;

size_t findShaderInLog(const std::string& log, size_t offset, GLenum& type)
{
  GLenum types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_COMPUTE_SHADER
  };

  size_t pos = NPOS;
  for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); i++){
    std::string search = std::string(shaderTypeName(types[i])) + std::string(" info");
    size_t searchpos   = log.find(search.c_str(), offset);
    if (searchpos < pos ){
      type = types[i];
      pos = searchpos;
    }
  }

  return pos;
}

struct ShaderInfo {
  GLenum        type;
  std::string   infile;
  std::string   preprocessfile;
  std::vector<std::string>  defines;
};

int main(int argc, char **argv)
{
  bool  useSeparate = false;
  bool  useOutfile  = false;
  bool  usePreprocessonly = false;

  const char* preprocessfilename = NULL;
  const char* outfilename   = NULL;
  const char* versionstring = NULL;
  std::string version;
  std::string alldefines;
  std::vector<ShaderInfo>   shaders;
  std::vector<std::string>  defines;
  std::vector<std::string>  shaderdefines;

  {
    GLenum shadertype = 0;
    const char* filename = NULL;

    for (int i = 1; i < argc; i++){
      if ( strcmp(argv[i],"-profile")==0 && i + 1 < argc) {
        if (strcmp(argv[i+1],"vertex")==0){
          shadertype = GL_VERTEX_SHADER;
        }
        else if (strcmp(argv[i+1],"fragment")==0){
          shadertype = GL_FRAGMENT_SHADER;
        }
        else if (strcmp(argv[i+1],"geometry")==0){
          shadertype = GL_GEOMETRY_SHADER;
        }
        else if (strcmp(argv[i+1],"tessevaluation")==0){
          shadertype = GL_TESS_EVALUATION_SHADER;
        }
        else if (strcmp(argv[i+1],"tesscontrol")==0){
          shadertype = GL_TESS_CONTROL_SHADER;
        }
        else if (strcmp(argv[i+1],"compute")==0){
          shadertype = GL_COMPUTE_SHADER;
        }
        else {
          fprintf(stderr,"error: illegal profile\n");
          exit(1);
        }
        i++;
      }
      else if (strcmp(argv[i],"-separable") == 0){
        useSeparate = true;
      }
      else if (strcmp(argv[i],"-vertex")==0){
        shadertype = GL_VERTEX_SHADER;
      }
      else if (strcmp(argv[i],"-fragment")==0){
        shadertype = GL_FRAGMENT_SHADER;
      }
      else if (strcmp(argv[i],"-geometry")==0){
        shadertype = GL_GEOMETRY_SHADER;
      }
      else if (strcmp(argv[i],"-tessevaluation")==0){
        shadertype = GL_TESS_EVALUATION_SHADER;
      }
      else if (strcmp(argv[i],"-tesscontrol")==0){
        shadertype = GL_TESS_CONTROL_SHADER;
      }
      else if (strcmp(argv[i],"-compute")==0){
        shadertype = GL_COMPUTE_SHADER;
      }
      else if (strstr(argv[i],"-D") == argv[i]){
        std::string def(argv[i]+2);
        if (def.find('=') != NPOS){
          def[def.find('=')] = ' ';
        }
        else {
          def += std::string(" 1");
        }

        defines.push_back(std::string("#define ") + def + eol);
      }
      else if (strstr(argv[i],"-SD") == argv[i]){
        std::string def(argv[i]+3);
        if (def.find('=') != NPOS){
          def[def.find('=')] = ' ';
        }
        else {
          def += std::string(" 1");
        }

        shaderdefines.push_back(std::string("#define ") + def + eol);
      }
      else if (strcmp(argv[i],"-glslversion")==0 && i + 1 < argc){
        versionstring = argv[i+1];
        i++;
      }
      else if (strcmp(argv[i],"-o")==0 && i + 1 < argc){
        outfilename = argv[i+1];
        i++;
      }
      else if (strcmp(argv[i],"-E")==0){
        usePreprocessonly = true;
      }
      else if (strcmp(argv[i],"-P")==0 && i + 1 < argc){
        preprocessfilename = argv[i+1];
        i++;
      }
      else {
        if (!shadertype){
          fprintf(stderr,"error: profile not provided\n");
          exit(1);
        }

        filename = argv[i];
        ShaderInfo info;
        info.infile  = std::string(filename);
        info.preprocessfile = preprocessfilename ? std::string(preprocessfilename) : std::string();
        info.type    = shadertype;
        info.defines = shaderdefines;
        shaders.push_back(info);
        shaderdefines.clear();
      }
    }
  }


  if (argc == 1){
    printHelp();
    return 0;
  }

  if (!shaders.size()){
    fprintf(stderr,"error: no filename(s) provided\n");
    return 1;
  }
  
  if (!usePreprocessonly && !createContext()){
    fprintf(stderr,"could not create GL context\n");
    return 1;
  }
  
  if (!usePreprocessonly && !initGL()) {
    fprintf(stderr,"could not initialize GL functions\n");
    return 1;
  }

  
  printf("glslc v%d\n",GLSLC_VERSION);

  GLuint program;
  if (!usePreprocessonly){
    printf("GL Vendor:   %s\n", (const char*)glGetString(GL_VENDOR));
    printf("GL Renderer: %s\n", (const char*)glGetString(GL_RENDERER));
    printf("GL Version:  %s\n", (const char*)glGetString(GL_VERSION));

    if (useSeparate && !SUPPORTS_SEPARATESHADERS) {
      printf("separable option not supported, disabled\n");
      useSeparate = true;
    }

    useOutfile = PFNGLGETPROGRAMBINARYPROCvar && outfilename && strstr((const char*)glGetString(GL_VENDOR), "NVIDIA");

    program = glCreateProgram();
    if (useSeparate) {
      glProgramParameteri(program,GL_PROGRAM_SEPARABLE,GL_TRUE);
    }
    if (useOutfile){
      glProgramParameteri(program,GL_PROGRAM_BINARY_RETRIEVABLE_HINT,GL_TRUE);
    } 
  }

  if (versionstring){
    version = std::string("#version ") + std::string(versionstring);
  }

  printf("\n");
  if (defines.size()){
    printf("Global defines\n");
    for (size_t i = 0; i < defines.size(); i++){
      printf("%s",defines[i].c_str());
      alldefines += defines[i];
    }
    printf("\n");
  }
  

  for (size_t i = 0; i < shaders.size(); i++){
    const ShaderInfo& info = shaders[i];
    const char* filename   = info.infile.c_str();

    std::string currentdefines;
    if (info.defines.size()){
      printf("Shader defines\n");
      for (size_t i = 0; i < info.defines.size(); i++){
        printf("%s",info.defines[i].c_str());
        currentdefines += info.defines[i];
      }
    }

    std::string shadertext = manualInclude(filename,alldefines + currentdefines,version);

    if (!info.preprocessfile.empty()){
      FILE* outfile = fopen(info.preprocessfile.c_str(),"wt");
      if (!outfile){
        fprintf(stderr,"error: could not create preprocess file, \"%s\"\n",info.preprocessfile.c_str());
        return 1;
      }
      else{
        printf("pre-processed written to \"%s\"\n",info.preprocessfile.c_str());
      }
      fwrite(&shadertext[0],shadertext.size(),1,outfile);
    }
    
    if (usePreprocessonly){
      if (info.preprocessfile.empty()){
        printf("pre-processed \"%s\"\n",filename);
        printf("%s\n\n",shadertext.c_str());
      }
      continue;
    }

    GLuint shader = glCreateShader(info.type);
    printf("compiling %s:\"%s\"...\n",shaderTypeName(info.type),filename);
 
    const char*  strings[1];
    GLint        lengths[1];

    strings[0] = shadertext.c_str();
    lengths[0] = (GLint)strlen(strings[0]);

    glShaderSource(shader,1,strings,lengths);
    glCompileShader(shader);

    GLint status = 0;
    GLint logLength = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1){
      std::string log;
      log.resize(logLength,0);
      glGetShaderInfoLog(shader, logLength,NULL,&log[0]);
      printCorrectedLog(log,filename);
      printf("\n");
    }

    if (!status){
      fprintf(stderr,"error: shader could not be compiled\n");
      return 1;
    }

    printf("success\n\n");
    glAttachShader(program,shader);
  }

  if (usePreprocessonly){
    return 0;
  }

  glLinkProgram(program);

  {
    GLint status = 0;
    GLint logLength = 0;

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1){
      std::string log;
      log.resize(logLength,0);
      glGetProgramInfoLog(program, logLength,NULL,&log[0]);

      GLenum type;
      size_t infobegin = findShaderInLog(log,0,type);
      size_t infoend   = NPOS;
      if (infobegin != NPOS){
        printf("%s",log.substr(0,infobegin).c_str());
        while (infobegin != NPOS){
          GLenum typenext;
          infoend = findShaderInLog(log,infobegin+1,typenext);
          std::string sublog = log.substr(infobegin,infoend);
          const char* filename = "";
          for (size_t i = 0; i < shaders.size(); i++){
            if (shaders[i].type == type){
              filename = shaders[i].infile.c_str();
            }
          }

          printCorrectedLog(sublog,filename);
          infobegin = infoend;
          type      = typenext;
        }

      }
      else{
        printf("%s",log.c_str());
      }
      
      printf("\n");
    }

    if (!status){
      fprintf(stderr,"error: program could not be linked\n");
      return 1;
    }

    printf("successfully linked\n");
  }
  
  if (useOutfile){
    GLsizei binaryLength = 0;
    GLenum format = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH,&binaryLength);

    if (!binaryLength){
      fprintf(stderr,"error: could not retrieve program binary\n");
      return 1;
    }

    std::string binary(size_t(binaryLength+1),0);
    glGetProgramBinary(program,binaryLength,NULL,&format,&binary[0]);

    size_t startpos = binary.find("!!NV",0);
    size_t endpos   = binary.rfind("END");
  
    if (startpos == NPOS || endpos == NPOS){
      fprintf(stderr,"error: cannot find NV_gpu_program strings\n");
      return 1;
    }

    FILE* outfile = fopen(outfilename,"wb");
    if (!outfile){
      fprintf(stderr,"error: could not create output file, \"%s\"\n",outfilename);
      return 1;
    }

    while (startpos != NPOS && endpos != NPOS){
      size_t temppos = binary.find("!!NV", startpos+1);
      if (temppos != NPOS){
        endpos   = binary.rfind("END", temppos);
      }

      std::string shaderoutput = binary.substr(startpos,endpos + 3 - startpos);
      fprintf(outfile,"%s",shaderoutput.c_str());
      fprintf(outfile,"\n\n");

      startpos = binary.find("!!NV", endpos + 3);
      endpos   = binary.rfind("END");
    }
    printf("NV_gpu_program written to:\n%s\n",outfilename);
    fclose(outfile);
  }

  return 0;
}
