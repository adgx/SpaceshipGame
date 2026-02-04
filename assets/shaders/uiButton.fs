#version 400 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D ui_tex;
uniform sampler2D ui_hover_tex;
uniform vec4 color_val;

subroutine vec4 uiTexture();
subroutine uniform uiTexture uiTextureMode;

subroutine( uiTexture )
vec4 uiTextureBase()
{
    return texture(ui_tex, TexCoords)*color_val;
}

subroutine( uiTexture )
vec4 uiTextureHover()
{
    return texture(ui_hover_tex, TexCoords)*color_val*vec4(1, 0.36, 0, 1);
}

void main()
{
    FragColor = uiTextureMode();
}