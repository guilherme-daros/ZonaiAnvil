#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 renderSize;

// Output fragment color
out vec4 finalColor;

float bayer2x2(vec2 uv) {
    int x = int(mod(uv.x * renderSize.x, 2.0));
    int y = int(mod(uv.y * renderSize.y, 2.0));
    int index = x + y * 2;
    float m[4] = float[](0.0, 2.0, 3.0, 1.0);
    return m[index] / 4.0;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    float brightness = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    
    float threshold = bayer2x2(fragTexCoord);
    float val = brightness > threshold ? 1.0 : 0.0;
    
    finalColor = vec4(vec3(val), 1.0) * colDiffuse;
}
