in vec4 lPickColor;
in float lScalarMeta;
in float lFalloffAlpha;

uniform bool additiveBlend = true;
uniform bool subtractiveBelnding = false;

uniform vec4 color;
uniform float mixColor;
uniform float mixAlpha;

uniform float fallofPower = 2.0;

uniform sampler2D tf;

void main() {
    vec4 res = texture(tf, vec2(lScalarMeta, 0.5f));
    
    if (subtractiveBelnding) {
        res.rgb = 1 - res.rgb;
    }
    
    res.rgb = mix(res.rgb, color.rgb, mixColor);
    res.a = mix(res.a, color.a, mixAlpha);

    if (additiveBlend) {
        res.a *= pow(lFalloffAlpha, fallofPower);
    }

    PickingData = lPickColor;
    FragData0 = res;
}
