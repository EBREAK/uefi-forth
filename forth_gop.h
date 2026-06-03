#pragma once

#include "forth.h"

#define FORTH_GOP_MAX 8

extern void forth_o_gop_init(struct forth_context *fctx);
extern void forth_o_gop_cnt(struct forth_context *fctx);
extern void forth_o_gop_sel(struct forth_context *fctx);
extern void forth_o_gop_cur(struct forth_context *fctx);
extern void forth_o_gop_mode_cnt(struct forth_context *fctx);
extern void forth_o_gop_mode_sel(struct forth_context *fctx);
extern void forth_o_gop_mode_cur(struct forth_context *fctx);
extern void forth_o_gop_fb(struct forth_context *fctx);
extern void forth_o_gop_fbsize(struct forth_context *fctx);
extern void forth_o_gop_pixfmt(struct forth_context *fctx);
extern void forth_o_gop_ppsl(struct forth_context *fctx);
extern void forth_o_gop_hres(struct forth_context *fctx);
extern void forth_o_gop_vres(struct forth_context *fctx);
extern void forth_o_gop_plot(struct forth_context *fctx);
extern void forth_o_gop_solid(struct forth_context *fctx);
extern void forth_init_gop(void);

