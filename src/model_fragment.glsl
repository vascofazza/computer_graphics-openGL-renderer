#version 120

varying vec3 pos;                   // [from vertex shader] position in world space
varying vec3 norm;                  // [from vertex shader] normal in world space (need normalization)
varying vec2 texcoord;              // [from vertex shader] texture coordinate

uniform vec3 camera_pos;            // camera position (center of the camera frame)

uniform vec3 ambient;               // scene ambient

uniform int lights_num;             // number of lights
uniform vec3 light_pos[16];         // light positions
uniform vec3 light_intensity[16];   // light intensities

uniform vec3 material_kd;           // material kd
uniform vec3 material_ks;           // material ks
uniform float material_n;           // material n
uniform int material_is_lines;     // whether the material is lines or meshes

uniform bool material_kd_txt_on;    // material kd texture enabled
uniform sampler2D material_kd_txt;  // material kd texture
uniform bool material_ks_txt_on;    // material ks texture enabled
uniform sampler2D material_ks_txt;  // material ks texture
uniform bool material_norm_txt_on;    // material norm texture enabled
uniform sampler2D material_norm_txt;  // material norm texture

// main
void main() {
    // re-normalize normals
    vec3 n = normalize(norm);
    vec3 c = vec3(0,0,0);   // initialize to red to see it well
    vec3 kd, ks;
    // YOUR CODE GOES HERE ---------------------
    vec3 camdir = normalize(camera_pos-pos);
    n = faceforward(n, camdir, -n);
    // compute material values by looking up textures is necessary
    if(material_kd_txt_on)
    {
        vec3 tex = texture2D(material_kd_txt, texcoord).rgb;
        kd = tex*material_kd;
    }
    else
        kd = material_kd;
    if(material_ks_txt_on)
    {
        vec3 tex = texture2D(material_ks_txt, texcoord).rgb;
        ks = tex*material_ks;
    }
    else
        ks = material_ks;
    // lookup normal map if needed
    if(material_norm_txt_on)
    {
        vec3 tex = texture2D(material_norm_txt, texcoord).xyz;
        n = normalize(tex*2-1);
    }
    // accumulate ambient
    c += ambient*kd;
    // foreach light
    int i;
    for(i = 0; i < lights_num; i++)
    {
        // compute point light color at pos
        vec3 lightIntensity = light_intensity[i] / pow(distance(light_pos[i], pos),2);
        
        // compute light direction at pos
        vec3 ldir = normalize(light_pos[i]-pos);
        // compute view direction using camera_pos and pos

        // compute h
        vec3 bisector = normalize(ldir+camdir);
        // accumulate blinn-phong model
        c += (lightIntensity * (kd + ks * max(0, pow((1-material_is_lines)*dot(n, bisector)+material_is_lines*sqrt(1- dot(n, bisector)*dot(n, bisector)),material_n))) * max(0, (1-material_is_lines)*dot(n,ldir) + material_is_lines*sqrt(1- dot(n, bisector)*dot(n, bisector)))); //tangente nella spline
    }
    // output final color by setting gl_FragColor
    gl_FragColor = vec4(c,1);
}
