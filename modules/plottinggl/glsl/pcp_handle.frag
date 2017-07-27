uniform vec4 color;
uniform vec3 pickColor;

uniform sampler2D texColor;

in vec2 p;

void main(){
    vec4 outColor = texture(texColor,p).rgba;
    outColor.rgb *= color.rgb;
    
    FragData0 = outColor;
    PickingData = vec4(pickColor,1);
}