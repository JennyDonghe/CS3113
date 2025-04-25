uniform sampler2D diffuse;
uniform vec2 light_position;
uniform int use_spotlight;

varying vec2 tex_coord_var;
varying vec2 pix_position;

float attenuate(float dist, float lin_att, float quad_att)
{
    return 1.0 / (1.0 + (lin_att * dist) + (quad_att * dist * dist));
}

void main()
{
    vec4 color = texture2D(diffuse, tex_coord_var);

    if (use_spotlight == 1)
    {
        float brightness = attenuate(
            distance(light_position, pix_position),
            0.0,      // linear attenuation
            5.0       // quadratic attenuation
        );
        color.rgb *= brightness;
    }

    gl_FragColor = color;
}

