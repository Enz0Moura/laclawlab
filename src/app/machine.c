#include "app/machine.h"
#include "app/theme.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const Color WHITE = {.78f, .84f, .91f}, DARK = {.10f, .13f, .24f};
static const Color RED = {.87f, .055f, .085f}, METAL = {.64f, .72f, .84f};

static Mat4 shape(Vec3 position, Vec3 size) {
    return mat4_mul(mat4_translate(position), mat4_scale(size));
}
static void message(PrizeMachine *m, const char *text) {
    snprintf(m->message, sizeof(m->message), "%s", text);
    m->message_time = 2.8f;
}
static SceneNode *add_node(SceneNode *parent, const char *name, Geometry *geometry) {
    SceneNode *node = scene_node_create(name, geometry);
    if (!node) {
        if (geometry)
            geometry->destroy(geometry);
        return NULL;
    }
    if (parent && !scene_node_add(parent, node)) {
        scene_node_destroy(node);
        return NULL;
    }
    return node;
}
static SceneNode *add_shape(SceneNode *parent, const char *name, Color color, bool cylinder,
                            Mat4 local) {
    Geometry *g = NULL;
    if (cylinder) {
        Cylinder *c = CylinderFabric.create(color, 24);
        if (c)
            g = &c->base;
    } else {
        Box *b = BoxFabric.create(color);
        if (b)
            g = &b->base;
    }
    if (!g)
        return NULL;
    SceneNode *node = add_node(parent, name, g);
    if (node)
        node->local = local;
    return node;
}
static bool solid(PrizeMachine *m, const char *name, Vec3 position, Vec3 size, Color color) {
    if (m->solid_count >= MAX_SOLIDS)
        return false;
    SceneNode *node = add_shape(m->root, name, color, false, shape(position, size));
    if (!node)
        return false;
    m->solids[m->solid_count++] = (Solid){node, name};
    return true;
}
static bool themed_solid(PrizeMachine *m, const char *name, ThemePart part) {
    if (m->solid_count >= MAX_SOLIDS)
        return false;
    Geometry *geometry = theme_cabinet_create(part);
    if (!geometry)
        return false;
    SceneNode *node = add_node(m->root, name, geometry);
    if (!node)
        return false;
    m->solids[m->solid_count++] = (Solid){node, name};
    return true;
}
static void update_world(PrizeMachine *m) {
    RoboticArm *a = &m->arm;
    const MachineConfig *c = &MACHINE_CONFIG;
    a->joint[0]->local =
        mat4_mul(mat4_translate(vec3(0, c->shoulder_height, 0)), mat4_rotate_y(a->angle[0]));
    a->joint[1]->local = mat4_rotate_z(a->angle[1]);
    a->joint[2]->local =
        mat4_mul(mat4_translate(vec3(0, c->upper_length, 0)), mat4_rotate_z(a->angle[2]));
    a->joint[3]->local =
        mat4_mul(mat4_translate(vec3(0, c->fore_length, 0)), mat4_rotate_z(a->angle[3]));
    for (int i = 0; i < 2; ++i) {
        float side = i == 0 ? -1.0f : 1.0f;
        a->finger[i]->local =
            mat4_mul(mat4_translate(vec3(side * .18f, .20f, 0)), mat4_rotate_z(-side * a->opening));
    }
    scene_update(m->root, mat4_identity());
    if (m->held >= 0) {
        Prize *p = &m->prizes[m->held];
        p->node->local = mat4_mul(a->grip->world, p->relative_to_grip);
        scene_update(p->node, mat4_identity());
    }
}
static bool build_cabinet(PrizeMachine *m) {
    if (!themed_solid(m, "piso", THEME_FLOOR) ||
        !solid(m, "fundo", vec3(1, 2.2f, -3.30f), vec3(3, 4.4f, .16f),
               (Color){.045f, .09f, .24f}) ||
        !solid(m, "coluna esquerda", vec3(-1.9f, 2.3f, 0), vec3(.16f, 4.6f, .16f), RED) ||
        !solid(m, "coluna direita", vec3(3.9f, 2.3f, 0), vec3(.16f, 4.6f, .16f), RED) ||
        !solid(m, "letreiro", vec3(1, 4.55f, -3.45f), vec3(2, .14f, .15f), RED) ||
        !solid(m, "batente", vec3(3.8f, .45f, 0), vec3(.18f, .90f, 2.4f), RED) ||
        !solid(m, "pedestal", vec3(0, .15f, 0), vec3(.8f, .30f, .8f), DARK))
        return false;
    m->tray_solid = m->solid_count;
    if (!solid(m, "bandeja", MACHINE_CONFIG.tray_center, MACHINE_CONFIG.tray_size,
               (Color){.18f, .68f, .55f}))
        return false;
    /* New structural parts stay outside the playable interior, but also
     * participate in picking/collision as conservative AABBs. */
    if (!themed_solid(m, "cupula", THEME_ROOF) || !themed_solid(m, "fuselagem", THEME_BASE) ||
        !themed_solid(m, "aleta esquerda", THEME_FIN_LEFT) ||
        !themed_solid(m, "aleta direita", THEME_FIN_RIGHT))
        return false;
    Geometry *details = theme_cabinet_create(THEME_DETAILS);
    if (!details || !add_node(m->root, "estrelas e planetas", details))
        return false;
    return true;
}
static bool build_arm(PrizeMachine *m) {
    RoboticArm *a = &m->arm;
    const MachineConfig *c = &MACHINE_CONFIG;
    SceneNode *parent = m->root;
    for (int i = 0; i < JOINT_COUNT; ++i) {
        a->angle[i] = a->target[i] = c->joints[i].initial;
        a->joint[i] = add_node(parent, c->joints[i].name, NULL);
        if (!a->joint[i])
            return false;
        parent = a->joint[i];
    }
    a->opening = a->opening_target = c->finger_max;
    a->grip = add_node(a->joint[3], "centro da garra", NULL);
    if (!a->grip)
        return false;
    a->grip->local = mat4_translate(vec3(0, c->grip_offset, 0));
    a->parts[0] =
        add_shape(a->joint[1], "braco superior", WHITE, false,
                  shape(vec3(0, c->upper_length * .5f, 0), vec3(.22f, c->upper_length, .28f)));
    a->parts[1] =
        add_shape(a->joint[2], "antebraco", WHITE, false,
                  shape(vec3(0, c->fore_length * .5f, 0), vec3(.20f, c->fore_length, .26f)));
    a->parts[2] = add_shape(a->joint[3], "palma", DARK, false,
                            shape(vec3(0, .10f, 0), vec3(.62f, .20f, .30f)));
    for (int i = 0; i < 3; ++i)
        if (!a->parts[i])
            return false;
    for (int i = 0; i < 2; ++i) {
        a->finger[i] = add_node(a->joint[3], "pivo do dedo", NULL);
        if (!a->finger[i])
            return false;
        a->parts[3 + i] = add_shape(a->finger[i], "dedo", METAL, false,
                                    shape(vec3(0, .27f, 0), vec3(.12f, .54f, .22f)));
        if (!a->parts[3 + i])
            return false;
    }
    for (int i = 1; i < JOINT_COUNT; ++i) {
        Mat4 hub = mat4_mul(mat4_rotate_x(90), mat4_scale(vec3(.42f, .36f, .42f)));
        a->parts[4 + i] = add_shape(a->joint[i], "articulacao", RED, true, hub);
        if (!a->parts[4 + i])
            return false;
    }
    a->parts[8] = add_shape(a->joint[0], "rotor", METAL, true,
                            shape(vec3(0, -.14f, 0), vec3(.65f, .30f, .65f)));
    if (!a->parts[8])
        return false;
    return true;
}
PrizeMachine *machine_create(void) {
    PrizeMachine *m = calloc(1, sizeof(*m));
    if (!m)
        return NULL;
    m->selected = m->held = m->release_contact = -1;
    m->root = scene_node_create("maquina", NULL);
    if (!m->root || !build_cabinet(m) || !build_arm(m)) {
        machine_destroy(m);
        return NULL;
    }
    const MachineConfig *c = &MACHINE_CONFIG;
    /* These fixed polar positions come from the documented pickup FK pose. */
    float radius = -c->upper_length * sinf(cg_radians(PICKUP_POSE[1])) -
                   c->fore_length * sinf(cg_radians(PICKUP_POSE[1] + PICKUP_POSE[2]));
    static const char *names[PRIZE_COUNT] = {"Coral", "Azul", "Amarelo"};
    for (int i = 0; i < PRIZE_COUNT; ++i) {
        float yaw = cg_radians(c->prize_yaw[i]);
        Mat4 pose = mat4_mul(
            mat4_translate(vec3(radius * cosf(yaw), c->prize_size * .5f, -radius * sinf(yaw))),
            mat4_rotate_y(c->prize_yaw[i]));
        Geometry *alien = theme_alien_create(c->prize_colors[i]);
        if (!alien) {
            machine_destroy(m);
            return NULL;
        }
        m->prizes[i].node = add_node(m->root, names[i], alien);
        m->prizes[i].name = names[i];
        m->prizes[i].state = PRIZE_AVAILABLE;
        if (!m->prizes[i].node) {
            machine_destroy(m);
            return NULL;
        }
        m->prizes[i].node->local =
            mat4_mul(pose, mat4_scale(vec3(c->prize_size, c->prize_size, c->prize_size)));
    }
    update_world(m);
    message(m, "Escolha um premio e aproxime a garra.");
    return m;
}
void machine_destroy(PrizeMachine *m) {
    if (m) {
        scene_node_destroy(m->root);
        free(m);
    }
}
bool machine_reset(PrizeMachine *m) {
    PrizeMachine *fresh = machine_create();
    if (!fresh)
        return false;
    SceneNode *old = m->root;
    *m = *fresh;
    free(fresh);
    scene_node_destroy(old);
    return true;
}
Vec3 machine_grip_position(const PrizeMachine *m) {
    return mat4_point(m->arm.grip->world, vec3(0, 0, 0));
}
AABB machine_delivery_region(const PrizeMachine *m) {
    AABB region = scene_bounds(m->solids[m->tray_solid].node);
    region.min.y = region.max.y;
    region.max.y += .8f;
    return region;
}
static float finger_gap(float angle) {
    float a = cg_radians(angle);
    return 2 * (.18f + .40f * sinf(a) - .06f * cosf(a));
}
static bool unobstructed(const PrizeMachine *m, Vec3 a, Vec3 b) {
    Vec3 delta = v3_sub(b, a);
    float length = v3_length(delta);
    if (length < 1e-6f)
        return true;
    Ray ray = {a, v3_scale(delta, 1 / length)};
    for (int i = 0; i < m->solid_count; ++i) {
        float t;
        if (ray_aabb(ray, scene_bounds(m->solids[i].node), &t) && t < length - .002f)
            return false;
    }
    return true;
}
int machine_capture_candidate(const PrizeMachine *m, float *required_gap) {
    if (m->held >= 0)
        return -1;
    Mat4 inverse;
    if (!mat4_inverse(m->arm.grip->world, &inverse))
        return -1;
    int best = -1;
    float nearest = FLT_MAX, gap = 0;
    for (int i = 0; i < PRIZE_COUNT; ++i)
        if (m->prizes[i].state == PRIZE_AVAILABLE) {
            const SceneNode *node = m->prizes[i].node;
            AABB local = aabb_transform(node->geometry->bounds, mat4_mul(inverse, node->world));
            Vec3 center = v3_scale(v3_add(local.min, local.max), .5f);
            float width = local.max.x - local.min.x + 2 * fabsf(center.x);
            if (fabsf(center.x) > MACHINE_CONFIG.capture_x ||
                fabsf(center.y) > MACHINE_CONFIG.capture_y ||
                fabsf(center.z) > MACHINE_CONFIG.capture_z ||
                width > finger_gap(m->arm.opening) + .025f || local.max.z - local.min.z > .64f ||
                local.max.y - local.min.y > .64f)
                continue;
            if (!unobstructed(m, machine_grip_position(m), mat4_point(node->world, vec3(0, 0, 0))))
                continue;
            float distance = v3_length(center);
            if (distance < nearest) {
                best = i;
                nearest = distance;
                gap = width;
            }
        }
    if (best >= 0 && required_gap)
        *required_gap = gap;
    return best;
}
static bool inside_workspace(AABB box) {
    AABB w = MACHINE_CONFIG.workspace;
    float t = MACHINE_CONFIG.collision_tolerance;
    return box.min.x >= w.min.x - t && box.max.x <= w.max.x + t && box.min.y >= w.min.y - t &&
           box.max.y <= w.max.y + t && box.min.z >= w.min.z - t && box.max.z <= w.max.z + t;
}
bool machine_pose_valid(const PrizeMachine *m) {
    float tolerance = MACHINE_CONFIG.collision_tolerance;
    int candidate = machine_capture_candidate(m, NULL);
    for (int part = 0; part < ARM_PART_COUNT; ++part) {
        AABB bounds = scene_bounds(m->arm.parts[part]);
        if (!inside_workspace(bounds))
            return false;
        for (int s = 0; s < m->solid_count; ++s)
            if (aabb_overlap(bounds, scene_bounds(m->solids[s].node), tolerance))
                return false;
        for (int p = 0; p < PRIZE_COUNT; ++p) {
            if (m->prizes[p].state == PRIZE_HELD)
                continue;
            /* In the finger/prize pair only, refine the world AABB in the finger's
             * local frame. This avoids a yaw-expanded box closing the empty gap. */
            if (part == 3 || part == 4) {
                if (p == candidate || p == m->release_contact)
                    continue;
                Mat4 inverse;
                if (mat4_inverse(m->arm.parts[part]->world, &inverse)) {
                    AABB local = aabb_transform(m->prizes[p].node->geometry->bounds,
                                                mat4_mul(inverse, m->prizes[p].node->world));
                    if (!aabb_overlap(local, m->arm.parts[part]->geometry->bounds, tolerance))
                        continue;
                }
            }
            if (aabb_overlap(bounds, scene_bounds(m->prizes[p].node), tolerance))
                return false;
        }
    }
    if (m->held >= 0) {
        AABB bounds = scene_bounds(m->prizes[m->held].node);
        if (!inside_workspace(bounds))
            return false;
        for (int s = 0; s < m->solid_count; ++s)
            if (aabb_overlap(bounds, scene_bounds(m->solids[s].node), tolerance))
                return false;
        for (int p = 0; p < PRIZE_COUNT; ++p)
            if (p != m->held)
                if (aabb_overlap(bounds, scene_bounds(m->prizes[p].node), tolerance))
                    return false;
    }
    return true;
}
void machine_target_joint(PrizeMachine *m, int joint, float degrees) {
    if (joint < 0 || joint >= JOINT_COUNT || !isfinite(degrees))
        return;
    const JointConfig *c = &MACHINE_CONFIG.joints[joint];
    m->arm.target[joint] = cg_clamp(degrees, c->min, c->max);
    if (degrees < c->min || degrees > c->max)
        message(m, "Limite da junta.");
}
void machine_nudge_joint(PrizeMachine *m, int joint, float degrees) {
    if (joint >= 0 && joint < JOINT_COUNT)
        machine_target_joint(m, joint, m->arm.target[joint] + degrees);
}
void machine_open(PrizeMachine *m) {
    m->arm.opening_target = MACHINE_CONFIG.finger_max;
    if (m->held >= 0) {
        Prize *p = &m->prizes[m->held];
        /* Prize always belongs to root: attachment changes transforms, not owners. */
        p->node->local = p->node->world;
        p->state = PRIZE_FALLING;
        p->fall_speed = 0;
        m->release_contact = m->held;
        m->held = -1;
        message(m, "Premio solto.");
    } else
        message(m, "Abrindo a garra.");
}
void machine_close(PrizeMachine *m) {
    if (m->held >= 0) {
        message(m, "Garra ocupada.");
        return;
    }
    m->arm.opening_target = MACHINE_CONFIG.finger_min;
    message(m, "Fechando a garra.");
}
static float approach(float value, float target, float step) {
    return value + cg_clamp(target - value, -step, step);
}
static void blocked(PrizeMachine *m) {
    ++m->blocked_moves;
    message(m, "Movimento bloqueado.");
}
static void move_joints(PrizeMachine *m, float dt) {
    for (int i = 0; i < JOINT_COUNT; ++i) {
        float old = m->arm.angle[i];
        m->arm.angle[i] = approach(old, m->arm.target[i], MACHINE_CONFIG.joints[i].speed * dt);
        if (m->arm.angle[i] == old)
            continue;
        update_world(m);
        if (!machine_pose_valid(m)) {
            m->arm.angle[i] = m->arm.target[i] = old;
            update_world(m);
            blocked(m);
        }
    }
}
static void move_fingers(PrizeMachine *m, float dt) {
    float old = m->arm.opening,
          new_angle = approach(old, m->arm.opening_target, MACHINE_CONFIG.finger_speed * dt);
    if (new_angle == old)
        return;
    float required = 0;
    int candidate = machine_capture_candidate(m, &required);
    m->arm.opening = new_angle;
    update_world(m);
    if (!machine_pose_valid(m)) {
        m->arm.opening = m->arm.opening_target = old;
        update_world(m);
        blocked(m);
        return;
    }
    if (candidate >= 0 && new_angle < old && finger_gap(new_angle) <= required + .02f) {
        Mat4 inverse;
        if (!mat4_inverse(m->arm.grip->world, &inverse))
            return;
        Prize *p = &m->prizes[candidate];
        p->relative_to_grip = mat4_mul(inverse, p->node->world);
        p->state = PRIZE_HELD;
        m->held = candidate;
        m->arm.opening_target = new_angle;
        message(m, "Premio capturado!");
    }
}
static bool horizontal_overlap(AABB a, AABB b) {
    return a.max.x > b.min.x && a.min.x < b.max.x && a.max.z > b.min.z && a.min.z < b.max.z;
}
static void fall(PrizeMachine *m, float dt) {
    for (int p = 0; p < PRIZE_COUNT; ++p)
        if (m->prizes[p].state == PRIZE_FALLING) {
            Prize *prize = &m->prizes[p];
            AABB bounds = scene_bounds(prize->node);
            prize->fall_speed += MACHINE_CONFIG.gravity * dt;
            float new_bottom = bounds.min.y - prize->fall_speed * dt, top = -FLT_MAX;
            int support = -1;
            /* Sweep the bottom through all support planes: a fall cannot tunnel. */
            for (int s = 0; s < m->solid_count + PRIZE_COUNT; ++s) {
                if (s == m->solid_count + p)
                    continue;
                if (s >= m->solid_count && m->prizes[s - m->solid_count].state != PRIZE_AVAILABLE &&
                    m->prizes[s - m->solid_count].state != PRIZE_DELIVERED)
                    continue;
                AABB obstacle = s < m->solid_count
                                    ? scene_bounds(m->solids[s].node)
                                    : scene_bounds(m->prizes[s - m->solid_count].node);
                if (horizontal_overlap(bounds, obstacle) &&
                    obstacle.max.y <= bounds.min.y + .003f && obstacle.max.y >= new_bottom &&
                    obstacle.max.y > top) {
                    top = obstacle.max.y;
                    support = s;
                }
            }
            float displacement = support >= 0 ? top - bounds.min.y : -prize->fall_speed * dt;
            prize->node->local.m[13] += displacement;
            scene_update(prize->node, mat4_identity());
            if (support >= 0) {
                prize->fall_speed = 0;
                prize->state = PRIZE_AVAILABLE;
                AABB area = machine_delivery_region(m);
                if (support == m->tray_solid && bounds.min.x >= area.min.x &&
                    bounds.max.x <= area.max.x && bounds.min.z >= area.min.z &&
                    bounds.max.z <= area.max.z) {
                    prize->state = PRIZE_DELIVERED;
                    ++m->delivered;
                    if (m->selected == p)
                        m->selected = -1;
                    message(m, m->delivered == PRIZE_COUNT
                                   ? "Todos os premios entregues! R: reiniciar."
                                   : "Entrega concluida!");
                } else
                    message(m, "Premio apoiado. Tente novamente.");
            }
        }
}
void machine_update(PrizeMachine *m, float dt) {
    if (!isfinite(dt) || dt <= 0)
        return;
    /* Pause stalls are capped; each accepted slice has bounded angular motion. */
    dt = fminf(dt, .10f);
    m->message_time = fmaxf(0, m->message_time - dt);
    int steps = (int)ceilf(dt / MACHINE_CONFIG.max_step);
    float step = dt / (float)steps;
    for (int i = 0; i < steps; ++i) {
        move_joints(m, step);
        move_fingers(m, step);
        fall(m, step);
        if (m->release_contact >= 0) {
            AABB p = scene_bounds(m->prizes[m->release_contact].node);
            if (!aabb_overlap(p, scene_bounds(m->arm.parts[3]), 0) &&
                !aabb_overlap(p, scene_bounds(m->arm.parts[4]), 0))
                m->release_contact = -1;
        }
    }
}
int machine_pick(PrizeMachine *m, Ray ray) {
    float closest = FLT_MAX;
    int selected = -1;
    for (int s = 0; s < m->solid_count; ++s) {
        float distance;
        if (ray_aabb(ray, scene_bounds(m->solids[s].node), &distance))
            closest = fminf(closest, distance);
    }
    for (int p = 0; p < ARM_PART_COUNT; ++p) {
        float distance;
        if (ray_aabb(ray, scene_bounds(m->arm.parts[p]), &distance))
            closest = fminf(closest, distance);
    }
    /* Held/delivered prizes occlude too, although they are not selectable. */
    for (int p = 0; p < PRIZE_COUNT; ++p)
        if (m->prizes[p].state != PRIZE_AVAILABLE) {
            float distance;
            if (ray_aabb(ray, scene_bounds(m->prizes[p].node), &distance))
                closest = fminf(closest, distance);
        }
    for (int p = 0; p < PRIZE_COUNT; ++p)
        if (m->prizes[p].state == PRIZE_AVAILABLE) {
            float distance;
            if (ray_aabb(ray, scene_bounds(m->prizes[p].node), &distance) && distance < closest) {
                selected = p;
                closest = distance;
            }
        }
    m->selected = selected;
    if (selected >= 0)
        message(m, "Premio selecionado. Controle as juntas.");
    else
        message(m, "Nenhum premio selecionado.");
    return selected;
}
bool machine_at_target(const PrizeMachine *m) {
    for (int i = 0; i < JOINT_COUNT; ++i)
        if (fabsf(m->arm.angle[i] - m->arm.target[i]) > .001f)
            return false;
    return fabsf(m->arm.opening - m->arm.opening_target) < .001f;
}
const char *prize_state_name(PrizeState state) {
    switch (state) {
    case PRIZE_AVAILABLE:
        return "Disponivel";
    case PRIZE_HELD:
        return "Na garra";
    case PRIZE_FALLING:
        return "Em queda";
    case PRIZE_DELIVERED:
        return "Entregue";
    }
    return "?";
}
