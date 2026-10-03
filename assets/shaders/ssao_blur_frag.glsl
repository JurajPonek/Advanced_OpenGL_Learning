#version 460 core
out float FragColor;
  
in vec2 o_texture_coords;
  
uniform sampler2D tex0;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(tex0, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(tex0, o_texture_coords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
} 