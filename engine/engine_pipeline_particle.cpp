/**
 * @file		engine_pipeline_fullscreen2d.cpp
 * @brief	A pipeline for rendering a texture to the fullscreen in 2D
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



 //////////////
 // #INCLUDE //
 //////////////

    // Main include:
#include "engine.h"

// OGL:      
#include <GL/glew.h>
#include <GLFW/glfw3.h>



/////////////
// SHADERS //
/////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline vertex shader.
 */
static const std::string pipeline_vs_3 = R"(

layout(std430, binding=1) buffer ParticleTransforms
{
    mat4 wTms[];
};

// Out:
out vec2 texCoord;
out vec4 color; // New variable for color

// In:
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

// Uniforms:
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    mat4 wTms1 = wTms[gl_InstanceID];
    color = wTms1[0];
    wTms1[0] = vec4(0.0f);
    gl_Position = view*wTms1 * vec4(vertex.xy, 0.0, 1.0);
    texCoord = vertex.zw;
})";

static const std::string pipeline_gs = R"(

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

// In:
in vec2 texCoord[];
in vec4 color[]; // New input for color

// Out:
out vec2 texCoordG;
out vec4 colorG; // Pass the color to fragment shader

// Uniforms:
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{

    vec4 p = gl_in[0].gl_Position;
    // Lower left vertex:
    {
        vec2 pv = p.xy + vec2(-0.5, -0.5);
        gl_Position = projection * vec4(pv, p.zw);
        texCoordG = vec2(0.0, 0.0);
        colorG = color[0]; // Pass the color from vertex shader
        EmitVertex();
    }

    // Upper left vertex:
    {
        vec2 pv = p.xy + vec2(-0.5, 0.5);
        gl_Position = projection * vec4(pv, p.zw);
        texCoordG = vec2(0.0, 1.0);
        colorG = color[0]; // Pass the color from vertex shader
        EmitVertex();
    }

    // Lower right vertex:
    {
        vec2 pv = p.xy + vec2(0.5, -0.5);
        gl_Position = projection * vec4(pv, p.zw);
        texCoordG = vec2(1.0, 0.0);
        colorG = color[0]; // Pass the color from vertex shader
        EmitVertex();
    }

    // Upper right vertex:
    {
        vec2 pv = p.xy + vec2(0.5, 0.5);
        gl_Position = projection * vec4(pv, p.zw);
        texCoordG = vec2(1.0, 1.0);
        colorG = color[0]; // Pass the color from vertex shader
        EmitVertex();
    }

    EndPrimitive();
}
)";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
static const std::string pipeline_fs_3 = R"(
   
// Uniform:
#ifdef ENG_BINDLESS_SUPPORTED
   layout (bindless_sampler) uniform sampler2D texture0;
#else
   layout (binding = 0) uniform sampler2D texture0;    
#endif

// In:   
in vec2 texCoordG;
in vec4 colorG; // New input for color

// Out:
out vec4 outFragment;

void main()
{
    vec4 particle = texture(texture0, texCoordG);
    outFragment = particle*colorG; // Output the color passed from the vertex shader
})";

/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineParticle reserved structure.
 */
struct Eng::PipelineParticle::Reserved
{
    Eng::Shader vs;
    Eng::Shader fs;
    Eng::Shader gs;
    Eng::Program program;
    Eng::Vao vao;  ///< Dummy VAO, always required by context profiles
    unsigned int particle;
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    /**
     * Constructor.
     */
    Reserved()
    {}
};



////////////////////////////////////////
// BODY OF CLASS PipelineParticle //
////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineParticle::PipelineParticle() : reserved(std::make_unique<Eng::PipelineParticle::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineParticle::PipelineParticle(const std::string& name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelineParticle::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelineParticle::PipelineParticle(PipelineParticle&& other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
{
    ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineParticle::~PipelineParticle()
{
    ENG_LOG_DETAIL("[-]");
    if (this->isInitialized())
        free();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineParticle::init()
{
    // Already initialized?
    if (this->Eng::Managed::init() == false)
        return false;
    if (!this->isDirty())
        return false;

    // Build:
    reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs_3);
    reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs_3);
    reserved->gs.load(Eng::Shader::Type::geometry, pipeline_gs);
    if (reserved->program.build({ reserved->vs, reserved->fs, reserved->gs }) == false)
    {
        ENG_LOG_ERROR("Unable to build fullscreen2D program");
        return false;
    }
    this->setProgram(reserved->program);

    // Init dummy VAO:
    if (reserved->vao.init() == false)
    {
        ENG_LOG_ERROR("Unable to init VAO for fullscreen2D");
        return false;
    }





    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
    };
    glGenVertexArrays(1, &reserved->particle);
    glGenBuffers(1, &VBO);
    glBindVertexArray(reserved->particle);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    // Done: 
    this->setDirty(false);
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Releases this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineParticle::free()
{
    if (this->Eng::Managed::free() == false)
        return false;

    // Done:   
    return true;
}


void ENG_API Eng::PipelineParticle::setModel(glm::mat4 model)
{
    reserved->model = model;
}

void ENG_API Eng::PipelineParticle::setView(glm::mat4 view)
{
    reserved->view = view;
}

void ENG_API Eng::PipelineParticle::setProjection(glm::mat4 projection)
{
    reserved->projection = projection;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.
 * @param camera view camera
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineParticle::render(const Eng::Texture& texture, unsigned int particleCount)
{
    // Safety net:
    if (texture == Eng::Texture::empty)
    {
        ENG_LOG_ERROR("Invalid params");
        return false;
    }

    // Lazy-loading:
    if (this->isDirty())
        if (!this->init())
        {
            ENG_LOG_ERROR("Unable to render (initialization failed)");
            return false;
        }

    // Apply program:
    Eng::Program& program = getProgram();
    if (program == Eng::Program::empty)
    {
        ENG_LOG_ERROR("Invalid program");
        return false;
    }
    program.render();
    texture.render(0);
    program.setMat4("projection", reserved->projection);
    program.setMat4("model", reserved->model);
    program.setMat4("view", reserved->view);
    glBindVertexArray(reserved->particle);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArraysInstanced(GL_POINTS, 0, 1, particleCount);
    glBindVertexArray(0);

    // Done:   
    return true;
}
