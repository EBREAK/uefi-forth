#include "forth_gop.h"
#include "main.h"
#include "forth.h"
#include "debug.h"

static EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop_table[FORTH_GOP_MAX];
static UINTN gop_cnt = 0;
static UINTN gop_sel = 0;

void forth_o_gop_init(struct forth_context *fctx)
{
	GST->BootServices->SetMem(gop_table,
				  sizeof(gop_table[0]) * FORTH_GOP_MAX, 0);
	EFI_STATUS Status;
	UINTN handle_count = 0;
	EFI_HANDLE *handle_buffer = NULL;

	Status = GST->BootServices->LocateHandleBuffer(
		ByProtocol, &gop_guid, NULL, &handle_count, &handle_buffer);

	if (EFI_ERROR(Status) || handle_count == 0) {
		gop_cnt = 0;
		gop_sel = 0;
		forth_ppush(fctx, fctx->tos);
		fctx->tos = 0;
		return;
	}

	gop_cnt = (handle_count > FORTH_GOP_MAX) ? FORTH_GOP_MAX : handle_count;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo;

	for (UINTN i = 0; i < gop_cnt; i++) {
		EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
		Status = GST->BootServices->HandleProtocol(
			handle_buffer[i], &gop_guid, (void **)&gop);
		if (EFI_ERROR(Status)) {
			gop_table[i] = NULL;
		} else {
			Status = gop->QueryMode(
				gop, gop->Mode == NULL ? 0 : gop->Mode->Mode,
				&SizeOfInfo, &info);
			if (!EFI_ERROR(Status)) {
				GST->BootServices->FreePool(info);
			}
			if (Status == EFI_NOT_STARTED) {
				Status = gop->SetMode(gop, 0);
			}
			if (EFI_ERROR(Status)) {
				debug_putws(L"GOP ");
				debug_puthex((uintptr_t)gop);
				debug_putws(L" UNABLE GET NATIVE MODE\r\n");
			}
			gop_table[i] = gop;
		}
	}

	GST->BootServices->FreePool(handle_buffer);
	gop_sel = 0;

	forth_ppush(fctx, fctx->tos);
	fctx->tos = gop_cnt;
}

void forth_o_gop_cnt(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	fctx->tos = gop_cnt;
}

void forth_o_gop_sel(struct forth_context *fctx)
{
	UINTN n;
	n = fctx->tos;
	fctx->tos = FORTH_FALSE;
	if (n >= FORTH_GOP_MAX) {
		return;
	}
	if (n >= gop_cnt) {
		return;
	}
	gop_sel = n;
	fctx->tos = FORTH_TRUE;
}

void forth_o_gop_cur(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	fctx->tos = gop_sel;
}

void forth_o_gop_mode_cnt(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	if (gop_table[gop_sel] == NULL) {
		fctx->tos = 0;
		return;
	}
	if (gop_table[gop_sel]->Mode == NULL) {
		fctx->tos = 0;
		return;
	}
	fctx->tos = gop_table[gop_sel]->Mode->MaxMode;
}

void forth_o_gop_mode_sel(struct forth_context *fctx)
{
	UINTN n;
	n = fctx->tos;
	fctx->tos = FORTH_FALSE;
	if (gop_table[gop_sel] == NULL) {
		return;
	}
	if ((gop_table[gop_sel]->Mode != NULL) &&
	    (n >= (gop_table[gop_sel]->Mode->MaxMode))) {
		return;
	}
	EFI_STATUS Status;
	Status = gop_table[gop_sel]->SetMode(gop_table[gop_sel], n);
	if (EFI_ERROR(Status)) {
		return;
	}
	fctx->tos = FORTH_TRUE;
}

void forth_o_gop_mode_cur(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	if (gop_table[gop_sel] == NULL) {
		fctx->tos = 0;
		return;
	}
	if (gop_table[gop_sel]->Mode == NULL) {
		fctx->tos = 0;
		return;
	}
	fctx->tos = gop_table[gop_sel]->Mode->Mode;
}

void forth_o_gop_hres(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	if (gop_table[gop_sel] == NULL) {
		fctx->tos = 0;
		return;
	}
	if (gop_table[gop_sel]->Mode == NULL) {
		fctx->tos = 0;
		return;
	}
	fctx->tos = gop_table[gop_sel]->Mode->Info->HorizontalResolution;
}

void forth_o_gop_vres(struct forth_context *fctx)
{
	forth_ppush(fctx, fctx->tos);
	if (gop_table[gop_sel] == NULL) {
		fctx->tos = 0;
		return;
	}
	if (gop_table[gop_sel]->Mode == NULL) {
		fctx->tos = 0;
		return;
	}
	fctx->tos = gop_table[gop_sel]->Mode->Info->VerticalResolution;
}

void gop_blt_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h,
		  uint32_t color)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION pixel;
	pixel.Raw = color;
	if (gop_table[gop_sel] == NULL) {
		return;
	}
	if (gop_table[gop_sel]->Mode == NULL) {
		return;
	}
	if ((x + w) >= gop_table[gop_sel]->Mode->Info->HorizontalResolution) {
		if (x >= gop_table[gop_sel]->Mode->Info->HorizontalResolution) {
			return;
		}
		w = gop_table[gop_sel]->Mode->Info->HorizontalResolution - x;
	}
	if ((y + h) >= gop_table[gop_sel]->Mode->Info->VerticalResolution) {
		if (y >= gop_table[gop_sel]->Mode->Info->VerticalResolution) {
			return;
		}
		h = gop_table[gop_sel]->Mode->Info->VerticalResolution - y;
	}
	gop_table[gop_sel]->Blt(gop_table[gop_sel], &pixel.Pixel,
				EfiBltVideoFill, 0, 0, x, y, w, h, 0);
}

static inline void gop_plot(uint32_t x, uint32_t y, uint32_t color)
{
	gop_blt_fill(x, y, 1, 1, color);
}

void forth_o_gop_plot(struct forth_context *fctx)
{
	uint32_t x, y, color;
	y = fctx->tos;
	x = forth_ppop(fctx);
	color = forth_ppop(fctx);
	fctx->tos = forth_ppop(fctx);
	gop_plot(x, y, color);
}

void forth_o_gop_solid(struct forth_context *fctx)
{
	uint32_t color, x, y, w, h;
	h = fctx->tos;
	w = forth_ppop(fctx);
	y = forth_ppop(fctx);
	x = forth_ppop(fctx);
	color = forth_ppop(fctx);
	fctx->tos = forth_ppop(fctx);
	gop_blt_fill(x, y, w, h, color);
}

static uint32_t color2pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t x)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION pixel;
	pixel.Pixel.Red = r;
	pixel.Pixel.Green = g;
	pixel.Pixel.Blue = b;
	pixel.Pixel.Reserved = x;
	return pixel.Raw;
}

static void pixel2color(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b,
		 uint8_t *x)
{
	EFI_GRAPHICS_OUTPUT_BLT_PIXEL_UNION pixel;
	pixel.Raw = color;
	*r = pixel.Pixel.Red;
	*g = pixel.Pixel.Green;
	*b = pixel.Pixel.Blue;
	*x = pixel.Pixel.Reserved;
}

void forth_o_gop_topixel(struct forth_context *fctx)
{
	uint8_t r, g, b, x;
	b = fctx->tos;
	g = forth_ppop(fctx);
	r = forth_ppop(fctx);
	x = forth_ppop(fctx);
	fctx->tos = color2pixel(r, g, b, x);
}

void forth_o_gop_tocolor(struct forth_context *fctx)
{
	uint8_t r, g, b, x;
	r = g = b = x = 0;
	pixel2color(fctx->tos, &r, &g, &b, &x);
	forth_ppush(fctx, r);
	forth_ppush(fctx, g);
	forth_ppush(fctx, b);
	fctx->tos = x;
}

void forth_init_gop(void)
{
	DFWL(L"GOP-INIT", O_GOP_INIT);
	DESC(L" ( -- GOP-CNT ) ");
	ENDW();

	DFWL(L"GOP-CNT", O_GOP_CNT);
	DESC(L" ( -- GOP-CNT ) ");
	ENDW();

	DFWL(L"GOP-SEL", O_GOP_SEL);
	DESC(L" ( GOP-IDX -- FLAG ) ");
	ENDW();

	DFWL(L"GOP-CUR", O_GOP_CUR);
	DESC(L" ( -- GOP-IDX ) ");
	ENDW();

	DFWL(L"GOP-MODE-CNT", O_GOP_MODE_CNT);
	DESC(L" ( -- GOP-MODE-CNT ) ");
	ENDW();

	DFWL(L"GOP-MODE-SEL", O_GOP_MODE_SEL);
	DESC(L" ( GOP-MODE-IDX -- FLAG ) ");
	ENDW();

	DFWL(L"GOP-MODE-CUR", O_GOP_MODE_CUR);
	DESC(L" ( -- GOP-MODE-IDX ) ");
	ENDW();

	DFWL(L"GOP-HRES", O_GOP_HRES);
	DESC(L" ( -- GOP-HRES ) ");
	ENDW();

	DFWL(L"GOP-VRES", O_GOP_VRES);
	DESC(L" ( -- GOP-VRES ) ");
	ENDW();

	DFWL(L"GOP-PLOT", O_GOP_PLOT);
	DESC(L" ( COLOR X Y -- ) ");
	ENDW();

	DFWL(L"GOP-SOLID", O_GOP_SOLID);
	DESC(L" ( COLOR X Y W H -- ) ");
	ENDW();

	CONSTANT(L"PIXEL-BLACK", color2pixel(0, 0, 0, 0xFF));
	ENDW();

	CONSTANT(L"PIXEL-RED", color2pixel(0xFF, 0, 0, 0xFF));
	ENDW();

	CONSTANT(L"PIXEL-GREEN", color2pixel(0, 0xFF, 0, 0xFF));
	ENDW();

	CONSTANT(L"PIXEL-BLUE", color2pixel(0, 0, 0xFF, 0xFF));
	ENDW();

	DFWH(L"GOP-BLANK");
	DESC(L" ( -- ) MAKE GOP SCREEN BLANK ");
	COMPILE(L"PIXEL-BLACK", L"$0", L"$0", L"GOP-HRES", L"GOP-VRES", L"GOP-SOLID",
		L"EXIT");
	ENDW();

	DFWL(L">PIXEL", O_GOP_TOPIXEL);
	DESC(L" ( R G B X -- PIXEL )");
	ENDW();

	DFWL(L">COLOR", O_GOP_TOCOLOR);
	DESC(L" ( PIXEL -- R G B X )");
	ENDW();
}
