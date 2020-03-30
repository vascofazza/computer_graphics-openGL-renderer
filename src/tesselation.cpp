#include "tesselation.h"
#include <random>

// make normals for each face - duplicates all vertex data
void facet_normals(Mesh* mesh) {
    // allocates new arrays
    auto pos = vector<vec3f>();
    auto norm = vector<vec3f>();
    auto texcoord = vector<vec2f>();
    auto triangle = vector<vec3i>();
    auto quad = vector<vec4i>();
    // froeach triangle
    for(auto f : mesh->triangle) {
        // grab current pos size
        auto nv = (int)pos.size();
        // compute face face normal
        auto fn = normalize(cross(mesh->pos[f.y]-mesh->pos[f.x], mesh->pos[f.z]-mesh->pos[f.x]));
        // add triangle
        triangle.push_back({nv,nv+1,nv+2});
        // add vertex data
        for(auto i : range(3)) {
            pos.push_back(mesh->pos[f[i]]);
            norm.push_back(fn);
            if(not mesh->texcoord.empty()) texcoord.push_back(mesh->texcoord[f[i]]);
        }
    }
    // froeach quad
    for(auto f : mesh->quad) {
        // grab current pos size
        auto nv = (int)pos.size();
        // compute face normal
        auto fn = normalize(normalize(cross(mesh->pos[f.y]-mesh->pos[f.x], mesh->pos[f.z]-mesh->pos[f.x])) +
                            normalize(cross(mesh->pos[f.z]-mesh->pos[f.x], mesh->pos[f.w]-mesh->pos[f.x])));
        // add quad
        quad.push_back({nv,nv+1,nv+2,nv+3});
        // add vertex data
        for(auto i : range(4)) {
            pos.push_back(mesh->pos[f[i]]);
            norm.push_back(fn);
            if(not mesh->texcoord.empty()) texcoord.push_back(mesh->texcoord[f[i]]);
        }
    }
    // set back mesh data
    mesh->pos = pos;
    mesh->norm = norm;
    mesh->texcoord = texcoord;
    mesh->triangle = triangle;
    mesh->quad = quad;
}

// smooth out normal - does not duplicate data
void smooth_normals(Mesh* mesh) {
    // PLACEHOLDER CODE - REMOVE AFTER FUNCTION IS IMPLEMENTED
    //mesh->norm.resize(mesh->pos.size());
    // YOUR CODE GOES HERE ---------------------
    // set normals array to the same length as pos and init all elements to zero
    mesh->norm = vector<vec3f>(mesh->pos.size(), zero3f);
    // foreach triangle
    for(auto triangle : mesh->triangle)
    {
        // compute face normal
        vec3f normal = normalize(cross(mesh->pos[triangle[1]]-mesh->pos[triangle[0]], mesh->pos[triangle[2]]-mesh->pos[triangle[0]]));
        // accumulate face normal to the vertex normals of each face index
        for(int i : range(3))
            mesh->norm[triangle[i]] += normal;
    }
    // foreach quad
    for(auto quad : mesh->quad)
    {
        // compute face normal
        //primo triangolo 0,1,2 - secondo triangolo 2,3,0
        vec3f normal = (normalize(cross(mesh->pos[quad[1]]-mesh->pos[quad[0]], mesh->pos[quad[2]]-mesh->pos[quad[0]]))+normalize(cross(mesh->pos[quad[3]]-mesh->pos[quad[2]], mesh->pos[quad[0]]-mesh->pos[quad[2]])))/2.f;
        // accumulate face normal to the vertex normals of each face index
        for(int i : range(4))
            mesh->norm[quad[i]] += normal;
    }
    // normalize all vertex normals
    for(int i : range(mesh->norm.size()))
        mesh->norm[i] = normalize(mesh->norm[i]);
}

// smooth out tangents
void smooth_tangents(Mesh* polyline) {
    // set tangent array
    //polyline->norm = vector<vec3f>(polyline->pos.size(),zero3f);
    //DEBUG!!! venivano cancellate tutte le normali
    polyline->norm.resize(polyline->pos.size());
    // foreach line
    for(auto l : polyline->line) {
        // compute line tangent
        auto lt = normalize(polyline->pos[l.y]-polyline->pos[l.x]);
        // accumulate segment tangent to vertex tangent on each vertex
        for (auto i : range(2)) polyline->norm[l[i]] += lt;
    }
    // normalize all vertex tangents
    for (auto& t : polyline->norm) t = normalize(t);
}

// apply Catmull-Clark mesh subdivision
// DOES subdivide texcoord
void subdivide_mesh(Mesh* subdiv) {
    // YOUR CODE GOES HERE ---------------------
    // skip is needed
    if(!subdiv->subdivision_level) return;
    // allocate a working Mesh copied from the subdiv
    Mesh* catmull = new Mesh(*subdiv);
    // foreach level
    for(int lvl : range(subdiv->subdivision_level))
    {
        // make empty pos and quad arrays
        vector<vec3f> npos = vector<vec3f>();
        vector<vec3f> norm = vector<vec3f>();
        vector<vec4i> nquad = vector<vec4i>();
        vector<vec2f> texcoord = vector<vec2f>();
        // create edge_map from current mesh
        EdgeMap emap = EdgeMap(catmull->triangle, catmull->quad);
        // linear subdivision - create vertices
        // copy all vertices from the current mesh
        for(vec3f v : catmull->pos)
            npos.push_back(vec3f(v));
        
        for(vec3f v : catmull->norm)
            norm.push_back(vec3f(v));
        
        for(vec2f v : catmull->texcoord)
            texcoord.push_back(vec2f(v));
        // add vertices in the middle of each edge (use EdgeMap)
        for(auto edge : emap.edges()){
            npos.push_back((catmull->pos[edge.x]+catmull->pos[edge.y])/2.f);
            texcoord.push_back((catmull->texcoord[edge.x]+catmull->texcoord[edge.y])/2.f);
            norm.push_back((catmull->norm[edge.x]+catmull->norm[edge.y])/2.f);
        }
        // add vertices in the middle of each triangle
        for(auto tri : catmull->triangle){
            npos.push_back((catmull->pos[tri.x]+catmull->pos[tri.y]+catmull->pos[tri.z])/3.f);
            texcoord.push_back((catmull->texcoord[tri.x]+catmull->texcoord[tri.y]+catmull->texcoord[tri.z])/3.f);
        }
        // add vertices in the middle of each quad
        for(auto quad : catmull->quad){
            npos.push_back((catmull->pos[quad.x]+catmull->pos[quad.y]+catmull->pos[quad.z]+catmull->pos[quad.w])/4.f);
            texcoord.push_back((catmull->texcoord[quad.x]+catmull->texcoord[quad.y]+catmull->texcoord[quad.z]+catmull->texcoord[quad.w])/4.f);
            norm.push_back((catmull->norm[quad.x]+catmull->norm[quad.y]+catmull->norm[quad.z]+catmull->norm[quad.w])/4.f);
        }
        // subdivision pass --------------------------------
        // compute an offset for the edge vertices //nelle pos dove cominciano gli edge? ecc
        int edgeVertices = catmull->pos.size();
        // compute an offset for the triangle vertices
        int triangleVertices = edgeVertices + emap.edges().size();//precedente piu tutti gli spigoli
        // compute an offset for the quad vertices
        int quadVertices = triangleVertices + catmull->triangle.size();
        // foreach triangle
        for(int t : range(catmull->triangle.size()))
        {
            // add three quads to the new quad array
            //?
            auto triangle = catmull->triangle[t];
            for(int i : range(3))
            {
                auto stri = vec4i(triangle[i], edgeVertices+emap.edge_index(vec2i(triangle[i], triangle[(i+1)%3])), triangleVertices+t, edgeVertices+emap.edge_index(vec2i(triangle[i], triangle[(i+2)%3])));
                nquad.push_back(stri);
            }
        }
        // foreach quad
        for(int q : range(catmull->quad.size()))
        {
            auto quad = catmull->quad[q];
            // add four quads to the new quad array
            for(int i : range(4))
            {
                //prendo il vertice di bordo facendo lookup tra i vertici del quad adj
                auto squad = vec4i(quad[i], edgeVertices+emap.edge_index(vec2i(quad[i], quad[(i+1)%4])), quadVertices+q, edgeVertices+emap.edge_index(vec2i(quad[i], quad[(i+3)%4])));
                nquad.push_back(squad);
            }
        }
        // set new arrays pos, quad back into the working mesh; clear triangle array
        catmull->pos = npos;
        catmull->quad = nquad;
        catmull->texcoord = texcoord;
        catmull->norm = norm;
        catmull->triangle = vector<vec3i>();
    }
    // clear subdivision
    catmull->subdivision_level = 0;
    // according to smooth, either smooth_normals or facet_normals
    //smooth_normals(catmull);
    //facet_normals(catmull);
    // copy back
    *subdiv = *catmull;
    // clear
    free(catmull);
}

// apply Catmull-Clark mesh subdivision
// does not subdivide texcoord
void subdivide_catmullclark(Mesh* subdiv) {
    // YOUR CODE GOES HERE ---------------------
    // skip is needed
    if(!subdiv->subdivision_catmullclark_level) return;
    // allocate a working Mesh copied from the subdiv
    Mesh* catmull = new Mesh(*subdiv);
    // foreach level
    for(int lvl : range(subdiv->subdivision_catmullclark_level))
    {
        // make empty pos and quad arrays
        vector<vec3f> npos = vector<vec3f>();
        vector<vec4i> nquad = vector<vec4i>();
        // create edge_map from current mesh
        EdgeMap emap = EdgeMap(catmull->triangle, catmull->quad);
        // linear subdivision - create vertices
        // copy all vertices from the current mesh
        for(vec3f v : catmull->pos)
            npos.push_back(vec3f(v));
        // add vertices in the middle of each edge (use EdgeMap)
        for(auto edge : emap.edges())
            npos.push_back((catmull->pos[edge.x]+catmull->pos[edge.y])/2.f);
        // add vertices in the middle of each triangle
        for(auto tri : catmull->triangle)
            npos.push_back((catmull->pos[tri.x]+catmull->pos[tri.y]+catmull->pos[tri.z])/3.f);
        // add vertices in the middle of each quad
        for(auto quad : catmull->quad)
            npos.push_back((catmull->pos[quad.x]+catmull->pos[quad.y]+catmull->pos[quad.z]+catmull->pos[quad.w])/4.f);
        // subdivision pass --------------------------------
        // compute an offset for the edge vertices //nelle pos dove cominciano gli edge? ecc
        int edgeVertices = catmull->pos.size();
        // compute an offset for the triangle vertices
        int triangleVertices = edgeVertices + emap.edges().size();//precedente piu tutti gli spigoli
        // compute an offset for the quad vertices
        int quadVertices = triangleVertices + catmull->triangle.size();
        // foreach triangle
        for(int t : range(catmull->triangle.size()))
        {
            // add three quads to the new quad array
            //?
            auto triangle = catmull->triangle[t];
            for(int i : range(3))
            {
                auto stri = vec4i(triangle[i], edgeVertices+emap.edge_index(vec2i(triangle[i], triangle[(i+1)%3])), triangleVertices+t, edgeVertices+emap.edge_index(vec2i(triangle[i], triangle[(i+2)%3])));
                nquad.push_back(stri);
            }
        }
        // foreach quad
        for(int q : range(catmull->quad.size()))
        {
            auto quad = catmull->quad[q];
            // add four quads to the new quad array
            for(int i : range(4))
            {
                //prendo il vertice di bordo facendo lookup tra i vertici del quad adj
                auto squad = vec4i(quad[i], edgeVertices+emap.edge_index(vec2i(quad[i], quad[(i+1)%4])), quadVertices+q, edgeVertices+emap.edge_index(vec2i(quad[i], quad[(i+3)%4])));
                nquad.push_back(squad);
            }
        }
        // averaging pass ----------------------------------
        // create arrays to compute pos averages (avg_pos, avg_count)
        vector<vec3f> avg_pos = vector<vec3f>(npos.size());
        int avg_count[npos.size()];
        // arrays have the same length as the new pos array, and are init to zero
        for(int i : range(npos.size()))
        {
            avg_pos[i] = zero3f;
            avg_count[i] = 0;
        }
        // for each new quad
        for(auto quad : nquad)
        {
            // compute quad center using the new pos array
            auto c = (npos[quad.x]+npos[quad.y]+npos[quad.z]+npos[quad.w])/4.f;
            // foreach vertex index in the quad
            for(int vidx : range(4))
            {
                int idx = quad[vidx];
                if(idx >= npos.size())
                    message("");
                avg_pos[idx] += c;
                avg_count[idx]++;
            }
        }
        // normalize avg_pos with its count avg_count
        for(int i : range(npos.size()))
            avg_pos[i] /= avg_count[i];
        // correction pass ----------------------------------
        // foreach pos, compute correction p = p + (avg_p - p) * (4/avg_count)
        for(int i : range(npos.size()))
            npos[i] += (avg_pos[i] - npos[i]) * (4.f / avg_count[i]);
        // set new arrays pos, quad back into the working mesh; clear triangle array
        catmull->pos = npos;
        catmull->quad = nquad;
        catmull->triangle = vector<vec3i>();
    }
    // clear subdivision
    catmull->subdivision_catmullclark_level = 0;
    // according to smooth, either smooth_normals or facet_normals
    if(catmull->subdivision_catmullclark_smooth) smooth_normals(catmull);
    else facet_normals(catmull);
    // copy back
    *subdiv = *catmull;
    // clear
    free(catmull);
}

// subdivide bezier spline into line segments (assume bezier has only bezier segments and no lines)
void subdivide_bezier(Mesh* bezier) {
    // YOUR CODE GOES HERE ---------------------
    // skip is needed
    if(!bezier->subdivision_bezier_level) return;
    // allocate a working polyline from bezier
    Mesh* polyline = new Mesh(*bezier);
    // foreach level
    for(int lvl : range(bezier->subdivision_bezier_level))
    {
        // make new arrays of positions and bezier segments
        // copy all the vertices into the new array (this waste space but it is easier for now)
        vector<vec3f> npos(polyline->pos); //nella mesh non c'è un puntatore posso tenere sullo stack
        vector<vec4i> segments = vector<vec4i>();
        // foreach bezier segment
        for(vec4i segment : polyline->spline)
        {
            // apply subdivision algorithm
            vec3f q0 = (polyline->pos[segment.x]+polyline->pos[segment.y])/2.f;
            vec3f q2 = (polyline->pos[segment.z]+polyline->pos[segment.w])/2.f;
            vec3f r0 = (q0/2.f+(polyline->pos[segment.y]+polyline->pos[segment.z])/4.f);
            vec3f r1 = (q2/2.f+(polyline->pos[segment.y]+polyline->pos[segment.z])/4.f);
            vec3f s = (r0+r1)/2.f;
            // prepare indices for two new segments
            int lastIndex = npos.size();
            auto spline0 = vec4i(segment.x, lastIndex+1, lastIndex+2, lastIndex);
            auto spline1 = vec4i(lastIndex, lastIndex+3, lastIndex+4, segment.w);
            // add mid point
            npos.push_back(s);
            // add points for first segment and fix segment indices
            npos.push_back(q0);
            npos.push_back(r0);
            // add points for second segment and fix segment indices
            npos.push_back(r1);
            npos.push_back(q2);
            // add indices for both segments into new segments array
            segments.push_back(spline0);
            segments.push_back(spline1);
        }
        // set new arrays pos, segments into the working lineset
        polyline->pos = npos; //non è un puntatore
        polyline->spline = segments;
    }
    // copy bezier segments into line segments
    polyline->line.clear();
    for(vec4i seg : polyline->spline)
    {
        polyline->line.push_back(vec2i(seg.x, seg.y));
        polyline->line.push_back(vec2i(seg.y, seg.z));
        polyline->line.push_back(vec2i(seg.z, seg.w));
    }
    // clear bezier array from lines
    polyline->spline.clear();
    // run smoothing to get proper tangents
    smooth_tangents(polyline);
    // copy back
    *bezier = *polyline;
    // clear
    free(polyline);
}

Mesh* make_surface_mesh(frame3f frame, float radius, bool isquad, Material* mat, float offset) {
    auto mesh = new Mesh{};
    mesh->frame = frame;
    mesh->mat = mat;
    if(isquad) {
        mesh->pos = { {-radius,-radius,-offset}, {radius,-radius,-offset},
            {radius,radius,-offset}, {-radius,radius,-offset} };
        mesh->norm = {z3f,z3f,z3f,z3f};
        mesh->quad = { {0,1,2,3} };
    } else {
        map<pair<int,int>,int> vid;
        for(auto j : range(64+1)) {
            for(auto i : range(128+1)) {
                auto u = 2 * pif * i / 64.0f, v = pif * j / 32.0f;
                auto d = vec3f{cos(u)*sin(v),sin(u)*sin(v),cos(v)};
                vid[{i,j}] = mesh->pos.size();
                mesh->pos.push_back(d*radius*(1-offset));
                mesh->norm.push_back(d);
            }
        }
        for(auto j : range(64)) {
            for(auto i : range(128)) {
                mesh->quad.push_back({vid[{i,j}],vid[{i+1,j}],vid[{i+1,j+1}],vid[{i,j+1}]});
            }
        }
    }
    return mesh;
}

void subdivide_surface(Surface* surface) {
    surface->_display_mesh = make_surface_mesh(
        surface->frame, surface->radius, surface->isquad, surface->mat);
}

void apply_bump(Mesh* mesh)
{
    auto tex = mesh->mat->bump_txt;
    for(int i = 0; i < mesh->pos.size(); i++)
    {
        auto x = (tex->width()-1)*mesh->texcoord[i].x;
        auto y = (tex->height()-1)*mesh->texcoord[i].y;
        auto tcord = tex->at(x, y);
        mesh->pos[i] += mesh->norm[i] * mesh->mat->bump_factor * length(tcord);//sqrt(length(tcord)* mesh->mat->bump_factor);//length(tcord);
    }
    //facet_normals(mesh);
    smooth_normals(mesh);
    //mesh->subdivision_catmullclark_level = 2;
    //subdivide_catmullclark(mesh);
}

float get_quad_area(Mesh* m, int idx)
{
    auto quad = m->quad[idx];
    vec3f a,b,c,d;
    a = m->pos[quad.x];
    b = m->pos[quad.y];
    c = m->pos[quad.z];
    d = m->pos[quad.w];
    auto bd = d-b;
    auto ac = c-a;
    return length(cross(ac,bd))/2.f;
}

float get_triangle_area(Mesh* m, int idx)
{
    auto tri = m->triangle[idx];
    vec3f a,b,c;
    a = m->pos[tri.x];
    b = m->pos[tri.y];
    c = m->pos[tri.z];
    auto ab = b-a;
    auto ac = c-a;
    return length(cross(ab,ac))/2.f;
}

struct compute_area : public std::unary_function<int,double> {
    Mesh* mesh;
    bool operator() (int idx) {return get_triangle_area(mesh, idx);}
};

vec3f get_random_point_triangle(Mesh* mesh, vec3i tri, std::mt19937& generator)
{
    auto A = mesh->pos[tri.x];
    auto B = mesh->pos[tri.y];
    auto C = mesh->pos[tri.z];
    //u = 1 - sqrt(r1) and v = r2 * sqrt(r1)
    auto r1 = sqrt(generator()/((float)generator.max()));
    auto r2 = generator()/((float)generator.max());
    auto u = 1 - r1;
    auto v = r2 * r1;
    auto p = A + u*(B-A) + v*(C-A);
    return p;
}

vec3f get_random_point_quad(Mesh* mesh, vec4i quad, std::mt19937& generator)
{
    if((generator()/((float)generator.max())) > .5f)
    {
        return get_random_point_triangle(mesh, {quad.x, quad.y,quad.z}, generator);
    }
    return get_random_point_triangle(mesh, {quad.z, quad.w,quad.x}, generator);
    auto A = mesh->pos[quad.x];
    auto B = mesh->pos[quad.y];
    auto C = mesh->pos[quad.w];
    //u = 1 - sqrt(r1) and v = r2 * sqrt(r1)
    auto r1 = sqrt((generator()-generator.min())/((float)generator.max()+1));
    auto r2 = (generator()-generator.min())/((float)generator.max()+1);
    auto u = 1 - r1;
    auto v = r2 * r1;
    auto p = A + u*(B-A) + v*(C-A);
    return p;
}

void grow_hair(Mesh* mesh)
{
    for(auto quad : mesh->quad){
        mesh->triangle.push_back({quad.x,quad.y,quad.z});
        mesh->triangle.push_back({quad.z,quad.w,quad.x});
    }
    mesh->quad.clear();
    vector<float> weights = vector<float>();
    for(int i = 0; i < mesh->triangle.size(); i++)
        weights.push_back(get_triangle_area(mesh, i));
    std::random_device rd;
    std::mt19937 generator(rd());  // mt19937 is a standard mersenne_twister_engine
    std::discrete_distribution<> dist(weights.begin(), weights.end());//dist(size, 0, size, op);
    for (int i=0; i < mesh->mat->hair_count; ++i) {
        int t = dist(generator);
        auto tri = mesh->triangle[t];
        // calculate position
        vec3f random_point = get_random_point_triangle(mesh, tri, generator);
        //vec3f normal = t >= size ? (normalize(cross(mesh->pos[quad[1]]-mesh->pos[quad[0]], mesh->pos[quad[2]]-mesh->pos[quad[0]]))+normalize(cross(mesh->pos[quad[3]]-mesh->pos[quad[2]], mesh->pos[quad[0]]-mesh->pos[quad[2]])))/2.f : normalize(cross(mesh->pos[tri.y]-mesh->pos[tri.x], mesh->pos[tri.z]-mesh->pos[tri.x]));
        //la normale e' interpolata
        vec3f normal = normalize((((mesh->norm[tri.x] + mesh->norm[tri.y] + mesh->norm[tri.z])/3.f)/((mesh->pos[tri.x] + mesh->pos[tri.y] + mesh->pos[tri.z])/3.f))*random_point);
        int pos = mesh->pos.size();
        mesh->pos.push_back(random_point);
        mesh->pos.push_back(random_point + normal*0.4f);
        mesh->pos.push_back(random_point + normal*0.4f+vec3f(0.0f,-0.4f,0.0f));
        mesh->pos.push_back(random_point + normal*0.4f+vec3f(0.0f,-0.8f,0.0f));
        mesh->spline.push_back({pos, pos+1, pos+2, pos+3});
    }
    mesh->subdivision_bezier_level=3;
    //If your parallelogram is defined by the points ABCD such that AB, BC, CD and DA are the sides, then take your point as being:
    
    //p = A + uAB + bAD
    
    //P = (1 - sqrt(r1)) * A + (sqrt(r1) * (1 - r2)) * B + (sqrt(r1) * r2) * C
    
    //http://forum.devmaster.net/t/uniform-random-point-inside-a-triangle/12973
}

void subdivide(Scene* scene) {
    for(auto mesh : scene->meshes) {
        if(mesh->mat->hair_count)
            grow_hair(mesh);
        if(mesh->subdivision_catmullclark_level) subdivide_catmullclark(mesh);
        if(mesh->subdivision_level) subdivide_mesh(mesh);
        if(mesh->subdivision_bezier_level) subdivide_bezier(mesh);
        if(mesh->mat->bump_txt)
            apply_bump(mesh);
    }
    for(auto surface : scene->surfaces) {
        subdivide_surface(surface);
    }
}
