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

struct ParticleTransform
{
    vec3 position;
    float scale;
    vec4 color;
};

layout(std430, binding=1) buffer ParticleTransforms
{
    ParticleTransform transforms[];
};

// Out:
out vec2 texCoord;
out vec4 color; // New variable for color
out float scale;

// In:
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

// Uniforms:
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
    ParticleTransform particle = transforms[gl_InstanceID];

    color = particle.color;
    scale = particle.scale;

    float half_size = 0.5f * scale;

    vec2 offset;
    if (gl_VertexID == 0 || gl_VertexID == 3) {
        offset = vec2(-half_size, -half_size);
        texCoord = vec2(0.0f, 0.0f);
    } else if (gl_VertexID == 1) {
        offset = vec2(half_size, -half_size);
        texCoord = vec2(1.0f, 0.0f);
    } else if (gl_VertexID == 2 || gl_VertexID == 4) {
        offset = vec2(half_size, half_size);
        texCoord = vec2(1.0f, 1.0f);
    } else {
        offset = vec2(-half_size, half_size);
        texCoord = vec2(0.0f, 1.0f);
    }

    vec4 viewPos = view * model * vec4(particle.position, 1.0);
    vec2 pv = viewPos.xy + offset;
    gl_Position = projection * vec4(pv, viewPos.zw);
})";

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
in vec2 texCoord;
in vec4 color; // New input for color

// Out:
out vec4 outFragment;

void main()
{
    vec4 particle = texture(texture0, texCoord);
    outFragment = particle*color; // Output the color passed from the vertex shader
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
    if (reserved->program.build({ reserved->vs, reserved->fs }) == false)
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
    reserved->vao.render();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particleCount);

    // Done:   
    return true;
}
