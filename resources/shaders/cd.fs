#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex;

// Sejder za kosmicku prasinu ne treba da radi nista drugo sem da vraca njenu boju, kao i da
// implementira BLENDING

void main()
{

    vec3 color = vec3(0.5,0.5,0.5).rgb;
    FragColor = vec4(color, 0.5);
}