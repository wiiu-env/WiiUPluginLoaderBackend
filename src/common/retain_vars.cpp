#include "retain_vars.h"
#include "utils/overlay_helper.h"
replacement_data_t gbl_replacement_data __attribute__((section(".data")));
dyn_linking_relocation_data_t gbl_dyn_linking_data __attribute__((section(".data")));

bool gInBackground __attribute__((section(".data"))) = false;
bool g_NotInLoader __attribute__((section(".data"))) = true;
uint64_t gGameTitleID __attribute__((section(".data"))) = 0;
volatile uint8_t gSDInitDone __attribute__((section(".data"))) = 0;

struct buffer_store drc_store __attribute__((section(".data")));
struct buffer_store tv_store __attribute__((section(".data")));

char gbl_common_data[0x20000] __attribute__((section(".data")));
char * gbl_common_data_ptr __attribute__((section(".data"))) = gbl_common_data;

/*
GX2ColorBuffer g_vid_main_cbuf __attribute__((section(".data")));
GX2Texture g_vid_drcTex __attribute__((section(".data")));
GX2Sampler g_vid_sampler __attribute__((section(".data")));
GX2Texture g_vid_tvTex __attribute__((section(".data")));
GX2ContextState* g_vid_ownContextState  __attribute__((section(".data")));
GX2ContextState* g_vid_originalContextSave __attribute__((section(".data")))= NULL;*/
