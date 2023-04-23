#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "wrapper.h"
#include "debug.h"
#include "bridge.h"
#include "librarian/library_private.h"
#include "x86emu.h"
#include "callback.h"
#include "librarian.h"
#include "emu/x86emu_private.h"
#include "box86context.h"
#include "sdl1rwops.h"
#include "gltools.h"

#include "x86trace.h"
#include "threads.h"

const char* sdl1Name = "libSDL-1.2.so.0";
#define LIBNAME sdl1

int sdl_Yes() { return 1; }
int sdl_No() { return 0; }
int EXPORT my_SDL_Has3DNow() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_Has3DNowExt() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasAltiVec() __attribute__((alias("sdl_No")));
int EXPORT my_SDL_HasMMX() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasMMXExt() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasRDTSC() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE() __attribute__((alias("sdl_Yes")));
int EXPORT my_SDL_HasSSE2() __attribute__((alias("sdl_Yes")));

typedef struct {
  int32_t freq;
  uint16_t format;
  uint8_t channels;
  uint8_t silence;
  uint16_t samples;
  uint32_t size;
  void (*callback)(void *userdata, uint8_t *stream, int32_t len);
  void *userdata;
} SDL_AudioSpec;

typedef void (*vFv_t)();

#define ADDED_FUNCTIONS()           \
    GO(SDL_Quit, vFv_t)             \
    GO(SDL_AllocRW, sdl1_allocrw)   \
    GO(SDL_FreeRW, sdl1_freerw)     \

#include "generated/wrappedsdl1types.h"

#include "wrappercallback.h"

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)

// AudioCallback ...
#define GO(A)   \
static uintptr_t my_AudioCallback_fct_##A = 0;                                      \
static void my_AudioCallback_##A(void *userdata, uint8_t *stream, int32_t len)      \
{                                                                                   \
    RunFunction(my_context, my_AudioCallback_fct_##A, 3, userdata, stream, len);    \
}
SUPER()
#undef GO
static void* find_AudioCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_AudioCallback_fct_##A == (uintptr_t)fct) return my_AudioCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_AudioCallback_fct_##A == 0) {my_AudioCallback_fct_##A = (uintptr_t)fct; return my_AudioCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 AudioCallback callback\n");
    return NULL;
}
// TimerCallback ...
#define GO(A)   \
static uintptr_t my_TimerCallback_fct_##A = 0;                                                  \
static uint32_t my_TimerCallback_##A(uint32_t interval, void *userdata)                         \
{                                                                                               \
    return (uint32_t)RunFunction(my_context, my_TimerCallback_fct_##A, 2, interval, userdata);  \
}
SUPER()
#undef GO
static void* find_TimerCallback_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_TimerCallback_fct_##A == (uintptr_t)fct) return my_TimerCallback_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_TimerCallback_fct_##A == 0) {my_TimerCallback_fct_##A = (uintptr_t)fct; return my_TimerCallback_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 TimerCallback callback\n");
    return NULL;
}
// EvtFilter ...
#define GO(A)   \
static uintptr_t my_EvtFilter_fct_##A = 0;                      \
static int my_EvtFilter_##A(void* p)                            \
{                                                               \
    return RunFunction(my_context, my_EvtFilter_fct_##A, 1, p); \
}
SUPER()
#undef GO
static void* find_EvtFilter_Fct(void* fct)
{
    if(!fct) return fct;
    if(GetNativeFnc((uintptr_t)fct))  return GetNativeFnc((uintptr_t)fct);
    #define GO(A) if(my_EvtFilter_fct_##A == (uintptr_t)fct) return my_EvtFilter_##A;
    SUPER()
    #undef GO
    #define GO(A) if(my_EvtFilter_fct_##A == 0) {my_EvtFilter_fct_##A = (uintptr_t)fct; return my_EvtFilter_##A; }
    SUPER()
    #undef GO
    printf_log(LOG_NONE, "Warning, no more slot for SDL1 EvtFilter callback\n");
    return NULL;
}
static void* reverse_EvtFilterFct(void* fct)
{
    if(!fct) return fct;
    if(CheckBridged(my_lib->w.bridge, fct))
        return (void*)CheckBridged(my_lib->w.bridge, fct);
    #define GO(A) if(my_EvtFilter_##A == fct) return (void*)my_EvtFilter_fct_##A;
    SUPER()
    #undef GO
    return (void*)AddBridge(my_lib->w.bridge, iFp, fct, 0, NULL);
}
#undef SUPER

// TODO: track the memory for those callback
int EXPORT my_SDL_OpenAudio(x86emu_t* emu, void* d, void* o)
{
    (void)emu;
    SDL_AudioSpec *desired = (SDL_AudioSpec*)d;
    // create a callback
    void *fnc = (void*)desired->callback;
    desired->callback = find_AudioCallback_Fct(desired->callback);
    int ret = my->SDL_OpenAudio(desired, (SDL_AudioSpec*)o);
    if (ret!=0) {
        // error, clean the callback...
        desired->callback = fnc;
        return ret;
    }
    // put back stuff in place?
    desired->callback = fnc;

    return ret;
}

void EXPORT *my_SDL_LoadBMP_RW(x86emu_t* emu, void* a, int b)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->SDL_LoadBMP_RW(rw, b);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
int32_t EXPORT my_SDL_SaveBMP_RW(x86emu_t* emu, void* a, void* b, int c)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    int32_t r = my->SDL_SaveBMP_RW(rw, b, c);
    if(c==0)
        RWNativeEnd(rw);
    return r;
}
void EXPORT *my_SDL_LoadWAV_RW(x86emu_t* emu, void* a, int b, void* c, void* d, void* e)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    void* r = my->SDL_LoadWAV_RW(rw, b, c, d, e);
    if(b==0)
        RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE16(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE16(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadBE32(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadBE32(rw);
    RWNativeEnd(rw);
    return r;
}
uint64_t EXPORT my_SDL_ReadBE64(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint64_t r = my->SDL_ReadBE64(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE16(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE16(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_ReadLE32(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_ReadLE32(rw);
    RWNativeEnd(rw);
    return r;
}
uint64_t EXPORT my_SDL_ReadLE64(x86emu_t* emu, void* a)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint64_t r = my->SDL_ReadLE64(rw);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE16(x86emu_t* emu, void* a, uint16_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE16(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE32(x86emu_t* emu, void* a, uint32_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE32(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteBE64(x86emu_t* emu, void* a, uint64_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteBE64(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE16(x86emu_t* emu, void* a, uint16_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE16(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE32(x86emu_t* emu, void* a, uint32_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE32(rw, v);
    RWNativeEnd(rw);
    return r;
}
uint32_t EXPORT my_SDL_WriteLE64(x86emu_t* emu, void* a, uint64_t v)
{
    SDL1_RWops_t* rw = RWNativeStart(emu, (SDL1_RWops_t*)a);
    uint32_t r = my->SDL_WriteLE64(rw, v);
    RWNativeEnd(rw);
    return r;
}

// SDL1 doesn't really used rw_ops->type, but box86 does, so set sensible value (from SDL2)....
void EXPORT *my_SDL_RWFromConstMem(x86emu_t* emu, void* a, int b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromConstMem(a, b);
    RWSetType(r, 5);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromFP(x86emu_t* emu, void* a, int b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFP(a, b);
    RWSetType(r, 2);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromFile(x86emu_t* emu, void* a, void* b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromFile(a, b);
    RWSetType(r, 2);
    return AddNativeRW(emu, r);
}
void EXPORT *my_SDL_RWFromMem(x86emu_t* emu, void* a, int b)
{
    SDL1_RWops_t* r = (SDL1_RWops_t*)my->SDL_RWFromMem(a, b);
    RWSetType(r, 4);
    return AddNativeRW(emu, r);
}

EXPORT void* my_SDL_AddTimer(x86emu_t* emu, uint32_t a, void* cb, void* p)
{
    (void)emu;
    return my->SDL_AddTimer(a, find_TimerCallback_Fct(cb), p);
}

void EXPORT my_SDL_RemoveTimer(x86emu_t* emu, void* t)
{
    (void)emu;
    my->SDL_RemoveTimer(t);
}

int32_t EXPORT my_SDL_SetTimer(x86emu_t* emu, uint32_t t, void* p)
{
    (void)emu;
    return my->SDL_SetTimer(t, find_TimerCallback_Fct(p));
}
#if 0
int32_t EXPORT my_SDL_BuildAudioCVT(x86emu_t* emu, void* a, uint32_t b, uint32_t c, int32_t d, uint32_t e, uint32_t f, int32_t g)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_BuildAudioCVT\n");
    emu->quit = 1;
    return 0;
}

int32_t EXPORT my_SDL_ConvertAudio(x86emu_t* emu, void* a)
{
    printf_log(LOG_NONE, "Error, using Unimplemented SDL1 SDL_ConvertAudio\n");
    emu->quit = 1;
    return 0;
}
#endif
void EXPORT my_SDL_SetEventFilter(x86emu_t* emu, void* a)
{
    (void)emu;
    my->SDL_SetEventFilter(find_EvtFilter_Fct(a));
}
void EXPORT *my_SDL_GetEventFilter(x86emu_t* emu)
{
    (void)emu;
    return reverse_EvtFilterFct(my->SDL_GetEventFilter());
}

void EXPORT *my_SDL_CreateThread(x86emu_t* emu, void* cb, void* p)
{
    void* et = NULL;
    return my->SDL_CreateThread(my_prepare_thread(emu, cb, p, 0, &et), et);
}

void EXPORT my_SDL_KillThread(x86emu_t* emu, void* p)
{
    (void)emu;
    my->SDL_KillThread(p);
}

EXPORT void* my_SDL_GL_GetProcAddress(x86emu_t* emu, void* name) 
{
    khint_t k;
    const char* rname = (const char*)name;
    return getGLProcAddress(emu, (glprocaddress_t)my->SDL_GL_GetProcAddress, rname);
}

// DL functions from wrappedlibdl.c
void* my_dlopen(x86emu_t* emu, void *filename, int flag);
int my_dlclose(x86emu_t* emu, void *handle);
void* my_dlsym(x86emu_t* emu, void *handle, void *symbol);
EXPORT void* my_SDL_LoadObject(x86emu_t* emu, void* sofile)
{
    return my_dlopen(emu, sofile, 0);   // TODO: check correct flag value...
}
EXPORT void my_SDL_UnloadObject(x86emu_t* emu, void* handle)
{
    my_dlclose(emu, handle);
}
EXPORT void* my_SDL_LoadFunction(x86emu_t* emu, void* handle, void* name)
{
    return my_dlsym(emu, handle, name);
}

typedef struct my_SDL_version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} my_SDL_version;

typedef struct {
  my_SDL_version version;
  int subsystem;
  union {
    struct {
      void* display;
      void* window;
      void (*lock_func)(void);
      void (*unlock_func)(void);
      void* fswindow;
      void* wmwindow;
      void* gfxdisplay;
    } x11;
  } info;
} my_SDL_SysWMinfo;

EXPORT int32_t my_SDL_GetWMInfo(x86emu_t* emu, void* p)
{
    // does SDL_SysWMinfo needs alignment?
    int ret = my->SDL_GetWMInfo(p);
    my_SDL_SysWMinfo *info = (my_SDL_SysWMinfo*)p;
    if(info->info.x11.lock_func)
        info->info.x11.lock_func = (void*)AddBridge(emu->context->system, vFv, info->info.x11.lock_func, 0, NULL);
    if(info->info.x11.unlock_func)
        info->info.x11.unlock_func = (void*)AddBridge(emu->context->system, vFv, info->info.x11.unlock_func, 0, NULL);
    return ret;
}

#define CUSTOM_INIT \
    getMy(lib);                             \
    box86->sdl1allocrw = my->SDL_AllocRW;   \
    box86->sdl1freerw  = my->SDL_FreeRW;    \
    setNeededLibs(lib, 3,                   \
        "libm.so.6",                        \
        "libdl.so.2",                       \
        "librt.so.1");

#define CUSTOM_FINI \
    my->SDL_Quit();                 \
    freeMy();                       \
    my_context->sdl1allocrw = NULL; \
    my_context->sdl1freerw = NULL;

#include "wrappedlib_init.h"
