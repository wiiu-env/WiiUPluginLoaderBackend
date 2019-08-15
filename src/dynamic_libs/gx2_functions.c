/****************************************************************************
 * Copyright (C) 2015
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "coreinit.h"
#include "gx2_types.h"

uint32_t gx2_handle __attribute__((section(".data"))) = 0;

EXPORT_DECL(void, GX2Init, uint32_t * init_attribs);
EXPORT_DECL(void, GX2Shutdown, void);
EXPORT_DECL(void, GX2Flush, void);
EXPORT_DECL(int32_t, GX2GetMainCoreId, void) ;
EXPORT_DECL(int32_t, GX2DrawDone, void);
EXPORT_DECL(void, GX2ClearColor, GX2ColorBuffer *colorBuffer, float r, float g, float b, float a);
EXPORT_DECL(void, GX2SetViewport, float x, float y, float w, float h, float nearZ, float farZ);
EXPORT_DECL(void, GX2SetScissor, uint32_t x_orig, uint32_t y_orig, uint32_t wd, uint32_t ht);
EXPORT_DECL(void, GX2SetContextState, const GX2ContextState* state);
EXPORT_DECL(void, GX2DrawEx, int32_t primitive_type, uint32_t count, uint32_t first_vertex, uint32_t instances_count);
EXPORT_DECL(void, GX2DrawIndexedEx, int32_t primitive_type, uint32_t count, int32_t index_format, const void* idx, uint32_t first_vertex, uint32_t instances_count);
EXPORT_DECL(void, GX2ClearDepthStencilEx, GX2DepthBuffer *depthBuffer, float depth_value, uint8_t stencil_value, int32_t clear_mode);
EXPORT_DECL(void, GX2SetClearDepthStencil, GX2DepthBuffer *depthBuffer, float depth_value, uint8_t stencil_value);
EXPORT_DECL(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *colorBuffer, int32_t scan_target);
EXPORT_DECL(void, GX2SwapScanBuffers, void);
EXPORT_DECL(void, GX2SetTVEnable, int32_t enable);
EXPORT_DECL(void, GX2SetSwapInterval, uint32_t swap_interval);
EXPORT_DECL(uint32_t, GX2GetSwapInterval, void);
EXPORT_DECL(void, GX2WaitForVsync, void);
EXPORT_DECL(void, GX2CalcTVSize, int32_t tv_render_mode, int32_t format, int32_t buffering_mode, uint32_t * size, int32_t * scale_needed);
EXPORT_DECL(void, GX2Invalidate, int32_t invalidate_type, void * ptr, uint32_t buffer_size);
EXPORT_DECL(void, GX2SetTVBuffer, void *buffer, uint32_t buffer_size, int32_t tv_render_mode, int32_t format, int32_t buffering_mode);
EXPORT_DECL(void, GX2CalcSurfaceSizeAndAlignment, GX2Surface *surface);
EXPORT_DECL(void, GX2InitDepthBufferRegs, GX2DepthBuffer *depthBuffer);
EXPORT_DECL(void, GX2InitColorBufferRegs, GX2ColorBuffer *colorBuffer);
EXPORT_DECL(void, GX2CalcColorBufferAuxInfo, GX2ColorBuffer *colorBuffer, uint32_t *size, uint32_t *align);
EXPORT_DECL(void, GX2CalcDepthBufferHiZInfo, GX2DepthBuffer *depthBuffer, uint32_t *size, uint32_t *align);
EXPORT_DECL(void, GX2InitDepthBufferHiZEnable, GX2DepthBuffer *depthBuffer, int32_t hiZ_enable);
EXPORT_DECL(void, GX2SetupContextStateEx, GX2ContextState* state, int32_t enable_profiling);
EXPORT_DECL(void, GX2SetColorBuffer, const GX2ColorBuffer *colorBuffer, int32_t target);
EXPORT_DECL(void, GX2SetDepthBuffer, const GX2DepthBuffer *depthBuffer);
EXPORT_DECL(void, GX2SetAttribBuffer, uint32_t attr_index, uint32_t attr_size, uint32_t stride, const void* attr);
EXPORT_DECL(void, GX2InitTextureRegs, GX2Texture *texture);
EXPORT_DECL(void, GX2InitSampler, GX2Sampler *sampler, int32_t tex_clamp, int32_t min_mag_filter);
EXPORT_DECL(uint32_t, GX2CalcFetchShaderSizeEx, uint32_t num_attrib, int32_t fetch_shader_type, int32_t tessellation_mode);
EXPORT_DECL(void, GX2InitFetchShaderEx, GX2FetchShader* fs, void* fs_buffer, uint32_t count, const GX2AttribStream* attribs, int32_t fetch_shader_type, int32_t tessellation_mode);
EXPORT_DECL(void, GX2SetFetchShader, const GX2FetchShader* fs);
EXPORT_DECL(void, GX2SetVertexUniformReg, uint32_t offset, uint32_t count, const void *values);
EXPORT_DECL(void, GX2SetPixelUniformReg, uint32_t offset, uint32_t count, const void *values);
EXPORT_DECL(void, GX2SetPixelTexture, const GX2Texture *texture, uint32_t texture_hw_location);
EXPORT_DECL(void, GX2SetVertexTexture, const GX2Texture *texture, uint32_t texture_hw_location);
EXPORT_DECL(void, GX2SetPixelSampler, const GX2Sampler *sampler, uint32_t sampler_hw_location);
EXPORT_DECL(void, GX2SetVertexSampler, const GX2Sampler *sampler, uint32_t sampler_hw_location);
EXPORT_DECL(void, GX2SetPixelShader, const GX2PixelShader* pixelShader);
EXPORT_DECL(void, GX2SetVertexShader, const GX2VertexShader* vertexShader);
EXPORT_DECL(void, GX2InitSamplerZMFilter, GX2Sampler *sampler, int32_t z_filter, int32_t mip_filter);
EXPORT_DECL(void, GX2SetColorControl, int32_t lop, uint8_t blend_enable_mask, int32_t enable_multi_write, int32_t enable_color_buffer);
EXPORT_DECL(void, GX2SetDepthOnlyControl, int32_t enable_depth, int32_t enable_depth_write, int32_t depth_comp_function);
EXPORT_DECL(void, GX2SetBlendControl, int32_t target, int32_t color_src_blend, int32_t color_dst_blend, int32_t color_combine, int32_t separate_alpha_blend, int32_t alpha_src_blend, int32_t alpha_dst_blend, int32_t alpha_combine);
EXPORT_DECL(void, GX2CalcDRCSize, int32_t drc_mode, int32_t format, int32_t buffering_mode, uint32_t *size, int32_t *scale_needed);
EXPORT_DECL(void, GX2SetDRCBuffer, void *buffer, uint32_t buffer_size, int32_t drc_mode, int32_t surface_format, int32_t buffering_mode);
EXPORT_DECL(void, GX2SetDRCScale, uint32_t width, uint32_t height);
EXPORT_DECL(void, GX2SetDRCEnable, int32_t enable);
EXPORT_DECL(void, GX2SetPolygonControl, int32_t front_face_mode, int32_t cull_front, int32_t cull_back, int32_t enable_mode, int32_t mode_font, int32_t mode_back, int32_t poly_offset_front, int32_t poly_offset_back, int32_t point_line_offset);
EXPORT_DECL(void, GX2SetCullOnlyControl, int32_t front_face_mode, int32_t cull_front, int32_t cull_back);
EXPORT_DECL(void, GX2SetDepthStencilControl, int32_t enable_depth_test, int32_t enable_depth_write, int32_t depth_comp_function,  int32_t stencil_test_enable, int32_t back_stencil_enable,
            int32_t font_stencil_func, int32_t front_stencil_z_pass, int32_t front_stencil_z_fail, int32_t front_stencil_fail,
            int32_t back_stencil_func, int32_t back_stencil_z_pass, int32_t back_stencil_z_fail, int32_t back_stencil_fail);
EXPORT_DECL(void, GX2SetStencilMask, uint8_t mask_front, uint8_t write_mask_front, uint8_t ref_front, uint8_t mask_back, uint8_t write_mask_back, uint8_t ref_back);
EXPORT_DECL(void, GX2SetLineWidth, float width);
EXPORT_DECL(void, GX2SetTVGamma, float val);
EXPORT_DECL(void, GX2SetDRCGamma, float gam);
EXPORT_DECL(int32_t, GX2GetSystemTVScanMode, void);
EXPORT_DECL(int32_t, GX2GetSystemDRCScanMode, void);
EXPORT_DECL(void, GX2RSetAllocator, void * (* allocFunc)(uint32_t, uint32_t, uint32_t), void (* freeFunc)(uint32_t, void*));
EXPORT_DECL(void, GX2CopySurface, GX2Surface * srcSurface,uint32_t srcMip,uint32_t srcSlice,GX2Surface * dstSurface,uint32_t dstMip,uint32_t dstSlice );

EXPORT_DECL(int32_t, GX2GetLastFrame, int32_t target, GX2Texture * texture);
EXPORT_DECL(void, GX2BeginDisplayListEx,void * displayList,uint32_t size,int32_t unkwn);
EXPORT_DECL(uint32_t, GX2EndDisplayList, void * list);
EXPORT_DECL(void, GX2CallDisplayList, void * list, uint32_t size);
EXPORT_DECL(void, GX2ExpandAAColorBuffer,GX2ColorBuffer * buffer);
EXPORT_DECL(void, GX2ResolveAAColorBuffer, const GX2ColorBuffer * srcBuffer, GX2Surface * dstSurface,uint32_t dstMip,uint32_t dstSlice);

EXPORT_DECL(void, GX2ClearBuffersEx, GX2ColorBuffer * colorBuffer,GX2DepthBuffer * depthBuffer,float r, float g, float b, float a,float depthValue,uint8_t stencilValue,int32_t clearFlags);

void InitAcquireGX2(void) {
    if(coreinit_handle == 0) {
        InitAcquireOS();
    };
    OSDynLoad_Acquire("gx2.rpl", &gx2_handle);
}

void InitGX2FunctionPointers(void) {
    uint32_t *funcPointer = 0;
    InitAcquireGX2();

    OS_FIND_EXPORT(gx2_handle, GX2Init);
    OS_FIND_EXPORT(gx2_handle, GX2Shutdown);
    OS_FIND_EXPORT(gx2_handle, GX2Flush);
    OS_FIND_EXPORT(gx2_handle, GX2GetMainCoreId);
    OS_FIND_EXPORT(gx2_handle, GX2DrawDone);
    OS_FIND_EXPORT(gx2_handle, GX2ClearColor);
    OS_FIND_EXPORT(gx2_handle, GX2SetViewport);
    OS_FIND_EXPORT(gx2_handle, GX2SetScissor);
    OS_FIND_EXPORT(gx2_handle, GX2SetContextState);
    OS_FIND_EXPORT(gx2_handle, GX2DrawEx);
    OS_FIND_EXPORT(gx2_handle, GX2DrawIndexedEx);
    OS_FIND_EXPORT(gx2_handle, GX2ClearDepthStencilEx);
    OS_FIND_EXPORT(gx2_handle, GX2CopyColorBufferToScanBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2SwapScanBuffers);
    OS_FIND_EXPORT(gx2_handle, GX2SetTVEnable);
    OS_FIND_EXPORT(gx2_handle, GX2SetSwapInterval);
    OS_FIND_EXPORT(gx2_handle, GX2GetSwapInterval);
    OS_FIND_EXPORT(gx2_handle, GX2WaitForVsync);
    OS_FIND_EXPORT(gx2_handle, GX2CalcTVSize);
    OS_FIND_EXPORT(gx2_handle, GX2Invalidate);
    OS_FIND_EXPORT(gx2_handle, GX2SetTVBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2CalcSurfaceSizeAndAlignment);
    OS_FIND_EXPORT(gx2_handle, GX2InitDepthBufferRegs);
    OS_FIND_EXPORT(gx2_handle, GX2InitColorBufferRegs);
    OS_FIND_EXPORT(gx2_handle, GX2CalcColorBufferAuxInfo);
    OS_FIND_EXPORT(gx2_handle, GX2CalcDepthBufferHiZInfo);
    OS_FIND_EXPORT(gx2_handle, GX2InitDepthBufferHiZEnable);
    OS_FIND_EXPORT(gx2_handle, GX2SetupContextStateEx);
    OS_FIND_EXPORT(gx2_handle, GX2SetColorBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2SetDepthBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2SetAttribBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2InitTextureRegs);
    OS_FIND_EXPORT(gx2_handle, GX2InitSampler);
    OS_FIND_EXPORT(gx2_handle, GX2CalcFetchShaderSizeEx);
    OS_FIND_EXPORT(gx2_handle, GX2InitFetchShaderEx);
    OS_FIND_EXPORT(gx2_handle, GX2SetFetchShader);
    OS_FIND_EXPORT(gx2_handle, GX2SetVertexUniformReg);
    OS_FIND_EXPORT(gx2_handle, GX2SetPixelUniformReg);
    OS_FIND_EXPORT(gx2_handle, GX2SetPixelTexture);
    OS_FIND_EXPORT(gx2_handle, GX2SetVertexTexture);
    OS_FIND_EXPORT(gx2_handle, GX2SetPixelSampler);
    OS_FIND_EXPORT(gx2_handle, GX2SetVertexSampler);
    OS_FIND_EXPORT(gx2_handle, GX2SetPixelShader);
    OS_FIND_EXPORT(gx2_handle, GX2SetVertexShader);
    OS_FIND_EXPORT(gx2_handle, GX2InitSamplerZMFilter);
    OS_FIND_EXPORT(gx2_handle, GX2SetColorControl);
    OS_FIND_EXPORT(gx2_handle, GX2SetDepthOnlyControl);
    OS_FIND_EXPORT(gx2_handle, GX2SetBlendControl);
    OS_FIND_EXPORT(gx2_handle, GX2CalcDRCSize);
    OS_FIND_EXPORT(gx2_handle, GX2SetDRCBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2SetDRCScale);
    OS_FIND_EXPORT(gx2_handle, GX2SetDRCEnable);
    OS_FIND_EXPORT(gx2_handle, GX2SetPolygonControl);
    OS_FIND_EXPORT(gx2_handle, GX2SetCullOnlyControl);
    OS_FIND_EXPORT(gx2_handle, GX2SetDepthStencilControl);
    OS_FIND_EXPORT(gx2_handle, GX2SetStencilMask);
    OS_FIND_EXPORT(gx2_handle, GX2SetLineWidth);
    OS_FIND_EXPORT(gx2_handle, GX2SetDRCGamma);
    OS_FIND_EXPORT(gx2_handle, GX2SetTVGamma);
    OS_FIND_EXPORT(gx2_handle, GX2GetSystemTVScanMode);
    OS_FIND_EXPORT(gx2_handle, GX2GetSystemDRCScanMode);
    OS_FIND_EXPORT(gx2_handle, GX2RSetAllocator);
    OS_FIND_EXPORT(gx2_handle, GX2CopySurface);
    OS_FIND_EXPORT(gx2_handle, GX2GetLastFrame);
    OS_FIND_EXPORT(gx2_handle, GX2ClearBuffersEx);
    OS_FIND_EXPORT(gx2_handle, GX2BeginDisplayListEx);
    OS_FIND_EXPORT(gx2_handle, GX2EndDisplayList);
    OS_FIND_EXPORT(gx2_handle, GX2CallDisplayList);
    OS_FIND_EXPORT(gx2_handle, GX2ExpandAAColorBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2ResolveAAColorBuffer);
    OS_FIND_EXPORT(gx2_handle, GX2SetClearDepthStencil);
}
