#include "cg/render.h"

#include <GL/freeglut.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void render_init(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE); /* GL uses inverse-transpose modelview, then normalizes. */
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    const float ambient[] = {.25f, .28f, .34f, 1}, diffuse[] = {.88f, .86f, .80f, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glClearColor(.027f, .040f, .066f, 1);
}
void render_camera(const Camera *c, int x, int y, int width, int height) {
    Mat4 projection = camera_projection(c, (float)width / (float)height), view = camera_view(c);
    glViewport(x, y, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.m);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.m);
    const float light[] = {2, 12, 6, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light);
}
void render_line(Vec3 a, Vec3 b, Color color) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_LINES);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glEnd();
    glPopAttrib();
}
void render_aabb(AABB b, Color color) {
    Vec3 corners[8];
    for (int i = 0; i < 8; ++i)
        corners[i] =
            vec3(i & 1 ? b.max.x : b.min.x, i & 2 ? b.max.y : b.min.y, i & 4 ? b.max.z : b.min.z);
    for (int i = 0; i < 8; ++i)
        for (int bit = 1; bit <= 4; bit *= 2)
            if (!(i & bit))
                render_line(corners[i], corners[i | bit], color);
}
void render_axes(Mat4 world, float length) {
    Vec3 origin = mat4_point(world, vec3(0, 0, 0));
    render_line(origin, mat4_point(world, vec3(length, 0, 0)), (Color){1, .2f, .2f});
    render_line(origin, mat4_point(world, vec3(0, length, 0)), (Color){.2f, 1, .3f});
    render_line(origin, mat4_point(world, vec3(0, 0, length)), (Color){.2f, .5f, 1});
}
void render_overlay_begin(int width, int height) {
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_LINE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}
void render_overlay_end(void) {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}
void render_rect(float x, float y, float w, float h, Color c) {
    glColor3f(c.r, c.g, c.b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}
void render_text(float x, float y, const char *text, Color c, bool large) {
    glColor3f(c.r, c.g, c.b);
    glRasterPos2f(x, y);
    glutBitmapString(large ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12,
                     (const unsigned char *)text);
}
bool render_save_ppm(const char *path, int width, int height) {
    if (width <= 0 || height <= 0 || (size_t)width > SIZE_MAX / 3 / (size_t)height)
        return false;
    size_t stride = (size_t)width * 3, size = stride * (size_t)height;
    unsigned char *pixels = malloc(size);
    if (!pixels)
        return false;
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    FILE *file = fopen(path, "wb");
    if (!file) {
        free(pixels);
        return false;
    }
    bool ok = fprintf(file, "P6\n%d %d\n255\n", width, height) > 0;
    for (int row = height - 1; row >= 0; --row)
        if (fwrite(pixels + (size_t)row * stride, 1, stride, file) != stride)
            ok = false;
    if (fclose(file) != 0)
        ok = false;
    free(pixels);
    return ok;
}
