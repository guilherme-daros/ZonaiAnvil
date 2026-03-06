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

void main()
{
    vec2 uv = fragTexCoord;
    vec4 texelColor = texture(texture0, uv);

    // 1. Scanline effect (horizontal dark lines)
    float scanline = sin(uv.y * renderSize.y * 2.0) * 0.12;
    texelColor.rgb -= scanline;

    // 2. Fake "Lots" (Shadow mask / Dot pattern)
    // We use a high frequency sine wave on both axes to create a grid of dots
    float dots = sin(uv.x * renderSize.x * 1.5) * sin(uv.y * renderSize.y * 1.5);
    texelColor.rgb *= (1.0 + dots * 0.08);

    // 3. Vignette (darker corners)
    float dist = distance(uv, vec2(0.5, 0.5));
    texelColor.rgb *= (1.0 - dist * dist * 0.25);

    finalColor = texelColor * colDiffuse;
}
