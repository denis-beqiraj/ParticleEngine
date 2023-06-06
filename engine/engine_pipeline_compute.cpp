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

static const std::string LOCAL_SIZE = "8";

static const std::string pipeline_cs = R"(

// This is the (hard-coded) workgroup size:
layout (local_size_x = )"+LOCAL_SIZE+R"() in;

uniform float dT;
uniform float planeMinimum;
uniform float bounciness;

////////////
// LIGHTS //
////////////
   
struct ParticleCompute 
{    
	vec4 initPosition, initVelocity, initAcceleration;
	vec4 currentPosition, currentVelocity, currentAcceleration;
	vec4 colorStart;
    vec4 colorEnd;
	float initLife;
	float currentLife;
	float minLife;
    float scale;
};

struct ParticleTransform
{
    vec3 position;
    float scale;
    vec4 color;
};

layout(std430, binding=0) buffer ParticleData
{     
    ParticleCompute particles[];
};

layout(std430, binding=1) buffer ParticleTransforms
{
    ParticleTransform transforms[];
};

uint rng_state;

uint rand()
{
    // Xorshift algorithm from George Marsaglia's paper
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return rng_state;
}

float rand01()
{
    return float(rand()) * (1.0 / 4294967296.0);
}

//////////
// MAIN //
//////////

void main()
{   
   
    // Pixel coordinates:
    uint i = gl_GlobalInvocationID.x;
    rng_state=i;
    if(i>particles.length()){
    return;
    }
    particles[i].currentLife -= dT;
    if (particles[i].currentLife < particles[i].minLife) {
        // Spawn new particle
        float rColor = 0.5f + ((rand() % 100) / 100.0f);
        particles[i].currentPosition = particles[i].initPosition; // TODO(jan): learnopengl has an offset parameter here. Do we need it?
        particles[i].currentLife = particles[i].initLife;
        particles[i].currentVelocity = particles[i].initVelocity; // TODO(jan): calculate better initial velocity
        particles[i].currentAcceleration = particles[i].initAcceleration; // TODO(jan): calculate better initial 
        particles[i].scale = rand01()*5.0f;
    } else {
        // Update particle
        particles[i].currentPosition = particles[i].currentPosition + particles[i].currentVelocity*dT;
        particles[i].currentVelocity = particles[i].currentVelocity + particles[i].currentAcceleration*dT;
        if(particles[i].currentPosition.y<planeMinimum){
            particles[i].currentPosition.y=planeMinimum;
            particles[i].currentVelocity.y=-particles[i].currentVelocity.y*bounciness;
        }
    }

    transforms[i].position = particles[i].currentPosition.xyz;
    transforms[i].scale = particles[i].scale;
    float t = (particles[i].currentLife - particles[i].minLife) / (particles[i].initLife - particles[i].minLife);
    transforms[i].color = mix(particles[i].colorStart, particles[i].colorEnd, 1.0f - t);
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
    unsigned int particleSize;
    glm::mat4 model;
    Eng::Ssbo particles;
    Eng::Ssbo particleMatrices;
    float dT;
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
void ENG_API Eng::PipelineCompute::render()
{

    // Lazy-loading:
    if (this->isDirty()) {
        if (!this->init())
        {
            ENG_LOG_ERROR("Unable to render (initialization failed)");
        }
    }

    // Apply program:
    Eng::Program& program = getProgram();
    if (program == Eng::Program::empty)
    {
        ENG_LOG_ERROR("Invalid program");
    }
    program.render();
    reserved->particles.render(0);
    reserved->particleMatrices.render(1);
    program.compute(reserved->particleSize); // 8 is the hard-coded size of the workgroup
    program.wait();
}

bool ENG_API Eng::PipelineCompute::convert(std::shared_ptr<std::vector<Eng::ParticleEmitter::Particle>> particles)
{
    std::vector<Eng::PipelineCompute::ComputeParticle> particleSsbovs;
    for (auto& particle : *particles) {
        Eng::PipelineCompute::ComputeParticle pSsbos;
        pSsbos.initPosition = particle.initPosition;
        pSsbos.currentPosition = particle.currentPosition;
        pSsbos.initVelocity = particle.initVelocity;
        pSsbos.currentVelocity = particle.currentVelocity;
        pSsbos.initAcceleration = particle.initAcceleration;
        pSsbos.currentAcceleration = particle.currentAcceleration;
        pSsbos.initLife = particle.initLife;
        pSsbos.currentLife = particle.currentLife;
        pSsbos.minLife = particle.minLife;
        pSsbos.colorStart = particle.colorStart;
        pSsbos.colorEnd = particle.colorEnd;
        pSsbos.scale = particle.scale;
        particleSsbovs.push_back(pSsbos);
    }
    reserved->particles.create(particleSsbovs.size() * sizeof(Eng::PipelineCompute::ComputeParticle), particleSsbovs.data());
    std::vector<glm::mat4> particleMatricesSsbovs;
    particleMatricesSsbovs.resize(particleSsbovs.size(), glm::mat4(1.0f));
    reserved->particleMatrices.create(particleSsbovs.size() * sizeof(glm::mat4), particleMatricesSsbovs.data());
    reserved->particleSize = glm::pow(2, glm::ceil(glm::log(particles->size()) / glm::log(2))) /std::stoi(LOCAL_SIZE);
    return true;
}

Eng::Ssbo ENG_API* Eng::PipelineCompute::getMatricesSsbo() {
    return &reserved->particleMatrices;
}
