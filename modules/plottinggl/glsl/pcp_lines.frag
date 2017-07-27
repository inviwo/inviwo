in vec3 lpickColor;
in vec2 ltexCoord;
in float lfalloffAlpha;

uniform bool additiveBlend;
uniform float alpha;
uniform float lineWidth;

uniform int selected;
uniform vec4 selectedColor = vec4(1, 0, 0, 1);

uniform int filtered;
uniform int subtractiveBelnding;
uniform vec4 filterColor;
uniform float filterIntensity;
uniform float falllofPower = 2;

uniform sampler2D tf;
uniform sampler2D tfSelection;

void main() {
    vec4 res = vec4(1);

    if (selected > 0) {
        res = texture(tfSelection, vec2(ltexCoord.y,0.5f));
        //res.rgb = selectedColor.rgb;
    } else {
        res = texture(tf, vec2(ltexCoord.x,0.5f));
        if(subtractiveBelnding == 1){
            res.rgb = 1-res.rgb;
        }
        if (filtered == 1) 
             res.xyz = mix(res.xyz, filterColor.xyz, filterIntensity);
        if (additiveBlend) {
            res.a *= alpha * pow(lfalloffAlpha, falllofPower);
        }

    }

    PickingData = vec4(lpickColor, 1.0);

    FragData0 = res;
}