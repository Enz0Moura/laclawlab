#ifndef CG_RENDER_H
#define CG_RENDER_H

#include "cg/camera.h"
#include "cg/geometry.h"

void render_init(void);
void render_camera(const Camera *camera, int x, int y, int width, int height);
void render_aabb(AABB box, Color color);
void render_axes(Mat4 world, float length);
void render_line(Vec3 a, Vec3 b, Color color);
/* Overlay coordinates start at top-left. Pair begin/end; needs compatibility GL. */
void render_overlay_begin(int width, int height);
void render_overlay_end(void);
void render_rect(float x, float y, float w, float h, Color color);
void render_text(float x, float y, const char *text, Color color, bool large);
bool render_save_ppm(const char *path, int width, int height); /* Current back buffer. */

#endif
