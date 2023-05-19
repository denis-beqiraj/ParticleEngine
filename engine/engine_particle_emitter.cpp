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


Eng::ParticleEmitter Eng::ParticleEmitter::empty = Eng::ParticleEmitter(0, 0);



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief ParticleEmitter class reserved structure.
 */
struct Eng::ParticleEmitter::Reserved
{
    Eng::ParticleEmitter::ParticleArrayNode* particles;
    Eng::ParticleEmitter::ParticleArrayNode* particleArrayFreeListHead;
    unsigned int maxParticles;
    unsigned int newParticlesPerFrame;
    Eng::PipelineParticle particlePipe;
    Eng::Texture texture;

};

///////////////////////////////////
// BODY OF CLASS ParticleEmitter //
//////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::ParticleEmitter::ParticleEmitter(unsigned int maxParticles, unsigned int newParticlesPerFrame) : reserved(std::make_unique<Eng::ParticleEmitter::Reserved>())
{
    ENG_LOG_DETAIL("[+]");

    Reserved* pReserved = reserved.get();

    pReserved->maxParticles = maxParticles;
    pReserved->newParticlesPerFrame = newParticlesPerFrame;
    pReserved->particles = (ParticleArrayNode*)malloc(maxParticles * sizeof(ParticleArrayNode));

    // Initialize sparse particle array with free list
    pReserved->particleArrayFreeListHead = pReserved->particles;
    ParticleArrayNode* currentNode = pReserved->particleArrayFreeListHead;
    for (unsigned int i = 0; i < maxParticles; i++) {
        currentNode->isFree = true;
        currentNode->nextFree = currentNode + 1;
        currentNode = currentNode->nextFree;
    }

    ParticleArrayNode* lastNode = pReserved->particleArrayFreeListHead + maxParticles - 1;
    lastNode->nextFree = NULL;
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

Eng::ParticleEmitter::Particle ENG_API* Eng::ParticleEmitter::getFreeParticle() const
{
    Reserved* pReserved = reserved.get();

    if (pReserved->particleArrayFreeListHead != NULL) {
        // Pop free list head
        ParticleArrayNode* node = pReserved->particleArrayFreeListHead;
        pReserved->particleArrayFreeListHead = node->nextFree;

        node->isFree = false;
        node->particle = Particle();
        return &node->particle;
    }

    return NULL;
}

void ENG_API Eng::ParticleEmitter::respawnParticle(Particle* particle, const glm::vec3& position) const
{
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle->position = position; // TODO(jan): learnopengl has an offset parameter here. Do we need it?
    particle->color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle->life = 10.0f;
    particle->velocity = glm::vec3(0.0f, 0.1f, 0.0f); // TODO(jan): calculate better initial velocity
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
    Reserved* pReserved = reserved.get();

    auto renderData = *(Eng::ParticleEmitter::RenderData*)data;
    // TODO(jan): This is the position in view space, it would probably be nice to do these calculations in world space
    glm::vec3 position = glm::vec3(renderData.position[3]);

    // Spawn new particles
    for (unsigned int i = 0; i < pReserved->newParticlesPerFrame; i++) {
        Particle* particle = getFreeParticle();
        if (particle != NULL) {
            respawnParticle(particle, position);
        }
    }
    // Update all particles
    float dT = renderData.dt; // TODO(jan): pass
    ParticleArrayNode* node = pReserved->particles;
    for (unsigned int i = 0; i < pReserved->maxParticles; i++) {
        if (!node->isFree) {
            node->particle.life -= dT;
            if (node->particle.life < 0.0f) {
                node->isFree = true;
                node->nextFree = pReserved->particleArrayFreeListHead;
                pReserved->particleArrayFreeListHead = node;
            } else {
                node->particle.position -= node->particle.velocity * dT;
                node->particle.color.a -= dT * 2.5f;
            }
        }

        node++;
    }
    ParticleArrayNode* drawNode = pReserved->particles;
    for (unsigned int i = 0; i < pReserved->maxParticles; i++) {
        if (!drawNode->isFree) {
            glm::mat4 model(1.0f);
            model[3] = glm::vec4(drawNode->particle.position,1.0f);
            std::cout << glm::to_string(model[3])<<std::endl;
            reserved->particlePipe.setModel(model);
            reserved->particlePipe.render(reserved->texture.getDefault(true));
        }
        drawNode++;
    }
    // Done:
    return true;
}