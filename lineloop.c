// NOTE: This program demonstrates the GL_LINE_LOOP bug in etnaviv on the GCW Zero.
//       The bug is that drawing anything with a GL_LINE_LOOP seems to freeze all 
//       rendering once eglSwapBuffers() is called. Drawing things with
//       GL_LINE_STRIP works fine, however.  I have only tested with GLES 1.1,
//       as I am still learning GLES 2.0 and don't know anything about it yet.
//       ---->  Relevant lines to check are 384-390. <----
//       My email address is dansilsby <AT> gmail <DOT> com
//       My nick on IRC is senquack on freenode IRC #gcw and #gcw-dev
//       

#define USE_GLES1

#include <SDL/SDL.h>

#include <GLES/gl.h>
#include <GLES/egl.h>

#include <stdio.h>
#include <stdint.h>

#define SCREEN_W     320
#define SCREEN_H     240
#define SCREEN_BPP   16
#define SCREEN_FLAGS SDL_FULLSCREEN

typedef struct eglConnection {
  EGLDisplay display;
  EGLContext context;
  EGLSurface surface;
  NativeWindowType window;
} eglConnection;

eglConnection egl_screen = {
  EGL_NO_DISPLAY,
  EGL_NO_CONTEXT,
  EGL_NO_SURFACE,
  (NativeWindowType)0
};

#define totalConfigsIn 5 /** Total number of configurations to request */
EGLint totalConfigsFound = 0; /** Total number of configurations matching attributes */
EGLConfig eglConfigs[totalConfigsIn]; /** Structure containing references to matching configurations */

// These settings are correct for the GCW Zero
const EGLint egl_config_attr[] = {
  EGL_BUFFER_SIZE,           SCREEN_BPP,
  EGL_DEPTH_SIZE,            16,
  EGL_SURFACE_TYPE,          EGL_WINDOW_BIT,
  EGL_STENCIL_SIZE,          0,
  EGL_NONE  
};

enum EGL_SETTINGS_T {
   CFG_RED_SIZE, /** Number of bits of Red in the color buffer. */
   CFG_GREEN_SIZE, /** Number of bits of Green in the color buffer. */
   CFG_BLUE_SIZE, /** Number of bits of Blue in the color buffer. */
   CFG_ALPHA_SIZE, /** Number of bits of Alpha in the color buffer. */
   CFG_DEPTH_SIZE, /** Number of bits of Z in the depth buffer. */
   CFG_BUFFER_SIZE, /** The total color component bits in the color buffer. */
   CFG_STENCIL_SIZE, /** Number of bits of Stencil in the stencil buffer. */
   CFG_TOTAL /** Total number of settings. */
};
EGLint eglSettings[CFG_TOTAL]; /** Stores setting values. */

/** @brief Error checking function
 * @param file : string reference that contains the source file that the check is occuring in
 * @param line : numeric reference that contains the line number that the check is occuring in
 * @return : 0 if the function passed, else 1
 */
int8_t CheckEGLErrors( const char* file, uint16_t line )
{
   EGLenum error;
   const char* errortext;
   const char* description;
   error = eglGetError();
   if (error != EGL_SUCCESS && error != 0)
   {
      switch (error)
      {
         case EGL_NOT_INITIALIZED:
            errortext = "EGL_NOT_INITIALIZED.";
            description = "EGL is not or could not be initialized, for the specified display.";
            break;
         case EGL_BAD_ACCESS:
            errortext = "EGL_BAD_ACCESS EGL";
            description = "cannot access a requested resource (for example, a context is bound in another thread).";
            break;
         case EGL_BAD_ALLOC:
            errortext = "EGL_BAD_ALLOC EGL";
            description = "failed to allocate resources for the requested operation.";
            break;
         case EGL_BAD_ATTRIBUTE:
            errortext = "EGL_BAD_ATTRIBUTE";
            description = "An unrecognized attribute or attribute value was passed in anattribute list.";
            break;
         case EGL_BAD_CONFIG:
            errortext = "EGL_BAD_CONFIG";
            description = "An EGLConfig argument does not name a valid EGLConfig.";
            break;
         case EGL_BAD_CONTEXT:
            errortext = "EGL_BAD_CONTEXT";
            description = "An EGLContext argument does not name a valid EGLContext.";
            break;
         case EGL_BAD_CURRENT_SURFACE:
            errortext = "EGL_BAD_CURRENT_SURFACE";
            description = "The current surface of the calling thread is a window, pbuffer,or pixmap that is no longer valid.";
            break;
         case EGL_BAD_DISPLAY:
            errortext = "EGL_BAD_DISPLAY";
            description = "An EGLDisplay argument does not name a valid EGLDisplay.";
            break;
         case EGL_BAD_MATCH:
            errortext = "EGL_BAD_MATCH";
            description = "Arguments are inconsistent; for example, an otherwise valid context requires buffers (e.g. depth or stencil) not allocated by an otherwise valid surface.";
            break;
         case EGL_BAD_NATIVE_PIXMAP:
            errortext = "EGL_BAD_NATIVE_PIXMAP";
            description = "An EGLNativePixmapType argument does not refer to a validnative pixmap.";
            break;
         case EGL_BAD_NATIVE_WINDOW:
            errortext = "EGL_BAD_NATIVE_WINDOW";
            description = "An EGLNativeWindowType argument does not refer to a validnative window.";
            break;
         case EGL_BAD_PARAMETER:
            errortext = "EGL_BAD_PARAMETER";
            description = "One or more argument values are invalid.";
            break;
         case EGL_BAD_SURFACE:
            errortext = "EGL_BAD_SURFACE";
            description = "An EGLSurface argument does not name a valid surface (window,pbuffer, or pixmap) configured for rendering";
            break;
         case EGL_CONTEXT_LOST:
            errortext = "EGL_CONTEXT_LOST";
            description = "A power management event has occurred. The application mustdestroy all contexts and reinitialise client API state and objects to continue rendering.";
            break;
         default:
            errortext = "Unknown EGL Error";
            description = "";
            break;
      }
      printf( "EGLport ERROR: EGL Error detected in file %s at line %d: %s (0x%X)\n Description: %s\n", file, line, errortext, error, description );
      return 1;
   }
   return 0;
}

/** @brief Find a EGL configuration tht matches the defined attributes
 * @return : 0 if the function passed, else 1
 */
int8_t FindEGLConfigs( void )
{
   EGLBoolean result;
   int attrib = 0;
   EGLint ConfigAttribs[23];
   ConfigAttribs[attrib++] = EGL_RED_SIZE; /* 1 */
   ConfigAttribs[attrib++] = eglSettings[CFG_RED_SIZE]; /* 2 */
   ConfigAttribs[attrib++] = EGL_GREEN_SIZE; /* 3 */
   ConfigAttribs[attrib++] = eglSettings[CFG_GREEN_SIZE]; /* 4 */
   ConfigAttribs[attrib++] = EGL_BLUE_SIZE; /* 5 */
   ConfigAttribs[attrib++] = eglSettings[CFG_BLUE_SIZE]; /* 6 */
   ConfigAttribs[attrib++] = EGL_ALPHA_SIZE; /* 7 */
   ConfigAttribs[attrib++] = eglSettings[CFG_ALPHA_SIZE]; /* 8 */
   ConfigAttribs[attrib++] = EGL_DEPTH_SIZE; /* 9 */
   ConfigAttribs[attrib++] = eglSettings[CFG_DEPTH_SIZE]; /* 10 */
   ConfigAttribs[attrib++] = EGL_BUFFER_SIZE; /* 11 */
   ConfigAttribs[attrib++] = eglSettings[CFG_BUFFER_SIZE]; /* 12 */
   ConfigAttribs[attrib++] = EGL_STENCIL_SIZE; /* 13 */
   ConfigAttribs[attrib++] = eglSettings[CFG_STENCIL_SIZE]; /* 14 */
   ConfigAttribs[attrib++] = EGL_SURFACE_TYPE; /* 15 */
   ConfigAttribs[attrib++] = EGL_WINDOW_BIT; /* 16 */
#if defined(EGL_VERSION_1_2)
   ConfigAttribs[attrib++] = EGL_RENDERABLE_TYPE; /* 17 */
#if defined(USE_GLES1)
   ConfigAttribs[attrib++] = EGL_OPENGL_ES_BIT;
#elif defined(USE_GLES2)
   ConfigAttribs[attrib++] = EGL_OPENGL_ES2_BIT; /* 18 */
#endif /* USE_GLES1 */
#endif /* EGL_VERSION_1_2 */
   ConfigAttribs[attrib++] = EGL_NONE; /* 23 */
   result = eglChooseConfig( egl_screen.display, ConfigAttribs, eglConfigs, totalConfigsIn, &totalConfigsFound );
   if (result != EGL_TRUE || totalConfigsFound == 0)
   {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to query for available configs, found %d.\n", totalConfigsFound );
      return 1;
   }
   printf( "EGLport: Found %d available configs\n", totalConfigsFound );
   return 0;
}

//LINE_STRIP does work:
static void renderSquare_linestrip()
{
   static GLubyte color[5][4] = {
      {255,255,255,255},
      {255,255,255,255},
      {255,255,255,255},
      {255,255,255,255},
      {255,255,255,255}
   };

   static GLfloat square[5][3] = {
      {-0.5f,-0.5f, 0.0f},
      {-0.5f, 0.5f, 0.0f},
      { 0.5f, 0.5f, 0.0f},
      { 0.5f,-0.5f, 0.0f},
      {-0.5f,-0.5f, 0.0f}     // Last vertex is repeat of first vertex to complete drawing the square
   };

   glClearColor(0.0, 0.0, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
   glEnableClientState(GL_COLOR_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, square);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDrawArrays(GL_LINE_STRIP, 0, 5);
}

//LINE_LOOP does not work.. freezes rendering when glSwapBuffers is called
static void renderSquare_lineloop()
{
   static GLubyte color[4][4] = {
      {255,255,255,255},
      {255,255,255,255},
      {255,255,255,255},
      {255,255,255,255}
   };

   static GLfloat square[4][3] = {
      {-0.5f,-0.5f, 0.0f},
      {-0.5f, 0.5f, 0.0f},
      { 0.5f, 0.5f, 0.0f},
      { 0.5f,-0.5f, 0.0f}     // Line Loop should draw final line in the square for us 
   };

   glClearColor(0.0, 0.0, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glColorPointer(4, GL_UNSIGNED_BYTE, 0, color);
   glEnableClientState(GL_COLOR_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, square);
   glEnableClientState(GL_VERTEX_ARRAY);
   glDrawArrays(GL_LINE_LOOP, 0, 4);
}

   
int main()
{
   printf("Using SDL to set video mode to %dx%d %dbpp.\n", SCREEN_W, SCREEN_H, SCREEN_BPP);
   SDL_Surface *screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, SCREEN_BPP, SCREEN_FLAGS);
   if ( !screen ) {
      printf("Unable to create SDL OpenGL screen: %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   } 

   SDL_ShowCursor(SDL_DISABLE);

   EGLConfig e_config;
   uint32_t configIndex = 0;
   EGLint nc;
   EGLBoolean result;

   const char* output;
   EGLint eglMajorVer, eglMinorVer;

#if SCREEN_BPP > 16
   eglSettings[CFG_RED_SIZE]     = 8;
   eglSettings[CFG_BLUE_SIZE]    = 8;
   eglSettings[CFG_GREEN_SIZE]   = 8;
   eglSettings[CFG_ALPHA_SIZE]   = 8;
#else
   eglSettings[CFG_RED_SIZE]     = 5;
   eglSettings[CFG_GREEN_SIZE]   = 6;
   eglSettings[CFG_BLUE_SIZE]    = 5;
   eglSettings[CFG_ALPHA_SIZE]   = 0;
#endif
   eglSettings[CFG_BUFFER_SIZE]  = SCREEN_BPP;
   eglSettings[CFG_DEPTH_SIZE]   = 16;
   eglSettings[CFG_STENCIL_SIZE] = 0;

   printf( "EGLport: Using Config %d\n", configIndex );

   printf( "EGLport: Opening EGL display\n" );
   egl_screen.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (egl_screen.display == EGL_NO_DISPLAY)
   {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to create EGL display.\n" );
      return 1;
   }

   printf( "EGLport: Initializing\n" );
   result = eglInitialize(egl_screen.display, &eglMajorVer, &eglMinorVer );
   if (result != EGL_TRUE )
   {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to initialize EGL display.\n" );
      return 1;
   }

   /* Get EGL Library Information */
   printf( "EGL Implementation Version: Major %d Minor %d\n", eglMajorVer, eglMinorVer );
   output = eglQueryString( egl_screen.display, EGL_VENDOR );
   printf( "EGL_VENDOR: %s\n", output );
   output = eglQueryString( egl_screen.display, EGL_VERSION );
   printf( "EGL_VERSION: %s\n", output );
   output = eglQueryString( egl_screen.display, EGL_EXTENSIONS );
   printf( "EGL_EXTENSIONS: %s\n", output );

   
   if (FindEGLConfigs() != 0) {
      printf( "EGLport ERROR: Unable to configure EGL. See previous error.\n" );
      return 1;
   }
   printf( "EGLport: Using Config %d\n", configIndex );
   eglChooseConfig(egl_screen.display, egl_config_attr, &e_config, 1, &nc);

   printf( "EGLport: Creating Context\n" );
   egl_screen.context = eglCreateContext(egl_screen.display, eglConfigs[configIndex], EGL_NO_CONTEXT, NULL);
   if (egl_screen.context == EGL_NO_CONTEXT) {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to create GLES context!\n");
      return 1;
   }

   // open raw fbdev window:
   printf( "EGLport: Creating window surface\n" );
   egl_screen.window = 0;           
   egl_screen.surface = eglCreateWindowSurface(egl_screen.display, eglConfigs[configIndex], egl_screen.window, NULL);
   if (egl_screen.surface == EGL_NO_SURFACE) {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to create EGL surface!\n" );
      return 1;
   }

   printf( "EGLport: Making Current\n" );
   result = eglMakeCurrent(egl_screen.display, egl_screen.surface, egl_screen.surface, egl_screen.context);
   if (result != EGL_TRUE) {
      CheckEGLErrors( __FILE__, __LINE__ );
      printf( "EGLport ERROR: Unable to make GLES context current\n" );
      return 1;
   }

   printf( "EGLport: Setting swap interval\n" );
   eglSwapInterval(egl_screen.display, 1);

   printf( "EGLport: Complete\n" );
   CheckEGLErrors( __FILE__, __LINE__ );


   //senquack - for OpenGLES, we enable these once and leave them enabled:
   glEnableClientState (GL_VERTEX_ARRAY);
   glEnableClientState (GL_COLOR_ARRAY);

   glViewport (0, 0, SCREEN_W, SCREEN_H);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   glOrthof(-((float)SCREEN_W/(float)SCREEN_H), ((float)SCREEN_W/(float)SCREEN_H), -1.0 , 1.0 , -20.0, 20.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
   glShadeModel(GL_SMOOTH);

   // Main loop:
   int quit = 0;
   SDL_Event event;
   while (!quit ) {
      // Quit if button/key pressed:
      while(SDL_PollEvent(&event)){
         switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
               quit = 1;
               break;
            default:
               break;

         }
      }

      // UNCOMMENT ONE OF THESE TWO LINES:
//      renderSquare_lineloop();       // Does not work
      renderSquare_linestrip();        // Does work

      eglSwapBuffers(egl_screen.display, egl_screen.surface);
   }  
   // End main loop

   // Quit GLES and SDL:
   eglMakeCurrent(egl_screen.display, NULL, NULL, EGL_NO_CONTEXT);
   eglDestroySurface(egl_screen.display, egl_screen.surface);
   eglDestroyContext(egl_screen.display, egl_screen.context);
   egl_screen.surface = 0;
   egl_screen.context = 0;
   e_config = 0;
   eglTerminate(egl_screen.display);
   egl_screen.display = 0;
   SDL_Quit();
   return 0;
}
