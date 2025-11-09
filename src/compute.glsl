#version 460 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout (binding = 0, r8ui) uniform uimage3D writeTex;
layout (binding = 1, r8ui) uniform uimage3D readTex;
uniform bool wrapEdges;

uint countNeighbors(ivec3 pos) {
    uint count = 0;
    ivec3 size = imageSize(readTex);
    for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
            for (int x = -1; x <= 1; x++) {
                if (x == 0 && y == 0 && z == 0) continue;
                ivec3 neighborPos = pos + ivec3(x, y, z);

                if (wrapEdges) {
                    neighborPos.x = (neighborPos.x + size.x) % size.x;
                    neighborPos.y = (neighborPos.y + size.y) % size.y;
                    neighborPos.z = (neighborPos.z + size.z) % size.z;
                } else {
                    if (neighborPos.x < 0 || 
                        neighborPos.y < 0 || 
                        neighborPos.z < 0 ||
                        neighborPos.x >= size.x || 
                        neighborPos.y >= size.y || 
                        neighborPos.z >= size.z) {
                        continue;
                    }
                }

                count += imageLoad(readTex, neighborPos).r;
            }
        }
    }
    return count;
}

void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);
    
    if (pos.x >= imageSize(readTex).x ||
        pos.y >= imageSize(readTex).y ||
        pos.z >= imageSize(readTex).z) {
        return;
    }
    
    uint state = imageLoad(readTex, pos).r;
    
    uint neighbors = countNeighbors(pos);
    if (state == 0u) {
        if (neighbors >= 14u && neighbors <= 19u) {
            state = 1u;
        } else {
            state = 0u;
        }
    } else {
        if (neighbors < 13u) {
            state = 0u;
        } else {
            state = 1u;
        }
    }

    imageStore(writeTex, pos, uvec4(state, 0, 0, 0));
}
