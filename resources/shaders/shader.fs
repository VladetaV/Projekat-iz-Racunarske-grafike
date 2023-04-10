#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex;

// sejder za Sunce ne treba da radi nista sem da vraca boju onakva kakva je u teksturi jer je Sunce sam izvor svetlosti
// i njegova boja ne zavisi ni od kakvog izvora svetlosti

void main()
{

    vec3 color = vec3(texture(tex, TexCoords)).rgb;
    FragColor = vec4(color, 1.0);
}