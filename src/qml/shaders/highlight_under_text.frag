#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 tintColor;
    vec2 boxSize;
    float boxRadius;
    float tintStrength;
};

layout(binding = 1) uniform sampler2D src;

float sdRoundBox(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + r;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main() {
    vec4 page = texture(src, qt_TexCoord0);
    vec3 multiplied = page.rgb * tintColor.rgb;
    vec3 blended = mix(page.rgb, multiplied, tintStrength);

    vec2 uv = qt_TexCoord0 * boxSize;
    vec2 p = uv - boxSize * 0.5;
    float d = sdRoundBox(p, boxSize * 0.5, boxRadius);
    float alpha = 1.0 - smoothstep(-0.75, 0.75, d);

    fragColor = vec4(blended, alpha) * qt_Opacity;
}
