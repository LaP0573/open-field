// Base 31's options page: the stroke font, lines, arcs and rounded boxes in the four knob colours, OP-1 line art. The
// same drawing our own packs' screens use, cut to what the page draws. Drawn from scratch: no glyph or picture of the
// unit's own. Needs, before it: u8 / u16 and px(u16* fb, int x, int y, u16 c) for the frame buffer
// (480 x 216, index y + ((479 - x) << 8)).
#pragma once
static const u16 KC[4] = {0x3BDF, 0x3FED, 0xFFFF, 0xFC40};             // encoder colours, 1st..4th
static const u16 LAV = 0xCE9F, DIMG = 0x4208;                            // structure lines, empty / off
static inline u16 dimc(u16 c){ return (u16)((((c >> 11) * 3 >> 3) << 11) | ((((c >> 5) & 63) * 3 >> 3) << 5) | ((c & 31) * 3 >> 3)); }
static inline int iabs(int v){ return v < 0 ? -v : v; }
static const char kVChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+/%#b<>.^= ";
static const u16 kVOff[48] = {0,9,23,32,40,48,55,66,75,84,90,97,101,107,112,122,130,143,154,167,173,180,184,190,196,203,208,221,228,236,246,251,260,271,275,294,305,308,314,317,332,344,352,356,360,363,367,373};
static const u8 kVData[374] = {7,3,17,35,39,127,5,37,0,7,1,25,34,35,28,4,127,28,37,38,31,7,0,34,25,9,2,6,15,31,38,0,1,7,31,38,34,25,1,0,33,1,7,39,127,4,28,0,33,1,7,127,4,28,0,34,25,9,2,6,15,31,38,36,20,0,1,7,127,33,39,127,4,36,0,9,25,127,17,23,127,15,31,0,33,38,31,15,6,0,1,7,127,33,4,39,0,1,7,39,0,7,1,20,33,39,0,7,1,39,33,0,9,25,34,38,31,15,6,2,9,0,7,1,25,34,35,28,4,0,9,25,34,38,31,15,6,2,9,127,21,39,0,7,1,25,34,35,28,4,127,20,39,0,34,25,9,2,3,12,28,37,38,31,15,6,0,1,33,127,17,23,0,1,6,15,31,38,33,0,1,23,33,0,1,15,20,31,33,0,1,39,127,33,7,0,1,20,33,127,20,23,0,1,33,7,39,0,9,25,34,38,31,15,6,2,9,127,34,6,0,10,17,23,127,15,31,0,2,9,25,34,35,7,39,0,1,33,19,27,36,38,31,15,6,0,31,25,5,37,0,33,1,4,28,37,38,31,7,0,25,9,2,6,15,31,38,37,28,4,0,1,33,15,0,9,25,34,35,28,12,3,2,9,127,12,5,6,15,31,38,37,28,0,36,12,3,2,9,25,34,38,31,15,0,12,28,0,4,36,127,18,22,0,33,7,0,7,33,127,1,9,10,2,1,127,30,38,39,31,30,0,9,15,127,25,31,127,3,35,127,5,37,0,1,7,23,30,29,20,4,0,26,4,30,0,10,36,14,0,22,23,0,4,18,36,0,3,35,127,5,37,0,0};
static const short kSin64[65] = {0,402,804,1205,1606,2006,2404,2801,3196,3590,3981,4370,4756,5139,5520,5897,6270,6639,7005,7366,7723,8076,8423,8765,9102,9434,9760,10080,10394,10702,11003,11297,11585,11866,12140,12406,12665,12916,13160,13395,13623,13842,14053,14256,14449,14635,14811,14978,15137,15286,15426,15557,15679,15791,15893,15986,16069,16143,16207,16261,16305,16340,16364,16379,16384};   // sin over a quarter turn, Q14
static int isin(int a){ a &= 255; const int i = a & 63; switch (a >> 6) { case 0: return kSin64[i]; case 1: return kSin64[64 - i]; case 2: return -kSin64[i]; default: return -kSin64[64 - i]; } }   // 256 a turn, Q14
static inline int icos(int a){ return isin(a + 64); }
static void dotw(u16* fb, int x, int y, int t, u16 c){ for (int j = 0; j < t; ++j) for (int i = 0; i < t; ++i) px(fb, x + i, y + j, c); }
static void ln(u16* fb, int x0, int y0, int x1, int y1, u16 c, int t = 2){
    const int dx = iabs(x1 - x0), dy = -iabs(y1 - y0), sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1; int e = dx + dy;
    for (;;) { dotw(fb, x0, y0, t, c); if (x0 == x1 && y0 == y1) break; const int e2 = 2 * e; if (e2 >= dy) { e += dy; x0 += sx; } if (e2 <= dx) { e += dx; y0 += sy; } }
}
// An arc from angle a0 to a1 (256 a turn, 0 = right, 64 = down), radius r.
static void arc(u16* fb, int cx, int cy, int r, int a0, int a1, u16 c, int t = 2){
    const int st = r > 40 ? 2 : 4; int px0 = cx + (icos(a0) * r >> 14), py0 = cy + (isin(a0) * r >> 14);
    for (int a = a0 + st; ; a += st) { if (a > a1) a = a1; const int x = cx + (icos(a) * r >> 14), y = cy + (isin(a) * r >> 14); ln(fb, px0, py0, x, y, c, t); px0 = x; py0 = y; if (a == a1) break; }
}
static void rbox(u16* fb, int x0, int y0, int x1, int y1, u16 c, int t = 2, bool dashed = false){   // rounded box, corner radius 6
    const int r = 6;
    if (dashed) { for (int x = x0 + r; x < x1 - r; x += 8) { ln(fb, x, y0, x + 3, y0, c, t); ln(fb, x, y1, x + 3, y1, c, t); }
                  for (int y = y0 + r; y < y1 - r; y += 8) { ln(fb, x0, y, x0, y + 3, c, t); ln(fb, x1, y, x1, y + 3, c, t); } }
    else { ln(fb, x0 + r, y0, x1 - r, y0, c, t); ln(fb, x0 + r, y1, x1 - r, y1, c, t); ln(fb, x0, y0 + r, x0, y1 - r, c, t); ln(fb, x1, y0 + r, x1, y1 - r, c, t); }
    arc(fb, x0 + r, y0 + r, r, 128, 192, c, t); arc(fb, x1 - r, y0 + r, r, 192, 256, c, t); arc(fb, x1 - r, y1 - r, r, 0, 64, c, t); arc(fb, x0 + r, y1 - r, r, 64, 128, c, t);
}
// Stroke text: q = quarter pixels per font unit (a glyph is 4 x 6 units, 6 apart). vtext_w gives the width.
static int vtext_w(const char* s, int q){ int n = 0; while (s[n]) ++n; return n ? (n * 6 - 2) * q / 4 : 0; }
static void vtext(u16* fb, int x, int y, const char* s, int q, u16 c, int t = 2){
    for (; *s; ++s, x += 6 * q / 4) {
        char ch = *s; if (ch >= 'a' && ch <= 'z' && ch != 'b') ch = (char)(ch - 32);
        int g = -1; for (int k = 0; kVChars[k]; ++k) if (kVChars[k] == ch) { g = k; break; }
        if (g < 0) continue;
        int lx = -1, ly = -1;
        for (const u8* d = kVData + kVOff[g]; *d; ++d) {
            if (*d == 0x7F) { lx = -1; continue; }
            const int gx = x + ((*d - 1) >> 3) * q / 4, gy = y + ((*d - 1) & 7) * q / 4;
            if (lx >= 0) ln(fb, lx, ly, gx, gy, c, t); else dotw(fb, gx, gy, t, c);
            lx = gx; ly = gy;
        }
    }
}
