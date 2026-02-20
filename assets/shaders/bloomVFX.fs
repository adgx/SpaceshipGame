#version 400

out vec4 FragColor;
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

//textures
uniform sampler2D RenderTex;
uniform sampler2D BlurTex;

//res
uniform int Width;
uniform int Height;
uniform float LumThresh; // Luminance threshold

// Weights and offsets for the Gaussian blur
uniform float PixOffset[10] =
float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
uniform float Weight[10];

//extract the bright information
float luma( vec3 color ) 
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

//subroutines
subroutine vec4 RenderPassType();
subroutine uniform RenderPassType RenderPass;


// Pass to extract the bright parts
subroutine( RenderPassType )
vec4 pass1()
{
    vec4 val = texture(RenderTex, TexCoord);
    return val * clamp(luma(val.rgb) -LumThresh, 0.0, 1.0 ) * (1.0 / (1.0 - LumThresh) );
}

// First blur pass
subroutine( RenderPassType )
vec4 pass2()
{
    float dy = 1.0 / float(Height);
    vec4 sum = texture(BlurTex, TexCoord) * Weight[0];
    
    for(int i = 1; i < 10; i++)
    {
        sum += texture( BlurTex, TexCoord + vec2(0.0, PixOffset[i]) * dy ) * Weight[i];
        sum += texture( BlurTex, TexCoord - vec2(0.0, PixOffset[i]) * dy ) * Weight[i];
    }

    return sum;
}

// Second blur and add to original
subroutine( RenderPassType )
vec4 pass3()
{
    float dx = 1.0 / float(Width);
    vec4 val = texture(RenderTex, TexCoord);
    vec4 sum = texture(BlurTex, TexCoord) * Weight[0];
    
    for(int i = 1; i < 10; i++)
    {
        sum += texture(BlurTex, TexCoord + vec2(PixOffset[i], 0.0) * dx ) * Weight[i];
        sum += texture( BlurTex, TexCoord - vec2(PixOffset[i], 0.0) * dx ) * Weight[i];
    }
    
    return val + sum;
}


void main()
{
    //execute the postprocessing passes
    FragColor = RenderPass();
}