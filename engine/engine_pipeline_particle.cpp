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
static const std::string pipeline_vs = R"(

// Out:
out vec2 texCoord;
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
uniform mat4 projection;
uniform mat4 model;

void main()
{   
   texCoord = vertex.zw;
   gl_Position = projection*model*vec4(vertex.xy, 0.0, 1.0);
})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
static const std::string pipeline_fs = R"(
   
// Uniform:
#ifdef ENG_BINDLESS_SUPPORTED
   layout (bindless_sampler) uniform sampler2D texture0;
#else
   layout (binding = 0) uniform sampler2D texture0;    
#endif

// In:   
in vec2 texCoord;
   
// Out:
out vec4 outFragment;


void main()
{
   outFragment = texture(texture0, texCoord);   
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
    Eng::Program program;
    Eng::Vao vao;  ///< Dummy VAO, always required by context profiles
    unsigned int particle;
    glm::mat4 model;

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
    reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs);
    reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs);
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





    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.
 * @param camera view camera
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineParticle::render(const Eng::Texture& texture)
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
    program.setMat4("projection", glm::perspective(glm::radians(45.0f), 1024.0f/768.0f, 1.0f, 1000.0f));
    program.setMat4("model", reserved->model);
    glBindVertexArray(reserved->particle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Done:   
    return true;
}
