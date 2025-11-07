#version 460

struct Ray {
    vec3 origin;
    vec3 direction;
};

uniform vec2 resolution;
uniform mat4 view;
uniform mat4 projection;
uniform usampler3D voxelData;
uniform vec3 voxelSize;

const int MAX_STEPS = 512;

float ray_aabb(vec3 bmin, vec3 bmax, vec3 o, vec3 d) {
    float tmin = 0;
    float tmax = 1e30f;

    for (int i = 0; i < 3; i++) {
        float t1 = (bmin[i] - o[i]) / d[i];
        float t2 = (bmax[i] - o[i]) / d[i];

        float dmin = min(t1, t2);
        float dmax = max(t1, t2);

        tmin = max(tmin, dmin);
        tmax = min(tmax, dmax);
    }

    if (tmax >= tmin) {
        return tmin;
    }
    return 1e30f;
}

//float findNearest(vec3 o, vec3 d) {
    //vec3 gridMin = vec3(0.0);
    //vec3 gridMax = voxelSize;
    //float entryT = ray_aabb(gridMin, gridMax, o, d);
    //if (entryT == 1e30f) {
        //return 1e30f;
    //}
    //
    //uint voxelValue = texture(voxelData, vec3(0.0)).r;
    //if (voxelValue != 0u) {
        //return entryT;
    //}

    //return 1e30f;
//}

vec3 traverse(vec3 o, vec3 d) {
    vec3 gridMin = vec3(0.0);
    vec3 gridMax = voxelSize;
    float entryT = ray_aabb(gridMin, gridMax, o, d);
    if (entryT == 1e30f) {
        return vec3(0.0);
    }

    vec3 entryPos = o + d * (entryT + 0.0001f);
    
    vec3 step = sign(d);
    vec3 delta = abs(1.0f / d);

    // clamp entry point inside the grid
    vec3 pos = clamp(floor(entryPos), gridMin, gridMax - vec3(1.0f));
    
    vec3 tmax = (pos - entryPos + max(step, vec3(0.0))) / d;
    
    int axis = 0;
    for (int steps = 0; steps < MAX_STEPS; steps++) {
        uint voxelValue = texelFetch(voxelData, ivec3(pos), 0).r;

        if(voxelValue != 0u) {
            float t = entryT + (tmax[axis] - delta[axis]);
            float depth = t - entryT;
            return vec3(10 / (depth)) + vec3(0.05, 0.0, 0.1);
            //return vec3(exp(-depth * 0.1)) + vec3(0.05, 0.0, 0.1);
        }
        
        if (tmax.x < tmax.y) {
            if (tmax.x < tmax.z) {
                pos.x += step.x;
                if (pos.x < 0 || pos.x >= voxelSize.x) {
                    break;
                }
                axis = 0;
                tmax.x += delta.x;
            } else {
                pos.z += step.z;
                if (pos.z < 0 || pos.z >= voxelSize.z) {
                    break;
                }
                axis = 2;
                tmax.z += delta.z;
            }
        } else {
            if (tmax.y < tmax.z) {
                pos.y += step.y;
                if (pos.y < 0 || pos.y >= voxelSize.y) {
                    break;
                }
                axis = 1;
                tmax.y += delta.y;
            } else {
                pos.z += step.z;
                if (pos.z < 0 || pos.z >= voxelSize.z) {
                    break;
                }
                axis = 2;
                tmax.z += delta.z;
            }
        }
    }

    return vec3(0.0);
}

Ray getRay() {
vec2 px = gl_FragCoord.xy / resolution;
    mat4 invProj = inverse(projection);
    mat4 invView = inverse(view);

    vec2 pxNDS = px * 2.0 - 1.0;
    
    vec4 nearPoint = vec4(pxNDS, -1.0, 1.0);
    vec4 farPoint = vec4(pxNDS, 1.0, 1.0);
    
    vec4 nearEye = invProj * nearPoint;
    nearEye /= nearEye.w;
    
    vec4 farEye = invProj * farPoint;
    farEye /= farEye.w;
    
    vec3 nearWorld = (invView * nearEye).xyz;
    vec3 farWorld = (invView * farEye).xyz;
    
    vec3 dirWorld = normalize(farWorld - nearWorld);
    
    return Ray(nearWorld, dirWorld);
}

out vec4 FragColor;

void main() {
    
    Ray ray = getRay();

    FragColor = vec4(traverse(ray.origin, ray.direction), 1.0);
}