#version 430 core
#extension GL_ARB_compute_shader: enable
#extension GL_ARB_shader_storage_buffer_object: enable


#define SQRT_NUM_TILES 8
#define MAX_LIGHTS 256


shared uint lightIDs[MAX_LIGHTS];
shared uint lightCount = 0;
shared uint zMin;
shared uint zMax;

layout(std430, binding = 0) buffer pointLights {
    vec3 Lights[];
}

layout(local_size_x = SQRT_NUM_TILES, local_size_y = SQRT_NUM_TILES, local_size_z = 1) in;

void main() {
    // gl_GlobalInvocationID is equal to:
    //     gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID.
    uint idx = gl_GlobalInvocationID.x;
    uint idy = gl_GlobalInvocationID.y;

    //http://gamedev.stackexchange.com/questions/67431/deferred-tiled-shading-tile-frusta-calculation-in-opengl
    //calculate asymmetric frustum around each tile
    uint corner1 = TILE_RES * idx;
    uint corner2 = TILE_RES * idy;
    uint corner3 = TILE_RES * (idx+1.0);
    uint corner4 = TILE_RES * (idy+1.0);
    uint width = TILE_RES * NUM_TILES_X * 2.0 - 1.0;
    uint height = TILE_RES * NUM_TILES_Y * 2.0 - 1.0;

    vec3 frustum[4];
    //https://books.google.com/books?id=30ZOCgAAQBAJ&pg=PA437&lpg=PA437&dq=calculate+tile+frustum&source=bl&ots=2YgxdVJHGG&sig=z2zJgPDY9YhTnm7H1j06Ziuo7JI&hl=en&sa=X&ved=0ahUKEwiDqcqaq6fJAhXIPT4KHQdJCkAQ6AEITTAH#v=onepage&q=calculate%20tile%20frustum&f=false
    // need to convert from Projection to View 
    //create plane equation for each side
    //find intersection using signed Distance function

    //use z buffer pre-pass to find min and max depth per tile - use atomic min and max


    //check for intersections with radius of each light - arvo intersection test?
}
