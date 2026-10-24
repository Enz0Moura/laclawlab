#ifndef CG_CAMERA_H
#define CG_CAMERA_H

#include "cg/math.h"

typedef struct {
    Vec3 target;
    float yaw, pitch, distance, fov, near_z, far_z;
} Camera;
Camera camera_default(void);
Vec3 camera_eye(const Camera *camera);
Mat4 camera_view(const Camera *camera);
Mat4 camera_projection(const Camera *camera, float aspect);
void camera_orbit(Camera *camera, float delta_yaw, float delta_pitch);
void camera_zoom(Camera *camera, float delta);
/* x/y measured from top-left of the 3D viewport (NOT the whole UI window). */
bool camera_ray(const Camera *camera, float x, float y, int width, int height, Ray *out);
bool camera_project(const Camera *camera, Vec3 point, int width, int height, Vec3 *screen);

#endif
