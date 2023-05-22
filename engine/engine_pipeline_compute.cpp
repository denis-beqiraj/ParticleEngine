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
static const std::string pipeline_cs = R"(

// This is the (hard-coded) workgroup size:
layout (local_size_x = 8) in;



////////////
// LIGHTS //
////////////
   
struct ParticleCompute 
{    
	vec4 initPosition, initVelocity, initAcceleration;
	vec4 currentPosition, currentVelocity, currentAcceleration;
	vec4 color;
	float initLife;
	float currentLife;
	float minLife;
	float pos1;
};
   
layout(std430, binding=0) buffer ParticleData
{     
   ParticleCompute particles[];     
};   




//////////
// MAIN //
//////////

void main()
{   
   // Pixel coordinates:
   uint i = gl_GlobalInvocationID.x;
   if(i>particles.length()){
    return;
   }
   particles[i].initPosition.x=12.0f;
}
)";



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineCompute reserved structure.
 */
struct Eng::PipelineCompute::Reserved
{
    Eng::Shader cs;
    Eng::Program program;
    Eng::Vao vao;  ///< Dummy VAO, always required by context profiles
    unsigned int particle;
    glm::mat4 model;
    Eng::Ssbo particles;
    
    /**
     * Constructor.
     */
    Reserved()
    {}
};



////////////////////////////////////////
// BODY OF CLASS PipelineCompute //
////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineCompute::PipelineCompute() : reserved(std::make_unique<Eng::PipelineCompute::Reserved>())
{
    ENG_LOG_DETAIL("[+]");

    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineCompute::PipelineCompute(const std::string& name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelineCompute::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelineCompute::PipelineCompute(PipelineCompute&& other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
{
    ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineCompute::~PipelineCompute()
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
bool ENG_API Eng::PipelineCompute::init()
{
    // Already initialized?
    if (this->Eng::Managed::init() == false)
        return false;
    if (!this->isDirty())
        return false;

    // Build:
    reserved->cs.load(Eng::Shader::Type::compute, pipeline_cs);
    if (reserved->program.build({ reserved->cs }) == false)
    {
        ENG_LOG_ERROR("Unable to build RayTracing program");
        return false;
    }
    this->setProgram(reserved->program);





    // Done: 
    this->setDirty(false);
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Releases this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineCompute::free()
{
    if (this->Eng::Managed::free() == false)
        return false;

    // Done:   
    return true;
}


void ENG_API Eng::PipelineCompute::setModel(glm::mat4 model)
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
bool ENG_API Eng::PipelineCompute::render()
{

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
    reserved->particles.render(0);
    program.compute(256); // 8 is the hard-coded size of the workgroup
    program.wait();
    auto particles=(Eng::PipelineCompute::ComputeParticle*)reserved->particles.map(Eng::Ssbo::Mapping::read);
    if (particles) {
        std::cout << particles[0].initPosition.x;
    }
    reserved->particles.unmap();
    // Done:   
    return true;
}

bool ENG_API Eng::PipelineCompute::convert(std::shared_ptr<std::vector<Eng::ParticleEmitter::Particle>> particles)
{
    std::vector<Eng::PipelineCompute::ComputeParticle> particleSsbovs;
    for (auto particle : *particles) {
        Eng::PipelineCompute::ComputeParticle pSsbos;
        pSsbos.initPosition = glm::vec4(particle.initPosition,0.0f);
        pSsbos.color = glm::vec4(1.0f);
        particleSsbovs.push_back(pSsbos);
    }
    reserved->particles.create(particleSsbovs.size() * sizeof(Eng::PipelineCompute::ComputeParticle), particleSsbovs.data());
    return true;
}
