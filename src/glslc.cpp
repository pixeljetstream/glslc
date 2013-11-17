/*

glslc
=============================================================================
Copyright: 2013 Christoph Kubisch. 
http://pixeljetstream.luxinia.de

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

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <regex>

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

#include <GL/GL.h>

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
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
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
int SUPPORTS_SHADERINCLUDE = 0;

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


enum MGLERROR {
  MGL_NO_ERROR                       = 0,
  MGL_INVALID_ENUM                   = 0x0500,
  MGL_INVALID_VALUE                  = 0x0501,
  MGL_INVALID_OPERATION              = 0x0502,
  MGL_STACK_OVERFLOW                 = 0x0503,
  MGL_STACK_UNDERFLOW                = 0x0504,
  MGL_OUT_OF_MEMORY                  = 0x0505,
};


std::string readFile(const char* filename){
  std::string content;

  size_t filesize;
  FILE* infile = fopen(filename,"rb");

  if (!infile){
    fprintf(stderr,"error: could not open input file \"%s\"\n",filename);
    exit(1);
  }

  fseek (infile, 0, SEEK_END);   // non-portable
  filesize=ftell (infile);
  fseek (infile, 0, SEEK_SET);

  content.resize(filesize+1);
  fread(&content[0],filesize,1,infile);
  content[filesize] = 0;

  fclose (infile);

  return content;
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

  bool notfound = false;

#define INIT_GL_FUNC( type, name )   type##var = (type) wglGetProcAddress(name);  if (! type##var) notfound = true;

  // optional
  INIT_GL_FUNC(PFNGLGETPROGRAMBINARYPROC,"glGetProgramBinary");
  SUPPORTS_SEPARATESHADERS = strstr((const char*)glGetString(GL_EXTENSIONS),"GL_ARB_separate_shader_objects") != NULL;
  SUPPORTS_SHADERINCLUDE = strstr((const char*)glGetString(GL_EXTENSIONS),"GL_ARB_shading_language_include") != NULL;

  // mandatory
  INIT_GL_FUNC(PFNGLPROGRAMPARAMETERIPROC,"glProgramParameteri");

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

  INIT_GL_FUNC(PFNGLCOMPILESHADERINCLUDEARBPROC,"glCompileShaderIncludeARB");
  INIT_GL_FUNC(PFNGLDELETENAMEDSTRINGARBPROC,"glDeleteNamedStringARB");
  INIT_GL_FUNC(PFNGLGETNAMEDSTRINGARBPROC,"glGetNamedStringARB");
  INIT_GL_FUNC(PFNGLGETNAMEDSTRINGIVARBPROC,"glGetNamedStringivARB");
  INIT_GL_FUNC(PFNGLISNAMEDSTRINGARBPROC,"glIsNamedStringARB");
  INIT_GL_FUNC(PFNGLNAMEDSTRINGARBPROC,"glNamedStringARB");

  MGLERROR error = (MGLERROR)glGetError();

  return error != MGL_NO_ERROR || notfound;
}

void handleInclude(std::string pattern)
{
  struct _finddata_t c_file;
  intptr_t hFile;
  std::string path;
  size_t pos = pattern.find_last_of('/');
  if (pos != std::string::npos){
    path = pattern.substr(0,pos+1);
  }

  if ((hFile = _findfirst (pattern.c_str(), &c_file)) == -1L) {
    return;
  }

  do {
    std::string filepath = (path + std::string(c_file.name));
    std::string content = readFile(filepath.c_str() );
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, -1, (std::string("/") + filepath).c_str(), -1, content.c_str()) ;
  } while(_findnext (hFile, &c_file) != -1L);
  _findclose (hFile);
}


#endif

void removeVersion(std::string &shader)
{
  std::tr1::regex pattern("#version");

  shader = std::tr1::regex_replace( shader, pattern, std::string("//#version"));
}

void printCorrectedLog(std::string &log, const char *filename)
{
  // 0(572) : 
  // (0) :

  std::tr1::regex patternLine("0(\\(\\d+\\) : )");

  std::string output = std::tr1::regex_replace( log, patternLine, std::string(filename) + std::string("$1") );

  std::tr1::regex patternFile("\\(0\\)");

  output = std::tr1::regex_replace( output, patternFile, std::string(filename) );

  printf(output.c_str());
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

static const size_t INVALID = std::string::npos;

size_t findShaderLogInfo(const std::string& log, size_t offset, GLenum& type)
{
  GLenum types[] = {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_COMPUTE_SHADER
  };

  size_t pos = INVALID;
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
};

int main(int argc, char **argv)
{
  const char* outfilename   = NULL;
  const char* versionstring = NULL;
  std::vector<ShaderInfo>   shaders;
  std::vector<std::string>  defines;
  std::vector<std::string>  includes;
  
  
  bool allowSeparate = false;

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
        allowSeparate = true;
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
        if (def.find('=') != std::string::npos){
          def[def.find('=')] = ' ';
        }
        else {
          def += std::string(" 1");
        }

        defines.push_back(std::string("#define ") + def + "\n");
      }
      else if (strstr(argv[i],"-I") == argv[i]){
        std::string pattern(argv[i]+2);
        includes.push_back(pattern);
      }
      else if (strcmp(argv[i],"-glslversion")==0 && i + 1 < argc){
        versionstring = argv[i+1];
        i++;
      }
      else if (strcmp(argv[i],"-o")==0 && i + 1 < argc){
        outfilename = argv[i+1];
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
        info.type    = shadertype;
        shaders.push_back(info);
      }
    }
  }


  if (argc == 1){
    printf("glslc\n");
    printf("-----\n");
    printf("(c) 2013 Christoph Kubisch: pixeljetstream@luxinia.de\n");
    printf("http://github.com/CrazyButcher/glslc\n");
    printf("\n");
    printf("Basic offline compiler for GLSL\n");
    printf("Creates a dummy window and evokes the GL driver for compiling.\n");
    printf("Can dump pseudo assembly files for NVIDIA\n");
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
    printf("  -separable\n");
    printf("       enables separate shader objects usage (default false)\n");
    printf("  -o outputfilename\n");
    printf("       NVIDIA drivers can output pseudo assembly based on NV_program\n");
    printf("  -DMACRO[=VALUE]\n");
    printf("       prepends '#define MACRO VALUE' to shader\n");
    printf("       If VALUE is not specified it defaults to 1.\n");
    printf("  -IPATTERN\n");
    printf("       uses PATTERN to find files for includes\n");
    printf("       requires GL_ARB_shading_language_include\n");
    printf("  -glslversion \"version string\"\n");
    printf("       prepends version string prior defines, puts // in front of #version directives in shader file\n");
    printf("\n");
    return 0;
  }

  if (!shaders.size()){
    fprintf(stderr,"error: no filename(s) provided\n");
    exit(1);
  }
  
  if (createContext()){
    fprintf(stderr,"could not create GL context\n");
    exit(1);
  }

  printf("glslc\n");
  printf("GL Vendor:   %s\n",glGetString(GL_VENDOR));
  printf("GL Renderer: %s\n",glGetString(GL_RENDERER));
  printf("GL Version:  %s\n",glGetString(GL_VERSION));


  if (allowSeparate && !SUPPORTS_SEPARATESHADERS) {
    printf("separable option not supported, disabled\n");
  }

  GLuint program = glCreateProgram();
  if (allowSeparate && SUPPORTS_SEPARATESHADERS) {
    glProgramParameteri(program,GL_PROGRAM_SEPARABLE,GL_TRUE);
  }
  if (PFNGLGETPROGRAMBINARYPROCvar){
    glProgramParameteri(program,GL_PROGRAM_BINARY_RETRIEVABLE_HINT,GL_TRUE);
  } 
  
  if (!SUPPORTS_SHADERINCLUDE && includes.size()){
    printf("GL_ARB_shading_language_include is not supported, includes will be ignored\n");
  }
  else if (includes.size()){
    for (size_t i = 0; i < includes.size(); i++){
      handleInclude(includes[i]);
    }
  }

  printf("\n");
  std::string version("");
  if (versionstring){
    version = std::string("#version ") + std::string(versionstring) + std::string("\n");
  }
  std::string alldefines("");
  for (size_t i = 0; i < defines.size(); i++){
    printf(defines[i].c_str());
    alldefines += defines[i];
  }

  for (size_t i = 0; i < shaders.size(); i++){
    const ShaderInfo& info = shaders[i];
    const char* filename   = info.infile.c_str();
    std::string shadertext = readFile(filename);

    GLuint shader = glCreateShader(info.type);
    printf("compiling %s:\"%s\"...\n",shaderTypeName(info.type),filename);
 
    const char*  strings[1];
    GLint        lengths[1];

    if (versionstring){
      removeVersion(shadertext);
    }

    shadertext = version + alldefines  + std::string("#line 0\n") + shadertext;

    strings[0] = shadertext.c_str();
    lengths[0] = (GLint)strlen(strings[0]);

    glShaderSource(shader,1,strings,lengths);

    if (SUPPORTS_SHADERINCLUDE){
      const char* paths = "/";
      glCompileShaderIncludeARB(shader,1,&paths,NULL);
    }
    else{
      glCompileShader(shader);
    }

    GLint status = 0;
    GLint logLength = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1){
      std::string buffer;
      buffer.resize(logLength,0);
      glGetShaderInfoLog(shader, logLength,NULL,&buffer[0]);
      printCorrectedLog(buffer,filename);
      printf("\n");
      if (!status) return 1;
    }

    printf("success\n\n");
    glAttachShader(program,shader);
  }

  glLinkProgram(program);

  {
    GLint status = 0;
    GLint logLength = 0;

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1){
      std::string buffer;
      buffer.resize(logLength,0);
      glGetProgramInfoLog(program, logLength,NULL,&buffer[0]);

      GLenum type;
      size_t infobegin = findShaderLogInfo(buffer,0,type);
      size_t infoend   = INVALID;
      if (infobegin != INVALID){
        printf(buffer.substr(0,infobegin).c_str());
        while (infobegin != INVALID){
          GLenum typenext;
          infoend = findShaderLogInfo(buffer,infobegin+1,typenext);
          std::string sublog = buffer.substr(infobegin,infoend);
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
        printf(buffer.c_str());
      }
      
      printf("\n");
      if (!status) return 1;
    }

    printf("successfully linked\n");
  }
  
  if (PFNGLGETPROGRAMBINARYPROCvar && outfilename && strstr((const char*)glGetString(GL_VENDOR), "NVIDIA")){
    GLsizei binaryLength = 0;
    GLenum format = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH,&binaryLength);
    std::string binary(size_t(binaryLength+1),0);
    glGetProgramBinary(program,binaryLength,NULL,&format,&binary[0]);

    size_t startpos = binary.find("!!NV",0);
    size_t endpos   = binary.rfind("END");
  
    if (startpos == INVALID || endpos == INVALID)
      return 1;

    FILE* outfile = fopen(outfilename,"wb");
    if (!outfile){
      fprintf(stderr,"error: could not create output file, \"%s\"\n",outfilename);
      return 1;
    }

    while (startpos != INVALID && endpos != INVALID){
      size_t temppos = binary.find("!!NV", startpos+1);
      if (temppos != INVALID){
        endpos   = binary.rfind("END", temppos);
      }

      std::string shaderoutput = binary.substr(startpos,endpos + 3 - startpos);
      fprintf(outfile,shaderoutput.c_str());
      fprintf(outfile,"\n\n");

      startpos = binary.find("!!NV", endpos + 3);
      endpos   = binary.rfind("END");
    }
    printf("NV_program written to:\n%s\n",outfilename);
    fclose(outfile);
  }

  return 0;
}


