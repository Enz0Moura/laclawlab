#include "app/ui.h"
#include "cg/render.h"

#include <GL/freeglut.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static PrizeMachine *machine;
static UserInterface ui;
static int last_time, drag_x, drag_y, held_direction;
static bool dragging, smoke;
static const char *screenshot;
static int exit_code = EXIT_SUCCESS, smoke_phase, smoke_prize, smoke_frames;
static float smoke_wait;
static const char *pending_capture;
static int smoke_menu_phase;

static void choose_mode(GameMode mode) {
    if (!machine_start(machine, mode)) {
        fprintf(stderr, "Sem memoria para iniciar; partida anterior preservada.\n");
        return;
    }
    ui.menu = ui.help = false;
    ui.camera = camera_default();
    ui.active_joint = held_direction = 0;
    dragging = false;
}
static void open_menu(void) {
    ui.menu = true;
    ui.help = false;
    held_direction = 0;
    dragging = false;
}

static void cleanup(void) {
    machine_destroy(machine);
    machine = NULL;
}
static void quit_failure(const char *why) {
    fprintf(stderr, "FAIL graphical smoke: %s\n", why);
    if (machine) {
        for (int p = 0; p < PRIZE_COUNT; ++p) {
            Vec3 pos = mat4_point(machine->prizes[p].node->world, vec3(0, 0, 0));
            Vec3 v = machine->prizes[p].velocity;
            fprintf(stderr, "prize %d state %d position %.3f %.3f %.3f velocity %.3f %.3f %.3f\n",
                    p, machine->prizes[p].state, (double)pos.x, (double)pos.y, (double)pos.z,
                    (double)v.x, (double)v.y, (double)v.z);
        }
    }
    exit_code = EXIT_FAILURE;
    glutLeaveMainLoop();
}
static void reset(void) {
    if (!machine_reset(machine)) {
        fprintf(stderr, "Sem memoria para reiniciar; cena anterior preservada.\n");
        return;
    }
    ui.camera = camera_default();
    ui.active_joint = 0;
    held_direction = 0;
    dragging = false;
}
static void key(unsigned char k, int x, int y) {
    (void)x;
    (void)y;
    k = (unsigned char)tolower(k);
    if (ui.menu) {
        if (k >= '1' && k <= '4')
            choose_mode((GameMode)(k - '1'));
        else if (k == 27)
            glutLeaveMainLoop();
        glutPostRedisplay();
        return;
    }
    if (k == 'm') {
        open_menu();
        return;
    }
    if (ui.help && k != 'h' && k != 27)
        return;
    if (k >= '1' && k <= '4') {
        ui.active_joint = k - '1';
        held_direction = 0;
    }
    switch (k) {
    case 27:
        glutLeaveMainLoop();
        break;
    case '+':
    case '=':
        machine_nudge_joint(machine, ui.active_joint, 5);
        break;
    case '-':
    case '_':
        machine_nudge_joint(machine, ui.active_joint, -5);
        break;
    case '[':
        machine_nudge_joint(machine, ui.active_joint, -1);
        break;
    case ']':
        machine_nudge_joint(machine, ui.active_joint, 1);
        break;
    case 'c':
        machine_close(machine);
        break;
    case 'o':
        machine_open(machine);
        break;
    case ' ':
        if (machine->held >= 0 || machine->arm.opening_target < 20)
            machine_open(machine);
        else
            machine_close(machine);
        break;
    case 'r':
        reset();
        break;
    case 'h':
        ui.help = !ui.help;
        held_direction = 0;
        dragging = false;
        break;
    case 'd':
        ui.debug = !ui.debug;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}
static void special(int k, int x, int y) {
    (void)x;
    (void)y;
    if (ui.menu)
        return;
    if (k == GLUT_KEY_F1) {
        key('h', 0, 0);
        return;
    }
    if (ui.help)
        return;
    if (k == GLUT_KEY_LEFT)
        held_direction = -1;
    if (k == GLUT_KEY_RIGHT)
        held_direction = 1;
    if (k == GLUT_KEY_HOME)
        ui.camera = camera_default();
}
static void special_up(int k, int x, int y) {
    (void)x;
    (void)y;
    if ((k == GLUT_KEY_LEFT && held_direction < 0) || (k == GLUT_KEY_RIGHT && held_direction > 0)) {
        held_direction = 0;
        /* Releasing a held key stops its remaining target immediately. */
        machine->arm.target[ui.active_joint] = machine->arm.angle[ui.active_joint];
    }
}
static void mouse(int button, int state, int x, int y) {
    if (ui.menu) {
        int mode = ui_mode_at(&ui, x, y);
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && mode >= 0)
            choose_mode((GameMode)mode);
        return;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && y >= 23 && y <= 57 &&
        x >= ui.width - 155 && x <= ui.width - 25) {
        open_menu();
        return;
    }
    if (button == GLUT_RIGHT_BUTTON) {
        dragging = state == GLUT_DOWN && !ui.help && x < ui_view_width(&ui) && y >= UI_TOP &&
                   y < ui.height - UI_BOTTOM;
        drag_x = x;
        drag_y = y;
        return;
    }
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
        return;
    if (x >= ui_view_width(&ui)) {
        unsigned char command = ui_click(&ui, x, y);
        if (command >= 'a' && command <= 'd') {
            ui.active_joint = command - 'a';
            key('-', x, y);
        } else if (command >= 'e' && command <= 'h') {
            ui.active_joint = command - 'e';
            key('+', x, y);
        } else if (command)
            key(command, x, y);
    } else if (!ui.help && y >= UI_TOP && y < ui.height - UI_BOTTOM) {
        Ray ray;
        if (camera_ray(&ui.camera, (float)x, (float)(y - UI_TOP), ui_view_width(&ui),
                       ui_view_height(&ui), &ray))
            machine_pick(machine, ray);
    }
}
static void motion(int x, int y) {
    if (dragging)
        camera_orbit(&ui.camera, -(float)(x - drag_x) * .35f, (float)(y - drag_y) * .25f);
    drag_x = x;
    drag_y = y;
}
static void wheel(int wheel_number, int direction, int x, int y) {
    (void)wheel_number;
    if (!ui.menu && !ui.help && x < ui_view_width(&ui) && y >= UI_TOP && y < ui.height - UI_BOTTOM)
        camera_zoom(&ui.camera, -(float)direction * .6f);
}
static void entry(int state) {
    if (state == GLUT_LEFT) {
        held_direction = 0;
        dragging = false;
    }
}
static void reshape(int width, int height) {
    ui.width = width > 0 ? width : 1;
    ui.height = height > 0 ? height : 1;
}
static void smoke_target(int joint, float value) {
    key((unsigned char)('1' + joint), 0, 0);
    int steps = (int)lroundf((value - machine->arm.target[joint]) / 5);
    for (int i = 0; i < abs(steps); ++i)
        key(steps > 0 ? '+' : '-', 0, 0);
}
static void smoke_step(float dt) {
    if (smoke_menu_phase < 2) {
        if (!ui.menu) {
            quit_failure("initial mode menu");
            return;
        }
        if (smoke_menu_phase == 0) {
            key('c', 0, 0);
            if (machine->arm.opening_target != MACHINE_CONFIG.finger_max) {
                quit_failure("menu input isolation");
                return;
            }
            for (int i = 0; i < MODE_COUNT; ++i) {
                UIRect r = ui_mode_rect(&ui, i);
                mouse(GLUT_LEFT_BUTTON, GLUT_DOWN, (int)(r.x + r.w / 2), (int)(r.y + r.h / 2));
                if (ui.menu || machine->mode != (GameMode)i ||
                    machine->time_remaining != GAME_MODES[i].time_limit) {
                    quit_failure("mode mouse selection");
                    return;
                }
                key('m', 0, 0);
            }
            pending_capture = "build/smoke_menu.ppm";
        } else {
            if (machine->time_remaining != GAME_MODES[MODE_HARD].time_limit) {
                quit_failure("paused menu clock");
                return;
            }
            key('1', 0, 0);
        }
        ++smoke_menu_phase;
        return;
    }
    if (++smoke_frames > 2400) {
        quit_failure("timeout");
        return;
    }
    smoke_wait += dt;
    if (!machine_at_target(machine) || smoke_wait < .12f)
        return;
    smoke_wait = 0;
    if (machine->blocked_moves && smoke_phase < 12) {
        quit_failure("documented route blocked");
        return;
    }
    switch (smoke_phase) {
    case 0: {
        if (smoke_prize == 0)
            pending_capture = "build/smoke_initial.ppm";
        Vec3 screen;
        if (!camera_project(&ui.camera,
                            mat4_point(machine->prizes[smoke_prize].node->world, vec3(0, 0, 0)),
                            ui_view_width(&ui), ui_view_height(&ui), &screen)) {
            quit_failure("project prize");
            return;
        }
        mouse(GLUT_LEFT_BUTTON, GLUT_DOWN, (int)lroundf(screen.x), (int)lroundf(screen.y) + UI_TOP);
        if (machine->selected != smoke_prize) {
            quit_failure("mouse picking");
            return;
        }
        smoke_target(0, MACHINE_CONFIG.prize_yaw[smoke_prize]);
        break;
    }
    case 1:
        smoke_target(3, -50);
        break;
    case 2:
        smoke_target(1, -45);
        break;
    case 3:
        key('c', 0, 0);
        break;
    case 4:
        if (machine->held != smoke_prize) {
            quit_failure("capture");
            return;
        }
        if (smoke_prize == 0)
            pending_capture = "build/smoke_pickup.ppm";
        smoke_target(1, -25);
        break;
    case 5:
        smoke_target(3, -70);
        break;
    case 6:
        smoke_target(0, MACHINE_CONFIG.drop_yaw[smoke_prize]);
        break;
    case 7:
        if (machine->delivered != smoke_prize) {
            quit_failure("count before release");
            return;
        }
        key('o', 0, 0);
        break;
    case 8:
        if (machine->prizes[smoke_prize].state == PRIZE_FALLING)
            return;
        if (machine->delivered != smoke_prize + 1) {
            quit_failure("delivery");
            return;
        }
        if (++smoke_prize < PRIZE_COUNT) {
            smoke_phase = 0;
            return;
        }
        pending_capture = "build/smoke_delivery.ppm";
        break;
    case 9:
        mouse(GLUT_RIGHT_BUTTON, GLUT_DOWN, 300, 300);
        motion(390, 330);
        mouse(GLUT_RIGHT_BUTTON, GLUT_UP, 390, 330);
        wheel(0, 1, 300, 300);
        special(GLUT_KEY_HOME, 0, 0);
        key('d', 0, 0);
        if (!ui.debug || fabsf(ui.camera.distance - camera_default().distance) > .001f) {
            quit_failure("camera/debug");
            return;
        }
        pending_capture = "build/smoke_debug.ppm";
        break;
    case 10:
        key('h', 0, 0);
        pending_capture = "build/smoke_help.ppm";
        break;
    case 11:
        key('h', 0, 0);
        key('r', 0, 0);
        smoke_target(1, -85);
        break;
    case 12:
        if (!machine->blocked_moves || !machine_pose_valid(machine)) {
            quit_failure("collision rejection");
            return;
        }
        key('r', 0, 0);
        if (machine->delivered || machine->held != -1 || machine->selected != -1) {
            quit_failure("reset");
            return;
        }
        key('m', 0, 0);
        key('4', 0, 0);
        machine->time_remaining = .01f;
        break;
    case 13:
        if (machine->round != ROUND_LOST) {
            quit_failure("timed round expiration");
            return;
        }
        pending_capture = "build/smoke_timeout.ppm";
        break;
    case 14:
        key('r', 0, 0);
        if (machine->round != ROUND_PLAYING || machine->mode != MODE_HARD ||
            machine->time_remaining != GAME_MODES[MODE_HARD].time_limit) {
            quit_failure("retry same difficulty");
            return;
        }
        puts("prize_machine: graphical session passed (menu, modes, pause, all 3 prizes, "
             "camera, collision, delivery, timeout, retry)");
        glutLeaveMainLoop();
        break;
    default:
        break;
    }
    ++smoke_phase;
}
static void display(void) {
    ui_draw(&ui, machine);
    if (smoke && glGetError() != GL_NO_ERROR) {
        quit_failure("OpenGL error");
        return;
    }
    if (pending_capture || screenshot) {
        const char *path = pending_capture ? pending_capture : screenshot;
        if (!render_save_ppm(path, ui.width, ui.height)) {
            fprintf(stderr, "Nao foi possivel gravar %s\n", path);
            exit_code = EXIT_FAILURE;
        }
        pending_capture = NULL;
        if (screenshot)
            glutLeaveMainLoop();
    }
    glutSwapBuffers();
}
static void timer(int value) {
    (void)value;
    if (!machine)
        return;
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = smoke ? 1.0f / 60 : fmaxf((float)(now - last_time) * .001f, 0);
    last_time = now;
    if (held_direction && !ui.menu && !ui.help)
        machine_target_joint(machine, ui.active_joint,
                             machine->arm.angle[ui.active_joint] +
                                 (float)held_direction *
                                     MACHINE_CONFIG.joints[ui.active_joint].speed * fminf(dt, .1f));
    if (!ui.menu && !ui.help)
        machine_update(machine, dt);
    if (smoke)
        smoke_step(dt);
    glutPostRedisplay();
    glutTimerFunc(smoke ? 1 : 16, timer, 0);
}
int main(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--smoke-test") == 0)
            smoke = true;
        else if (strcmp(argv[i], "--screenshot") == 0 && i + 1 < argc)
            screenshot = argv[++i];
        else {
            fprintf(stderr, "Uso: %s [--smoke-test | --screenshot arquivo.ppm]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    machine = machine_create();
    if (!machine) {
        fprintf(stderr, "Sem memoria para construir a cena.\n");
        return EXIT_FAILURE;
    }
    ui = (UserInterface){.width = 1280, .height = 800, .menu = true, .camera = camera_default()};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(ui.width, ui.height);
    glutCreateWindow("Pizza Planet | LaClaw Lab");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutIgnoreKeyRepeat(1);
    render_init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutSpecialUpFunc(special_up);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMouseWheelFunc(wheel);
    glutEntryFunc(entry);
    glutCloseFunc(cleanup);
    last_time = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    cleanup();
    return exit_code;
}
