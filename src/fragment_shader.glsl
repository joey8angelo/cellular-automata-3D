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

uniform vec3 c0;
uniform vec3 c1;
uniform vec3 c2;
uniform vec3 c3;
uniform vec3 c4;
uniform vec3 c5;
uniform bool gradInverted;

const int MAX_STEPS = 512;

vec3 grad(float t) {
    if (gradInverted) {
        t = 1.0 - t;
    }
    if (t <= 0.0) return c0;
    else if (t >= 1.0) return c5;
    else if (t < 0.2) return mix(c0, c1, t / 0.2);
    else if (t < 0.4) return mix(c1, c2, (t - 0.2) / 0.2);
    else if (t < 0.6) return mix(c2, c3, (t - 0.4) / 0.2);
    else if (t < 0.8) return mix(c3, c4, (t - 0.6) / 0.2);
    else return mix(c4, c5, (t - 0.8) / 0.2);
}

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

vec3 traverse(vec3 o, vec3 d) {
    vec3 gridMin = vec3(0.0);
    vec3 gridMax = voxelSize;
    float entryT = ray_aabb(gridMin, gridMax, o, d);
    if (entryT == 1e30f) {
        return vec3(0.0, 0.0, 0.0);
    }

    vec3 entryPos = o + d * (entryT + 0.0001);
    
    vec3 step = sign(d);
    vec3 delta = abs(1.0 / d);

    vec3 pos = floor(entryPos);
    
    vec3 tmax = (pos + max(step, vec3(0.0)) - entryPos) / d;
    
    int axis = 0;
    for (int steps = 0; steps < MAX_STEPS; steps++) {
        // check bounds before reading texture
        if (pos.x >= 0.0 && pos.x < voxelSize.x &&
            pos.y >= 0.0 && pos.y < voxelSize.y &&
            pos.z >= 0.0 && pos.z < voxelSize.z) {
            
            uint voxelValue = texelFetch(voxelData, ivec3(pos), 0).r;

            if(voxelValue != 0u) {
                float t = float(steps) / voxelSize[axis];
                return grad(clamp(t, 0.1, 1.0));
            }
        }
        
        // get next voxel
        axis = 0;
        if (tmax.y < tmax.x) axis = 1;
        if (tmax.z < tmax[axis]) axis = 2;
        
        pos[axis] += step[axis];
        tmax[axis] += delta[axis];
        
        // exit if outside the grid
        if (pos[axis] < 0.0 || pos[axis] >= voxelSize[axis]) {
            return vec3(0.0, 0.0, 0.0);
        }
    }

    return vec3(0.0, 0.0, 0.0);
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