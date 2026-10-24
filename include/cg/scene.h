#ifndef CG_SCENE_H
#define CG_SCENE_H

#include "cg/geometry.h"

typedef struct SceneNode SceneNode;
struct SceneNode {
    Mat4 local, world;
    Geometry *geometry; /* Exclusive ownership; may be NULL for a joint. */
    const char *name;   /* Borrowed: must outlive the node. */
    SceneNode *parent, *first_child, *next_sibling;
};
/* On success the node owns geometry. On failure ownership stays with caller. */
SceneNode *scene_node_create(const char *name, Geometry *geometry);
/* Only an unattached child may be adopted. Rejects cycles; no change on failure. */
bool scene_node_add(SceneNode *parent, SceneNode *child);
/* Destroy only a detached subtree/root. Recursively frees nodes and geometry. */
void scene_node_destroy(SceneNode *root);
void scene_update(SceneNode *root, Mat4 parent_world);
void scene_draw(SceneNode *root);         /* Camera view must already be loaded. */
AABB scene_bounds(const SceneNode *node); /* Requires non-NULL geometry. */

#endif
