#include "cg/render.h"
#include "cg/scene.h"

#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SceneNode *root, *pivot;
static Camera camera;
static int width = 900, height = 650, last_time, frames;
static bool smoke;
static float angle;

/* Adoption transfers ownership. This helper also cleans up failed construction. */
static SceneNode *adopt(SceneNode *parent, Geometry *geometry) {
    if (!geometry)
        return NULL;
    SceneNode *node = scene_node_create(geometry->name, geometry);
    if (!node) {
        geometry->destroy(geometry);
        return NULL;
    }
    if (!scene_node_add(parent, node)) {
        scene_node_destroy(node);
        return NULL;
    }
    return node;
}
static void cleanup(void) {
    scene_node_destroy(root);
    root = NULL;
}
static void draw(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_camera(&camera, 0, 0, width, height);
    scene_draw(root);
    render_axes(pivot->world, 1.8f);
    render_overlay_begin(width, height);
    render_text(24, 32, "libcg | Box + Cylinder", (Color){.9f, .95f, 1}, true);
    render_text(24, 56, "Filho orbita com o pai. Setas: camera | +/-: zoom | Esc: sair",
                (Color){.6f, .8f, .9f}, false);
    render_overlay_end();
    if (smoke && frames == 30) {
        if (glGetError() != GL_NO_ERROR) {
            fprintf(stderr, "OpenGL error in example\n");
            exit(EXIT_FAILURE);
        }
        puts("library_demo: 30 frames, two geometries and hierarchy rendered successfully");
        glutLeaveMainLoop();
    }
    glutSwapBuffers();
    ++frames;
}
static void timer(int value) {
    (void)value;
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = cg_clamp((float)(now - last_time) * .001f, 0, .05f);
    last_time = now;
    angle += 25 * dt;
    pivot->local = mat4_rotate_y(angle);
    scene_update(root, mat4_identity());
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}
static void reshape(int w, int h) {
    width = w > 0 ? w : 1;
    height = h > 0 ? h : 1;
}
static void key(unsigned char k, int x, int y) {
    (void)x;
    (void)y;
    if (k == 27)
        glutLeaveMainLoop();
    if (k == '+' || k == '=')
        camera_zoom(&camera, -.5f);
    if (k == '-')
        camera_zoom(&camera, .5f);
}
static void special(int k, int x, int y) {
    (void)x;
    (void)y;
    if (k == GLUT_KEY_LEFT)
        camera_orbit(&camera, -5, 0);
    if (k == GLUT_KEY_RIGHT)
        camera_orbit(&camera, 5, 0);
    if (k == GLUT_KEY_UP)
        camera_orbit(&camera, 0, 5);
    if (k == GLUT_KEY_DOWN)
        camera_orbit(&camera, 0, -5);
}
int main(int argc, char **argv) {
    smoke = argc == 2 && strcmp(argv[1], "--smoke-test") == 0;
    root = scene_node_create("root", NULL);
    pivot = scene_node_create("pivot", NULL);
    if (!root || !pivot) {
        scene_node_destroy(pivot);
        cleanup();
        return EXIT_FAILURE;
    }
    if (!scene_node_add(root, pivot)) {
        scene_node_destroy(pivot);
        cleanup();
        return EXIT_FAILURE;
    }
    Box *box = BoxFabric.create((Color){.1f, .8f, .7f});
    SceneNode *box_node = adopt(pivot, box ? &box->base : NULL);
    Cylinder *cylinder = CylinderFabric.create((Color){1, .6f, .2f}, 24);
    SceneNode *child = adopt(pivot, cylinder ? &cylinder->base : NULL);
    if (!box_node || !child) {
        cleanup();
        return EXIT_FAILURE;
    }
    /* The scaled mesh is a sibling of the orbiting child, not its parent. */
    box_node->local = mat4_scale(vec3(.8f, 2, .8f));
    child->local = mat4_mul(mat4_translate(vec3(2, 0, 0)), mat4_scale(vec3(.8f, .6f, .8f)));
    Geometry *interface = &box->base;
    interface->set_color(interface, (Color){.15f, .85f, .75f});
    camera = camera_default();
    camera.target = vec3(0, 0, 0);
    camera.distance = 8;
    scene_update(root, mat4_identity());
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("libcg - exemplo independente");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    render_init();
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutCloseFunc(cleanup);
    last_time = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    cleanup();
    return EXIT_SUCCESS;
}
