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

float bayer4x4(vec2 uv) {
    int x = int(mod(uv.x * renderSize.x, 4.0));
    int y = int(mod(uv.y * renderSize.y, 4.0));
    int index = x + y * 4;
    float m[16] = float[](
         0.0,  8.0,  2.0, 10.0,
        12.0,  4.0, 14.0,  6.0,
         3.0, 11.0,  1.0,  9.0,
        15.0,  7.0, 13.0,  5.0
    );
    return m[index] / 16.0;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    float brightness = dot(texelColor.rgb, vec3(0.299, 0.587, 0.114));
    
    float threshold = bayer4x4(fragTexCoord);
    float val = brightness > threshold ? 1.0 : 0.0;
    
    finalColor = vec4(vec3(val), 1.0) * colDiffuse;
}
