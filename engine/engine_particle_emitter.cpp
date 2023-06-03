/**
 * @file		engine_mesh.cpp
 * @brief	Geometric mesh
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



 //////////////
 // #INCLUDE //
 //////////////

    // Main include:
#include "engine.h"

// GLM:
#include <glm/gtc/packing.hpp>  

// OGL:      
#include <GL/glew.h>
#include <GLFW/glfw3.h>  

//#define PE_CUSTOM_CONTAINER



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief ParticleEmitter class reserved structure.
 */
struct Eng::ParticleEmitter::Reserved
{
    std::shared_ptr<std::vector<Particle>> particles;
    Eng::PipelineParticle particlePipe;
    Eng::Texture texture;
    Eng::PipelineCompute computePipe;
};

///////////////////////////////////
// BODY OF CLASS ParticleEmitter //
//////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::ParticleEmitter::ParticleEmitter(std::shared_ptr<std::vector<Particle>> particles) : reserved(std::make_unique<Eng::ParticleEmitter::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    if (particles) {
        reserved->particles = particles;
        reserved->computePipe.convert(reserved->particles);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) : Eng::Node(std::move(other)), reserved(std::move(other.reserved))
{
    ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::ParticleEmitter::~ParticleEmitter()
{
    ENG_LOG_DETAIL("[-]");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Rendering method.
 * @param value generic value
 * @param data generic pointer to any kind of data
 * @return TF
 */
bool ENG_API Eng::ParticleEmitter::render(uint32_t value, void* data) const
{
    auto renderData = *(Eng::ParticleEmitter::ParticleModelView*)data;

    //THINGS TO DO IN COMPUTE SHADER
    // Spawn new particles
    // Update all particles
    reserved->computePipe.render();

    //THINGS TO DO WHEN DRAW IN FRAGMENT SHADER
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    reserved->particlePipe.setModel(renderData.model);
    reserved->particlePipe.setView(renderData.view);
    reserved->particlePipe.render(reserved->texture, reserved->particles->size());
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Done:
    return true;
}

void ENG_API Eng::ParticleEmitter::setTexture(const Eng::Bitmap& sprite)
{
    reserved->texture.load(sprite);
}

void ENG_API Eng::ParticleEmitter::setProjection(glm::mat4 projection)
{
    reserved->particlePipe.setProjection(projection);
}

void ENG_API Eng::ParticleEmitter::setParticles(std::shared_ptr<std::vector<Particle>> particles)
{
    reserved->particles = particles;
    reserved->computePipe.convert(reserved->particles);
}

void ENG_API Eng::ParticleEmitter::setDt(float dT)
{
    reserved->computePipe.getProgram().render();
    reserved->computePipe.getProgram().setFloat("dT", dT);
}

void ENG_API Eng::ParticleEmitter::setPlaneMinimum(float planeMinimum)
{
    reserved->computePipe.getProgram().render();
    reserved->computePipe.getProgram().setFloat("planeMinimum", planeMinimum);
}

void ENG_API Eng::ParticleEmitter::setBounciness(float bounciness)
{
    reserved->computePipe.getProgram().render();
    reserved->computePipe.getProgram().setFloat("bounciness", bounciness);
}
