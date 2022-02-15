#include "audio_types.h"
#include "audio_render.h"
#include "audio_capture.h"
#include <a.h>
#include <b.h>

struct A1 {
    int32_t value;//测试
    int32_t ccnto(int cxunmz);
    int32_t (*FuncPointer)(int a,char b,int c);
};

struct A2 {
    int32_t value;
};

class A3 {
public:
    int32_t value;
    int32_t ccnto(int cxunmz);
    int32_t (*FuncPointer)(int adapter);
};

typedef void *AudioHandle;
typedef int32_t (*RenderCallback)(void *reserved, void *cookie);

class A4 {
public:
    int32_t value;
    AudioHandle a;
    RenderCallback b;
};