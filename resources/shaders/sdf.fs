#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

void main()
{
    // Use SDF thresholding
    float dist = texture(texture0, fragTexCoord).r;
    float width = 0.5;
    float edge = 0.1;
    float alpha = smoothstep(width - edge, width + edge, dist);
    
    if (alpha <= 0.0) discard;
    
    finalColor = vec4(fragColor.rgb, fragColor.a * alpha) * colDiffuse;
}
