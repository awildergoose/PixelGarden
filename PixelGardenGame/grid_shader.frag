uniform sampler2D gridTexture;
uniform vec2 resolution;

void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    uv.y = 1.0 - uv.y; // Flip y-coordinate bcz shaders or something idk
    
    vec4 color = texture2D(gridTexture, uv);
    
    vec3 baseColor;
    float tintAmount = 0.5;
    bool applyTint = true;

    if (color.r == 1.0) { // sand
        baseColor = vec3(1.0, 1.0, 0.0);
        tintAmount = 0.45;
    } else if (color.g == 1.0) { // mud
        baseColor = vec3(0.5, 0.25, 0.0);
        tintAmount = 0.3;
    } else if (color.b == 1.0) { // water
        baseColor = vec3(0.0, 0.0, 1.0);
        tintAmount = 0.4;
    } else {
        baseColor = vec3(0.0);
        applyTint = false;
    }

    if(applyTint) {
        baseColor += tintAmount*uv.y;
    }

    gl_FragColor = vec4(baseColor, 1.0);
}
