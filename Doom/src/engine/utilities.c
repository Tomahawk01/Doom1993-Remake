#include "engine/utilities.h"
#include "engine/state.h"
#include "map.h"
#include "math/matrix.h"
#include "math/vector.h"

#include <stdbool.h>

void insert_stencil_quad(mat4 transformation)
{
    if (stencil_ls.head == NULL)
    {
        stencil_ls.head = malloc(sizeof(stencil_node));
        *stencil_ls.head = (stencil_node){ transformation, NULL };

        stencil_ls.tail = stencil_ls.head;
    }
    else
    {
        stencil_ls.tail->next = malloc(sizeof(stencil_node));
        *stencil_ls.tail->next = (stencil_node){ transformation, NULL };

        stencil_ls.tail = stencil_ls.tail->next;
    }
}

sector* map_get_sector(vec2 position)
{
    uint16_t id = gl_m.num_nodes - 1;
    while ((id & 0x8000) == 0)
    {
        if (id > gl_m.num_nodes)
            return NULL;

        gl_node* node = &gl_m.nodes[id];

        vec2 delta = vec2_sub(position, node->partition);
        bool is_on_back = (delta.x * node->delta_partition.y - delta.y * node->delta_partition.x) <= 0.f;

        if (is_on_back)
            id = gl_m.nodes[id].back_child_id;
        else
            id = gl_m.nodes[id].front_child_id;
    }

    if ((id & 0x7fff) >= gl_m.num_subsectors)
        return NULL;

    gl_subsector* subsector = &gl_m.subsectors[id & 0x7fff];
    gl_segment* segment = &gl_m.segments[subsector->first_seg];
    linedef* linedef = &m.linedefs[segment->linedef];

    sidedef* sidedef;
    if (segment->side == 0)
        sidedef = &m.sidedefs[linedef->front_sidedef];
    else
        sidedef = &m.sidedefs[linedef->back_sidedef];

    return &m.sectors[sidedef->sector_index];
}
