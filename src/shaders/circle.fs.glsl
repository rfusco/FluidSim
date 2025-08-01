#version 330 core
in vec2 fragLocalPos;
in float fragPressure;

out vec4 FragColor;

uniform float u_minPressure;
uniform float u_maxPressure;
uniform int u_colorMode; // 0=Jet,1=Heat,2=BlueRed

vec3 jetColor(float t) {
    t = clamp(t, 0.0, 1.0);
    float b = clamp(1.5 - abs(4.0*t - 3.0), 0.0, 1.0);
    float g = clamp(1.5 - abs(4.0*t - 2.0), 0.0, 1.0);
    float r = clamp(1.5 - abs(4.0*t - 1.0), 0.0, 1.0);
    return vec3(r,g,b);
}

vec3 heatColor(float t) {
    t = clamp(t, 0.0, 1.0);
    return vec3(t, t*t, 0.0);
}

vec3 blueRedColor(float t) {
    t = clamp(t, 0.0, 1.0);
    return mix(vec3(0,0,1), vec3(1,0,0), (1-t) * 2.5);
}

void main() {
    float dist = length(fragLocalPos);
    if(dist > 1.0) discard;
    float alpha = smoothstep(1.0, 0.95, dist);

    float normPressure = (fragPressure - u_minPressure) / (u_maxPressure - u_minPressure + 1e-6);
    
    vec3 color;
    if(u_colorMode == 0) color = jetColor(normPressure);
    else if(u_colorMode == 1) color = heatColor(normPressure);
    else color = blueRedColor(normPressure);

    FragColor = vec4(color, alpha);
}
