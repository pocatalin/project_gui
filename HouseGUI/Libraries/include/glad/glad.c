#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>

#ifdef _WIN32
    #include <windows.h>
    static HMODULE libGL;
#else
    #include <dlfcn.h>
    static void* libGL;
#endif

#ifdef _WIN32
    static void* get_proc_address(const char* proc) {
        void* result = (void*)GetProcAddress(libGL, proc);
        if (!result) {
            result = (void*)wglGetProcAddress(proc);
        }
        return result;
    }
#else
    static void* get_proc_address(const char* proc) {
        void* result = dlsym(libGL, proc);
        if (!result) {
            result = (void*)glXGetProcAddress((const GLubyte*)proc);
        }
        return result;
    }
#endif

int gladLoadGLLoader(GLADloadproc load) {
#ifdef _WIN32
    libGL = LoadLibraryA("opengl32.dll");
#else
    libGL = dlopen("libGL.so.1", RTLD_NOW | RTLD_GLOBAL);
#endif

    if(libGL != NULL) {
        load = (GLADloadproc)get_proc_address;
    }

    if(load == NULL) {
#ifdef _WIN32
        FreeLibrary(libGL);
#else
        dlclose(libGL);
#endif
        return 0;
    }

    gladLoadGL();
    return 1;
} 