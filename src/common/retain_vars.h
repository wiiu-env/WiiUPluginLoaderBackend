#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include "patcher/function_patcher.h"
#include "plugin/dynamic_linking_defines.h"
#include "utils/overlay_helper.h"
//#include <dynamic_libs/gx2_functions.h>

extern replacement_data_t gbl_replacement_data;
extern to_link_and_load_data_t gbl_to_link_and_load_data[MAXIMUM_PLUGINS];
extern dyn_linking_relocation_data_t gbl_dyn_linking_data;

extern bool g_NotInLoader;

extern bool gInBackground;
extern uint64_t gGameTitleID;
extern volatile uint8_t gSDInitDone;

extern void * ntfs_mounts;
extern int32_t ntfs_mount_count;

extern struct buffer_store drc_store;
extern struct buffer_store tv_store;

/*
extern GX2ColorBuffer g_vid_main_cbuf;
extern GX2Texture g_vid_drcTex;
extern GX2Texture g_vid_tvTex;
extern GX2ContextState* g_vid_ownContextState;
extern GX2ContextState* g_vid_originalContextSave;
extern GX2Sampler g_vid_sampler;*/

#endif // RETAINS_VARS_H_
