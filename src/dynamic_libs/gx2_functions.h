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
#ifndef __GX2_FUNCTIONS_H_
#define __GX2_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "gx2_types.h"

extern uint32_t gx2_handle;

void InitGX2FunctionPointers(void);
void InitAcquireGX2(void);

extern void (* GX2Init)(uint32_t * init_attribs);
extern void (* GX2Shutdown)(void);
extern void (* GX2Flush)(void);
extern int32_t (* GX2GetMainCoreId)(void) ;
extern int32_t (* GX2DrawDone)(void);
extern void (* GX2ClearColor)(GX2ColorBuffer *colorBuffer, float r, float g, float b, float a);
extern void (* GX2SetViewport)(float x, float y, float w, float h, float nearZ, float farZ);
extern void (* GX2SetScissor)(uint32_t x_orig, uint32_t y_orig, uint32_t wd, uint32_t ht);
extern void (* GX2SetContextState)(const GX2ContextState* state);
extern void (* GX2DrawEx)(int32_t primitive_type, uint32_t count, uint32_t first_vertex, uint32_t instances_count);
extern void (* GX2DrawIndexedEx)(int32_t primitive_type, uint32_t count, int32_t index_format, const void* idx, uint32_t first_vertex, uint32_t instances_count);
extern void (* GX2ClearDepthStencilEx)(GX2DepthBuffer *depthBuffer, float depth_value, uint8_t stencil_value, int32_t clear_mode);
extern void (* GX2SetClearDepthStencil)(GX2DepthBuffer *depthBuffer, float depth_value, uint8_t stencil_value);
extern void (* GX2CopyColorBufferToScanBuffer)(const GX2ColorBuffer *colorBuffer, int32_t scan_target);
extern void (* GX2SwapScanBuffers)(void);
extern void (* GX2SetTVEnable)(int32_t enable);
extern void (* GX2SetSwapInterval)(uint32_t swap_interval);
extern uint32_t (* GX2GetSwapInterval)(void);
extern void (* GX2WaitForVsync)(void);
extern void (* GX2CalcTVSize)(int32_t tv_render_mode, int32_t format, int32_t buffering_mode, uint32_t * size, int32_t * scale_needed);
extern void (* GX2Invalidate)(int32_t invalidate_type, void * ptr, uint32_t buffer_size);
extern void (* GX2SetTVBuffer)(void *buffer, uint32_t buffer_size, int32_t tv_render_mode, int32_t format, int32_t buffering_mode);
extern void (* GX2CalcSurfaceSizeAndAlignment)(GX2Surface *surface);
extern void (* GX2InitDepthBufferRegs)(GX2DepthBuffer *depthBuffer);
extern void (* GX2InitColorBufferRegs)(GX2ColorBuffer *colorBuffer);
extern void (* GX2CalcColorBufferAuxInfo)(GX2ColorBuffer *colorBuffer, uint32_t *size, uint32_t *align);
extern void (* GX2CalcDepthBufferHiZInfo)(GX2DepthBuffer *depthBuffer, uint32_t *size, uint32_t *align);
extern void (* GX2InitDepthBufferHiZEnable)(GX2DepthBuffer *depthBuffer, int32_t hiZ_enable);
extern void (* GX2SetupContextStateEx)(GX2ContextState* state, int32_t enable_profiling);
extern void (* GX2SetColorBuffer)(const GX2ColorBuffer *colorBuffer, int32_t target);
extern void (* GX2SetDepthBuffer)(const GX2DepthBuffer *depthBuffer);
extern void (* GX2SetAttribBuffer)(uint32_t attr_index, uint32_t attr_size, uint32_t stride, const void* attr);
extern void (* GX2InitTextureRegs)(GX2Texture *texture);
extern void (* GX2InitSampler)(GX2Sampler *sampler, int32_t tex_clamp, int32_t min_mag_filter);
extern uint32_t (* GX2CalcFetchShaderSizeEx)(uint32_t num_attrib, int32_t fetch_shader_type, int32_t tessellation_mode);
extern void (* GX2InitFetchShaderEx)(GX2FetchShader* fs, void* fs_buffer, uint32_t count, const GX2AttribStream* attribs, int32_t fetch_shader_type, int32_t tessellation_mode);
extern void (* GX2SetFetchShader)(const GX2FetchShader* fs);
extern void (* GX2SetVertexUniformReg)(uint32_t offset, uint32_t count, const void *values);
extern void (* GX2SetPixelUniformReg)(uint32_t offset, uint32_t count, const void *values);
extern void (* GX2SetPixelTexture)(const GX2Texture *texture, uint32_t texture_hw_location);
extern void (* GX2SetVertexTexture)(const GX2Texture *texture, uint32_t texture_hw_location);
extern void (* GX2SetPixelSampler)(const GX2Sampler *sampler, uint32_t sampler_hw_location);
extern void (* GX2SetVertexSampler)(const GX2Sampler *sampler, uint32_t sampler_hw_location);
extern void (* GX2SetPixelShader)(const GX2PixelShader* pixelShader);
extern void (* GX2SetVertexShader)(const GX2VertexShader* vertexShader);
extern void (* GX2InitSamplerZMFilter)(GX2Sampler *sampler, int32_t z_filter, int32_t mip_filter);
extern void (* GX2SetColorControl)(int32_t lop, uint8_t blend_enable_mask, int32_t enable_multi_write, int32_t enable_color_buffer);
extern void (* GX2SetDepthOnlyControl)(int32_t enable_depth, int32_t enable_depth_write, int32_t depth_comp_function);
extern void (* GX2SetBlendControl)(int32_t target, int32_t color_src_blend, int32_t color_dst_blend, int32_t color_combine, int32_t separate_alpha_blend, int32_t alpha_src_blend, int32_t alpha_dst_blend, int32_t alpha_combine);
extern void (* GX2CalcDRCSize)(int32_t drc_mode, int32_t format, int32_t buffering_mode, uint32_t *size, int32_t *scale_needed);
extern void (* GX2SetDRCBuffer)(void *buffer, uint32_t buffer_size, int32_t drc_mode, int32_t surface_format, int32_t buffering_mode);
extern void (* GX2SetDRCScale)(uint32_t width, uint32_t height);
extern void (* GX2SetDRCEnable)(int32_t enable);
extern void (* GX2SetPolygonControl)(int32_t front_face_mode, int32_t cull_front, int32_t cull_back, int32_t enable_mode, int32_t mode_font, int32_t mode_back, int32_t poly_offset_front, int32_t poly_offset_back, int32_t point_line_offset);
extern void (* GX2SetCullOnlyControl)(int32_t front_face_mode, int32_t cull_front, int32_t cull_back);
extern void (* GX2SetDepthStencilControl)(int32_t enable_depth_test, int32_t enable_depth_write, int32_t depth_comp_function,  int32_t stencil_test_enable, int32_t back_stencil_enable,
        int32_t font_stencil_func, int32_t front_stencil_z_pass, int32_t front_stencil_z_fail, int32_t front_stencil_fail,
        int32_t back_stencil_func, int32_t back_stencil_z_pass, int32_t back_stencil_z_fail, int32_t back_stencil_fail);
extern void (* GX2SetStencilMask)(uint8_t mask_front, uint8_t write_mask_front, uint8_t ref_front, uint8_t mask_back, uint8_t write_mask_back, uint8_t ref_back);
extern void (* GX2SetLineWidth)(float width);
extern void (* GX2SetTVGamma)(float val);
extern void (* GX2SetDRCGamma)(float val);
extern int32_t (* GX2GetSystemTVScanMode)(void);
extern int32_t (* GX2GetSystemDRCScanMode)(void);
extern void (* GX2RSetAllocator)(void * (*allocFunc)(uint32_t, uint32_t, uint32_t), void (*freeFunc)(uint32_t, void*));
extern void (* GX2CopySurface)(GX2Surface * srcSurface,uint32_t srcMip,uint32_t srcSlice,GX2Surface * dstSurface,uint32_t dstMip,uint32_t dstSlice );
extern void (* GX2ClearBuffersEx)(GX2ColorBuffer * colorBuffer,GX2DepthBuffer * depthBuffer,float r, float g, float b, float a,float depthValue,uint8_t stencilValue,int32_t clearFlags);
extern int32_t (* GX2GetLastFrame)(int32_t target, GX2Texture * texture);
extern void (* GX2BeginDisplayListEx)(void * displayList,uint32_t size,int32_t unkwn);
extern uint32_t (*GX2EndDisplayList)(void * list);
extern void (*GX2CallDisplayList)(void * list, uint32_t size);
extern void (*GX2ExpandAAColorBuffer)(GX2ColorBuffer * buffer);
extern void (*GX2ResolveAAColorBuffer)(const GX2ColorBuffer * srcBuffer, GX2Surface * dstSurface,uint32_t dstMip,uint32_t dstSlice);

static inline void GX2InitDepthBuffer(GX2DepthBuffer *depthBuffer, int32_t dimension, uint32_t width, uint32_t height, uint32_t depth, int32_t format, int32_t aa) {
    depthBuffer->surface.dimension = dimension;
    depthBuffer->surface.width = width;
    depthBuffer->surface.height = height;
    depthBuffer->surface.depth = depth;
    depthBuffer->surface.num_mips = 1;
    depthBuffer->surface.format = format;
    depthBuffer->surface.aa = aa;
    depthBuffer->surface.use = ((format==GX2_SURFACE_FORMAT_D_D24_S8_UNORM) || (format==GX2_SURFACE_FORMAT_D_D24_S8_FLOAT)) ? GX2_SURFACE_USE_DEPTH_BUFFER : GX2_SURFACE_USE_DEPTH_BUFFER_TEXTURE;
    depthBuffer->surface.tile = GX2_TILE_MODE_DEFAULT;
    depthBuffer->surface.swizzle  = 0;
    depthBuffer->view_mip = 0;
    depthBuffer->view_first_slice = 0;
    depthBuffer->view_slices_count = depth;
    depthBuffer->clear_depth = 1.0f;
    depthBuffer->clear_stencil = 0;
    depthBuffer->hiZ_data = NULL;
    depthBuffer->hiZ_size = 0;
    GX2CalcSurfaceSizeAndAlignment(&depthBuffer->surface);
    GX2InitDepthBufferRegs(depthBuffer);
}

static inline void GX2InitColorBuffer(GX2ColorBuffer *colorBuffer, int32_t dimension, uint32_t width, uint32_t height, uint32_t depth, int32_t format, int32_t aa) {
    colorBuffer->surface.dimension = dimension;
    colorBuffer->surface.width = width;
    colorBuffer->surface.height = height;
    colorBuffer->surface.depth = depth;
    colorBuffer->surface.num_mips = 1;
    colorBuffer->surface.format = format;
    colorBuffer->surface.aa = aa;
    colorBuffer->surface.use = GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE_FTV;
    colorBuffer->surface.image_size = 0;
    colorBuffer->surface.image_data = NULL;
    colorBuffer->surface.mip_size = 0;
    colorBuffer->surface.mip_data = NULL;
    colorBuffer->surface.tile = GX2_TILE_MODE_DEFAULT;
    colorBuffer->surface.swizzle = 0;
    colorBuffer->surface.align = 0;
    colorBuffer->surface.pitch = 0;
    uint32_t i;
    for(i = 0; i < 13; i++)
        colorBuffer->surface.mip_offset[i] = 0;
    colorBuffer->view_mip = 0;
    colorBuffer->view_first_slice = 0;
    colorBuffer->view_slices_count = depth;
    colorBuffer->aux_data = NULL;
    colorBuffer->aux_size = 0;
    for(i = 0; i < 5; i++)
        colorBuffer->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&colorBuffer->surface);
    GX2InitColorBufferRegs(colorBuffer);
}

static inline void GX2InitAttribStream(GX2AttribStream* attr, uint32_t location, uint32_t buffer, uint32_t offset, int32_t format) {
    attr->location = location;
    attr->buffer = buffer;
    attr->offset = offset;
    attr->format = format;
    attr->index_type = 0;
    attr->divisor = 0;
    attr->destination_selector = attribute_dest_comp_selector[format & 0xff];
    attr->endian_swap  = GX2_ENDIANSWAP_DEFAULT;
}

static inline void GX2InitTexture(GX2Texture *tex, uint32_t width, uint32_t height, uint32_t depth, uint32_t num_mips, int32_t format, int32_t dimension, int32_t tile) {
    tex->surface.dimension = dimension;
    tex->surface.width = width;
    tex->surface.height = height;
    tex->surface.depth = depth;
    tex->surface.num_mips = num_mips;
    tex->surface.format = format;
    tex->surface.aa = GX2_AA_MODE_1X;
    tex->surface.use = GX2_SURFACE_USE_TEXTURE;
    tex->surface.image_size = 0;
    tex->surface.image_data = NULL;
    tex->surface.mip_size = 0;
    tex->surface.mip_data = NULL;
    tex->surface.tile = tile;
    tex->surface.swizzle = 0;
    tex->surface.align = 0;
    tex->surface.pitch = 0;
    uint32_t i;
    for(i = 0; i < 13; i++)
        tex->surface.mip_offset[i] = 0;
    tex->view_first_mip = 0;
    tex->view_mips_count = num_mips;
    tex->view_first_slice = 0;
    tex->view_slices_count = depth;
    tex->component_selector = texture_comp_selector[format & 0x3f];
    for(i = 0; i < 5; i++)
        tex->regs[i] = 0;

    GX2CalcSurfaceSizeAndAlignment(&tex->surface);
    GX2InitTextureRegs(tex);
}

#ifdef __cplusplus
}
#endif

#endif // __GX2_FUNCTIONS_H_
