#include "material.h"

#include "base/common.h"
#include "math/vector.h"

#include <string.h>

// @To-do: Materials should probably be loaded from files.

const materialT* defaultMaterial(void) {
    static const materialT default_material = {
        0.1f, 0.1f, 0.1f, // ambient
        0.5f, 0.5f, 0.5f, // diffuse
        0.4f, 0.4f, 0.4f, // specular
        1.0f              // shininess
    };

    return (&default_material);
}

const materialT* getNamedMaterial(const string* name) {
    const materialT* material = NULL;

    if (strcmp(name, "default")==0)
        return (defaultMaterial());

    // This is a debug material to visualize objects more clearly.
#ifdef _DEBUG
    if (strcmp(name, "debug")==0) {
        static const materialT debug_material = {
            1.0f, 0.0f, 1.0f, // ambient
            0.5f, 0.5f, 0.5f, // diffuse
            0.5f, 0.5f, 0.5f, // specular
            10.0f,             // shininess
        };

        material = &debug_material;
    }
#endif // _DEBUG

    if (strcmp(name, "shiny black")==0) {
        static const materialT shiny_black = {
            0.0f, 0.0f, 0.0f, // ambient
            0.1f, 0.1f, 0.1f, // diffuse
            1.0f, 1.0f, 1.0f, // specular
            10.0f             // shininess
        };

        material = &shiny_black;
    }
    
    if (!material)
        error("Unknown material: '%s'", name);

    return (material);
}