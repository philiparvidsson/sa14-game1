#include "shape.h"

#include "base/common.h"
#include "math/aabb.h"
#include "math/vector.h"

#include <stdlib.h>

shapeT* shapeNew(int num_points) {
    assert(0 <= num_points && num_points < ShapeMaxPoints)

    // One point is included in sizeof(shapeT) so we can subtract it.
    shapeT* shape = calloc(1, sizeof(shapeT) + sizeof(vec2) * num_points-1);

    shape->num_points = num_points;

    return (shape);
}

shapeT* shapeNewSquare(float width, float height) {
    width  /= 2.0f;
    height /= 2.0f;

    shapeT* square = shapeNew(4);

    square->points[0] = (vec2) {  width,  height };
    square->points[1] = (vec2) { -width,  height };
    square->points[2] = (vec2) { -width, -height };
    square->points[3] = (vec2) {  width, -height };

    return (square);
}

void shapeFree(shapeT* shape) {
    free(shape->points);
    free(shape);
}

aabbT shapeAABB(const shapeT* shape) {
    aabbT aabb = { 0 };

    for (int i = 0; i < shape->num_points; i++) {
        vec2* p = &shape->points[i];

        aabb.min.x = min(aabb.min.x, p->x);
        aabb.min.y = min(aabb.min.y, p->y);
        aabb.max.x = max(aabb.max.x, p->x);
        aabb.max.y = max(aabb.max.y, p->y);
    }

    return (aabb);
}
