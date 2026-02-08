#version 450

layout(set = 0, binding = 0) readonly buffer WaterfallBuffer {
    float samples[];
} waterfall;

layout(push_constant) uniform PushConstants {
    float binCount;
    float historyLength;
} pushConstants;

layout(location = 0) out vec3 fragColor;

void main() {
    int index = gl_VertexIndex;
    int bins = int(pushConstants.binCount);
    int rows = int(pushConstants.historyLength);
    if (bins <= 0 || rows <= 0) {
        gl_Position = vec4(0.0);
        fragColor = vec3(0.0);
        return;
    }
    int col = index % bins;
    int row = index / bins;
    float x = (float(col) / float(max(bins - 1, 1))) * 2.0 - 1.0;
    float z = (float(row) / float(max(rows - 1, 1))) * 2.0 - 1.0;
    float height = waterfall.samples[index];
    gl_Position = vec4(x, height, z, 1.0);
    gl_PointSize = 2.0;
    fragColor = vec3(height, 0.4 + height * 0.6, 1.0 - height);
}
