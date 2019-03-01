/****************************************************************************
 * Copyright (C) 2018 Maschell
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <utils/logger.h>
#include "texture_utils.h"
#include <wups.h>
#include <gd.h>
#include <dynamic_libs/gx2_functions.h>
#include <dynamic_libs/gx2_types.h>
#include "mem_utils.h"

void gdImageToUnormR8G8B8A8(gdImagePtr gdImg, u32 *imgBuffer, u32 width, u32 height, u32 pitch) {
    for(u32 y = 0; y < height; ++y) {
        for(u32 x = 0; x < width; ++x) {
            u32 pixel = gdImageGetPixel(gdImg, x, y);

            u8 a = 254 - 2*((u8)gdImageAlpha(gdImg, pixel));
            if(a == 254)
                a++;

            u8 r = gdImageRed(gdImg, pixel);
            u8 g = gdImageGreen(gdImg, pixel);
            u8 b = gdImageBlue(gdImg, pixel);

            imgBuffer[y * pitch + x] = (r << 24) | (g << 16) | (b << 8) | (a);
        }
    }
}


void TextureUtils::drawTexture(GX2Texture * texture, GX2Sampler* sampler, float x, float y, int32_t width, int32_t height, float alpha = 1.0f) {
    float widthScaleFactor = 1.0f / (float)1280;
    float heightScaleFactor = 1.0f / (float)720;

    glm::vec3 positionOffsets = glm::vec3(0.0f);

    positionOffsets[0] = (x-((1280)/2)+(width/2)) * widthScaleFactor * 2.0f;
    positionOffsets[1] = -(y-((720)/2)+(height/2)) * heightScaleFactor * 2.0f;

    glm::vec3 scale(width*widthScaleFactor,height*heightScaleFactor,1.0f);

    Texture2DShader::instance()->setShaders();
    Texture2DShader::instance()->setAttributeBuffer();
    Texture2DShader::instance()->setAngle(0.0f);
    Texture2DShader::instance()->setOffset(positionOffsets);
    Texture2DShader::instance()->setScale(scale);
    Texture2DShader::instance()->setColorIntensity(glm::vec4(alpha));
    Texture2DShader::instance()->setBlurring(glm::vec3(0.0f));
    Texture2DShader::instance()->setTextureAndSampler(texture, sampler);
    Texture2DShader::instance()->draw();
}


void TextureUtils::copyToTexture(GX2ColorBuffer* sourceBuffer, GX2Texture * target) {
    if(sourceBuffer == NULL || target == NULL) {
        return;
    }
    if (sourceBuffer->surface.aa == GX2_AA_MODE_1X) {
        // If AA is disabled, we can simply use GX2CopySurface.
        GX2CopySurface(&sourceBuffer->surface,
                       sourceBuffer->view_mip,
                       sourceBuffer->view_first_slice,
                       &target->surface, 0, 0);
        GX2DrawDone();
    } else {
        // If AA is enabled, we need to resolve the AA buffer.

        // Allocate surface to resolve buffer onto
        GX2Surface tempSurface;
        tempSurface = sourceBuffer->surface;
        tempSurface.aa = GX2_AA_MODE_1X;
        GX2CalcSurfaceSizeAndAlignment(&tempSurface);

        tempSurface.image_data = MemoryUtils::alloc(
                                     tempSurface.image_size,
                                     tempSurface.align
                                 );
        if(tempSurface.image_data == NULL) {
            DEBUG_FUNCTION_LINE("VideoSquoosher: failed to allocate AA surface\n");
            if(target->surface.image_data != NULL) {
                MemoryUtils::free(target->surface.image_data);
                target->surface.image_data = NULL;
            }
            return;
        }

        // Resolve, then copy result to target
        GX2ResolveAAColorBuffer(sourceBuffer,&tempSurface, 0, 0);
        GX2CopySurface(&tempSurface, 0, 0,&target->surface, 0, 0);

        if(tempSurface.image_data != NULL) {
            MemoryUtils::free(tempSurface.image_data);
            tempSurface.image_data = NULL;
        }
        GX2DrawDone();
        GX2Invalidate(GX2_INVALIDATE_CPU, target->surface.image_data, target->surface.image_size);
    }
}

bool TextureUtils::convertImageToTexture(const uint8_t *img, int32_t imgSize, void * _texture) {
    if(!img || (imgSize < 8) || _texture == NULL) {
        return false;
    }
    GX2Texture * texture = (GX2Texture *) _texture;

    gdImagePtr gdImg = 0;

    if (img[0] == 0xFF && img[1] == 0xD8) {
        //! not needed for now therefore comment out to safe ELF size
        //! if needed uncomment, adds 200 kb to the ELF size
        // IMAGE_JPEG
        gdImg = gdImageCreateFromJpegPtr(imgSize, (uint8_t*) img);
    } else if (img[0] == 'B' && img[1] == 'M') {
        // IMAGE_BMP
        //gdImg = gdImageCreateFromBmpPtr(imgSize, (uint8_t*) img);
    } else if (img[0] == 0x89 && img[1] == 'P' && img[2] == 'N' && img[3] == 'G') {
        // IMAGE_PNG
        gdImg = gdImageCreateFromPngPtr(imgSize, (uint8_t*) img);
    }
    //!This must be last since it can also intefere with outher formats
    else if(img[0] == 0x00) {
        // Try loading TGA image
        //gdImg = gdImageCreateFromTgaPtr(imgSize, (uint8_t*) img);
    }

    if(gdImg == 0) {
        return false;
    }

    uint32_t width = (gdImageSX(gdImg));
    uint32_t height = (gdImageSY(gdImg));

    //! Initialize texture
    GX2InitTexture(texture, width,  height, 1, 0, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_SURFACE_DIM_2D, GX2_TILE_MODE_LINEAR_ALIGNED);

    //! if this fails something went horribly wrong
    if(texture->surface.image_size == 0) {
        gdImageDestroy(gdImg);
        return false;
    }

    texture->surface.image_data = MemoryUtils::alloc(texture->surface.image_size, texture->surface.align);

    //! check if memory is available for image
    if(!texture->surface.image_data) {
        gdImageDestroy(gdImg);
        return false;
    }

    //! set mip map data pointer
    texture->surface.mip_data = NULL;

    gdImageToUnormR8G8B8A8(gdImg, (uint32_t*)texture->surface.image_data, texture->surface.width, texture->surface.height, texture->surface.pitch);

    //! free memory of image as its not needed anymore
    gdImageDestroy(gdImg);

    //! invalidate the memory
    GX2Invalidate(GX2_INVALIDATE_CPU_TEXTURE, texture->surface.image_data, texture->surface.image_size);
    return true;
}
