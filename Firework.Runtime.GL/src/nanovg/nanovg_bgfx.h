/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef NANOVG_BGFX_H_HEADER_GUARD
#define NANOVG_BGFX_H_HEADER_GUARD

#include "Firework.Runtime.GL.Exports.h"

#include <bgfx/bgfx.h>

namespace bx { struct AllocatorI; }

struct NVGcontext;

struct NVGLUframebuffer
{
  NVGcontext* ctx;
  bgfx::FrameBufferHandle handle;
  int image;
  bgfx::ViewId viewId;
};

// These are additional flags on top of NVGimageFlags.
enum NVGimageFlagsGL {
	NVG_IMAGE_NODELETE = 1<<16, // Do not delete GL texture handle.
};

///
_fw_gl_api NVGcontext* nvgCreate(int32_t _edgeaa, bgfx::ViewId _viewId, bx::AllocatorI* _allocator);

///
_fw_gl_api NVGcontext* nvgCreate(int32_t _edgeaa, bgfx::ViewId _viewId);

///
_fw_gl_api void nvgDelete(NVGcontext* _ctx);

///
_fw_gl_api void nvgSetViewId(NVGcontext* _ctx, bgfx::ViewId _viewId);

///
_fw_gl_api uint16_t nvgGetViewId(struct NVGcontext* _ctx);

// Helper functions to create bgfx framebuffer to render to.
// Example:
//		float scale = 2;
//		NVGLUframebuffer* fb = nvgluCreateFramebuffer(ctx, 100 * scale, 100 * scale, 0);
//		nvgluSetViewFramebuffer(VIEW_ID, fb);
//		nvgluBindFramebuffer(fb);
//		nvgBeginFrame(ctx, 100, 100, scale);
//		// renders anything offscreen
//		nvgEndFrame(ctx);
//		nvgluBindFramebuffer(NULL);
//
//		// Pastes the framebuffer rendering.
//		nvgBeginFrame(ctx, 1024, 768, scale);
//		NVGpaint paint = nvgImagePattern(ctx, 0, 0, 100, 100, 0, fb->image, 1);
//		nvgBeginPath(ctx);
//		nvgRect(ctx, 0, 0, 100, 100);
//		nvgFillPaint(ctx, paint);
//		nvgFill(ctx);
//		nvgEndFrame(ctx);

///
_fw_gl_api NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* _ctx, int32_t _width, int32_t _height, int32_t _imageFlags, bgfx::ViewId _viewId);

///
_fw_gl_api NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* _ctx, int32_t _width, int32_t _height, int32_t _imageFlags);

///
_fw_gl_api NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* _ctx, int32_t _imageFlags, bgfx::ViewId _viewId);

///
_fw_gl_api NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* _ctx, int32_t _imageFlags);

///
_fw_gl_api void nvgluBindFramebuffer(NVGLUframebuffer* _framebuffer);

///
_fw_gl_api void nvgluDeleteFramebuffer(NVGLUframebuffer* _framebuffer);

///
_fw_gl_api void nvgluSetViewFramebuffer(bgfx::ViewId _viewId, NVGLUframebuffer* _framebuffer);

///
_fw_gl_api int nvgCreateBgfxTexture(struct NVGcontext *_ctx, bgfx::TextureHandle _id, int _width, int _height, int _flags);

#endif // NANOVG_BGFX_H_HEADER_GUARD
