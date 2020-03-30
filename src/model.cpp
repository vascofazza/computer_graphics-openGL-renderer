#include "glcommon.h"

#include "scene.h"
#include "image.h"
#include "tesselation.h"

#include <cstdio>

// OpenGL state for shading
struct ShadeState {
    int gl_program_id = 0;          // OpenGL program handle
    int gl_vertex_shader_id = 0;    // OpenGL vertex shader handle
    int gl_fragment_shader_id = 0;  // OpenGL fragment shader handle
    map<image3f*,int> gl_texture_id;// OpenGL texture handles
};

// initialize the shaders
void init_shaders(ShadeState* state) {
    // load shader code from files
    auto vertex_shader_code = load_text_file("model_vertex.glsl");
    auto fragment_shader_code = load_text_file("model_fragment.glsl");
    auto vertex_shader_codes = (char *)vertex_shader_code.c_str();
    auto fragment_shader_codes = (char *)fragment_shader_code.c_str();

    // create shaders
    state->gl_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    state->gl_fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    
    // load shaders code onto the GPU
    glShaderSource(state->gl_vertex_shader_id,1,(const char**)&vertex_shader_codes,nullptr);
    glShaderSource(state->gl_fragment_shader_id,1,(const char**)&fragment_shader_codes,nullptr);
    
    // compile shaders
    glCompileShader(state->gl_vertex_shader_id);
    glCompileShader(state->gl_fragment_shader_id);
    
    // check if shaders are valid
    error_if_glerror();
    error_if_shader_not_valid(state->gl_vertex_shader_id);
    error_if_shader_not_valid(state->gl_fragment_shader_id);
    
    // create program
    state->gl_program_id = glCreateProgram();
    
    // attach shaders
    glAttachShader(state->gl_program_id,state->gl_vertex_shader_id);
    glAttachShader(state->gl_program_id,state->gl_fragment_shader_id);
    
    // bind vertex attributes locations
    glBindAttribLocation(state->gl_program_id, 0, "vertex_pos");
    glBindAttribLocation(state->gl_program_id, 1, "vertex_norm");
    glBindAttribLocation(state->gl_program_id, 2, "vertex_texcoord");

    // link program
    glLinkProgram(state->gl_program_id);
    
    // check if program is valid
    error_if_glerror();
    error_if_program_not_valid(state->gl_program_id);
}

// initialize the textures
void init_textures(Scene* scene, ShadeState* state) {
    // grab textures from scene
    auto textures = get_textures(scene);
    // foreach texture
    for(auto texture : textures) {
        // if already in the state->gl_texture_id map, skip
        if(state->gl_texture_id.find(texture) != state->gl_texture_id.end()) continue;
        // gen texture id
        unsigned int id = 0;
        glGenTextures(1, &id);
        // set id to the state->gl_texture_id map for later use
        state->gl_texture_id[texture] = id;
        // bind texture
        glBindTexture(GL_TEXTURE_2D, id);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        // load texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     texture->width(), texture->height(),
                     0, GL_RGB, GL_FLOAT, texture->data());
    }
}

// utility to bind texture parameters for shaders
// uses texture name, texture_on name, texture pointer and texture unit position
void _bind_texture(string name_map, string name_on, image3f* txt, int pos, ShadeState* state) {
    // if txt is not null
    if(txt) {
        // set texture on boolean parameter to true
        glUniform1i(glGetUniformLocation(state->gl_program_id,name_on.c_str()),GL_TRUE);
        // activate a texture unit at position pos
        glActiveTexture(GL_TEXTURE0+pos);
        // bind texture object to it from state->gl_texture_id map
        glBindTexture(GL_TEXTURE_2D, state->gl_texture_id[txt]);
        // set texture parameter to the position pos
        glUniform1i(glGetUniformLocation(state->gl_program_id, name_map.c_str()), pos);
    } else {
        // set texture on boolean parameter to false
        glUniform1i(glGetUniformLocation(state->gl_program_id,name_on.c_str()),GL_FALSE);
        // activate a texture unit at position pos
        glActiveTexture(GL_TEXTURE0+pos);
        // set zero as the texture id
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// render the scene with OpenGL
void shade(Scene* scene, ShadeState* state) {
    // enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // disable culling face
    glDisable(GL_CULL_FACE);
    // let the shader control the points
    glEnable(GL_POINT_SPRITE);
    
    // set up the viewport from the scene image size
    glViewport(0, 0, scene->image_width, scene->image_height);
    
    // clear the screen (both color and depth) - set cleared color to background
    glClearColor(scene->background.x, scene->background.y, scene->background.z, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // enable program
    glUseProgram(state->gl_program_id);
    
    // bind camera's position, inverse of frame and projection
    // use frame_to_matrix_inverse and frustum_matrix
    glUniform3fv(glGetUniformLocation(state->gl_program_id,"camera_pos"),
                 1, &scene->camera->frame.o.x);
    glUniformMatrix4fv(glGetUniformLocation(state->gl_program_id,"camera_frame_inverse"),
                       1, true, &frame_to_matrix_inverse(scene->camera->frame)[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(state->gl_program_id,"camera_projection"),
                       1, true, &frustum_matrix(-scene->camera->dist*scene->camera->width/2, scene->camera->dist*scene->camera->width/2,
                                                -scene->camera->dist*scene->camera->height/2, scene->camera->dist*scene->camera->height/2,
                                                scene->camera->dist,10000)[0][0]);
    
    // bind ambient and number of lights
    glUniform3fv(glGetUniformLocation(state->gl_program_id,"ambient"),1,&scene->ambient.x);
    glUniform1i(glGetUniformLocation(state->gl_program_id,"lights_num"),scene->lights.size());
    
    // foreach light
    auto count = 0;
    for(auto light : scene->lights) {
        // bind light position and internsity (create param name with tostring)
        glUniform3fv(glGetUniformLocation(state->gl_program_id,tostring("light_pos[%d]",count).c_str()),
                     1, &light->frame.o.x);
        glUniform3fv(glGetUniformLocation(state->gl_program_id,tostring("light_intensity[%d]",count).c_str()),
                     1, &light->intensity.x);
        count++;
    }
    
    // foreach mesh
    for(auto mesh : scene->meshes) {
        // bind material kd, ks, n
        glUniform3fv(glGetUniformLocation(state->gl_program_id,"material_kd"),
                     1,&mesh->mat->kd.x);
        glUniform3fv(glGetUniformLocation(state->gl_program_id,"material_ks"),
                     1,&mesh->mat->ks.x);
        glUniform1f(glGetUniformLocation(state->gl_program_id,"material_n"),
                    mesh->mat->n);
        glUniform1i(glGetUniformLocation(state->gl_program_id,"material_is_lines"),
                    mesh->line.size() > 0? 1:0);
        // bind texture params (txt_on, sampler)
        _bind_texture("material_kd_txt", "material_kd_txt_on", mesh->mat->kd_txt, 0, state);
        _bind_texture("material_ks_txt", "material_ks_txt_on", mesh->mat->ks_txt, 1, state);
        _bind_texture("material_norm_txt", "material_norm_txt_on", mesh->mat->norm_txt, 2, state);
        
        // bind mesh frame - use frame_to_matrix
        glUniformMatrix4fv(glGetUniformLocation(state->gl_program_id,"mesh_frame"),
                           1,true,&frame_to_matrix(mesh->frame)[0][0]);
    
        // enable vertex attributes arrays and set up pointers to the mesh data
        auto vertex_pos_location = glGetAttribLocation(state->gl_program_id, "vertex_pos");
        auto vertex_norm_location = glGetAttribLocation(state->gl_program_id, "vertex_norm");
        glEnableVertexAttribArray(vertex_pos_location);
        glVertexAttribPointer(vertex_pos_location, 3, GL_FLOAT, GL_FALSE, 0, &mesh->pos[0].x);
        auto vertex_texcoord_location = glGetAttribLocation(state->gl_program_id, "vertex_texcoord");
        if(not mesh->norm.empty()) {
            glEnableVertexAttribArray(vertex_norm_location);
            glVertexAttribPointer(vertex_norm_location, 3, GL_FLOAT, GL_FALSE, 0, &mesh->norm[0].x);
        } else glVertexAttrib3f(vertex_norm_location, 0, 0, 1);
        if(not mesh->texcoord.empty()) {
            glEnableVertexAttribArray(vertex_texcoord_location);
            glVertexAttribPointer(vertex_texcoord_location, 2, GL_FLOAT, GL_FALSE, 0, &mesh->texcoord[0].x);
        }
        else glVertexAttrib2f(vertex_texcoord_location, 0, 0);
        
        // draw triangles and quads
        if(not scene->draw_wireframe) {
            if(mesh->triangle.size()) glDrawElements(GL_TRIANGLES, mesh->triangle.size()*3, GL_UNSIGNED_INT, &mesh->triangle[0].x);
            if(mesh->quad.size()) glDrawElements(GL_QUADS, mesh->quad.size()*4, GL_UNSIGNED_INT, &mesh->quad[0].x);
        } else {
            auto edges = EdgeMap(mesh->triangle, mesh->quad).edges();
            glDrawElements(GL_LINES, edges.size()*2, GL_UNSIGNED_INT, &edges[0].x);
        }
        
        // draw line sets
        if(not mesh->line.empty()) glDrawElements(GL_LINES, mesh->line.size()*2, GL_UNSIGNED_INT, mesh->line.data());
        for(auto segment : mesh->spline) glDrawElements(GL_LINE_STRIP, 4, GL_UNSIGNED_INT, &segment);
        
        // disable vertex attribute arrays
        glDisableVertexAttribArray(vertex_pos_location);
        if(not mesh->norm.empty()) glDisableVertexAttribArray(vertex_norm_location);
        if(not mesh->texcoord.empty()) glDisableVertexAttribArray(vertex_texcoord_location);
    }
}

string scene_filename;          // scene filename
string image_filename;          // image filename
Scene* scene;                   // scene arrays

// uiloop
void uiloop() {
    auto ok = glfwInit();
    error_if_not(ok, "glfw init error");
    
    // setting an error callback
    glfwSetErrorCallback([](int ecode, const char* msg){ return error(msg); });
    
    // glfwWindowHint(GLFW_SAMPLES, scene->image_samples*scene->image_samples);

    auto window = glfwCreateWindow(scene->image_width,
                                   scene->image_height,
                                   "graphics16 | model", NULL, NULL);
    error_if_not(window, "glfw window error");
    
    glfwMakeContextCurrent(window);
    
    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int key) {
        switch (key) {
            case 's':
                scene->draw_captureimage = true;
                break;
            case 'w':
                scene->draw_wireframe = not scene->draw_wireframe;
                break;
        }
    });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
#ifdef _WIN32
	auto ok1 = glewInit();
	error_if_not(GLEW_OK == ok1, "glew init error");
#endif

    auto state = new ShadeState();
    init_shaders(state);
    init_textures(scene,state);
    
    auto mouse_last_x = -1.0;
    auto mouse_last_y = -1.0;
    
    while(not glfwWindowShouldClose(window)) {
        glfwGetFramebufferSize(window, &scene->image_width, &scene->image_height);
        scene->camera->width = (scene->camera->height * scene->image_width) / scene->image_height;
        
        shade(scene,state);

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            if (mouse_last_x < 0 or mouse_last_y < 0) { mouse_last_x = x; mouse_last_y = y; }
            auto delta_x = x - mouse_last_x, delta_y = y - mouse_last_y;
            
            set_view_turntable(scene->camera, delta_x*0.01, -delta_y*0.01, 0, 0, 0);
            
            mouse_last_x = x;
            mouse_last_y = y;
        } else { mouse_last_x = -1; mouse_last_y = -1; }
        
        if(scene->draw_captureimage) {
            auto image = image3f(scene->image_width,scene->image_height);
            glReadPixels(0, 0, scene->image_width, scene->image_height, GL_RGB, GL_FLOAT, &image.at(0,0));
            write_png(image_filename, image, true);
            scene->draw_captureimage = false;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    
    delete state;
}

// main function
int main(int argc, char** argv) {
    auto args = parse_cmdline(argc, argv,
        { "02_model", "raytrace a scene",
            {  {"resolution", "r", "image resolution", "int", true, jsonvalue() }  },
            {  {"scene_filename", "", "scene filename", "string", false, jsonvalue("scene.json")},
               {"image_filename", "", "image filename", "string", true, jsonvalue("")}  }
        });
    scene_filename = args.object_element("scene_filename").as_string();
    image_filename = (args.object_element("image_filename").as_string() != "") ?
        args.object_element("image_filename").as_string() :
        scene_filename.substr(0,scene_filename.size()-5)+".png";
    scene = load_json_scene(scene_filename);
    if(not args.object_element("resolution").is_null()) {
        scene->image_height = args.object_element("resolution").as_int();
        scene->image_width = scene->camera->width * scene->image_height / scene->camera->height;
    }
    subdivide(scene);
    uiloop();
}

