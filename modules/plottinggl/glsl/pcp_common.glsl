uniform ivec2 dims;
uniform vec4 spacing = vec4(0);

/**
 * Convert a normalized position  [0 1] into screen coordinates [-1 1] considering canvas dimensions
 * and optional spacing
 */
vec2 getPosWithSpacing(vec2 inPos) {
    vec2 pixelSpace = (inPos * (dims - spacing.wz - spacing.yx) + spacing.wz);
    return pixelSpace * 2.0 / dims - 1.0;
}

vec2 getPixelSpacing() { return 1.0f / dims.xy; }
