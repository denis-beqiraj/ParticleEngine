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

Eng::ParticleEmitter Eng::ParticleEmitter::empty = Eng::ParticleEmitter(std::vector<Particle>(), 0);



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief ParticleEmitter class reserved structure.
 */
struct Eng::ParticleEmitter::Reserved
{
#ifdef PE_CUSTOM_CONTAINER
    Eng::ParticleEmitter::ParticleArrayNode* particles;
    Eng::ParticleEmitter::ParticleArrayNode* particleArrayFreeListHead;
#else
    std::vector<Particle> particles;
#endif
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
ENG_API Eng::ParticleEmitter::ParticleEmitter(std::vector<Particle> particles, unsigned int newParticlesPerFrame) : reserved(std::make_unique<Eng::ParticleEmitter::Reserved>())
{
    ENG_LOG_DETAIL("[+]");

    reserved->maxParticles = particles.size();
    reserved->newParticlesPerFrame = newParticlesPerFrame;
    reserved->particles = particles;
#ifdef PE_CUSTOM_CONTAINER
    reserved->particles = (ParticleArrayNode*)malloc(maxParticles * sizeof(ParticleArrayNode));

    // Initialize sparse particle array with free list
    reserved->particleArrayFreeListHead = reserved->particles;
    ParticleArrayNode* currentNode = reserved->particleArrayFreeListHead;
    for (unsigned int i = 0; i < maxParticles; i++) {
        currentNode->isFree = true;
        currentNode->nextFree = currentNode + 1;
        currentNode = currentNode->nextFree;
    }

    ParticleArrayNode* lastNode = reserved->particleArrayFreeListHead + maxParticles - 1;
    lastNode->nextFree = NULL;
#endif
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
#ifdef PE_CUSTOM_CONTAINER
    if (reserved->particleArrayFreeListHead != NULL) {
        // Pop free list head
        ParticleArrayNode* node = reserved->particleArrayFreeListHead;
        reserved->particleArrayFreeListHead = node->nextFree;

        node->isFree = false;
        node->particle = Particle();
        return &node->particle;
    }
#else
    if (reserved->particles.size() < reserved->maxParticles) {
        reserved->particles.push_back(Particle());
        return &reserved->particles[reserved->particles.size() - 1];
    }
#endif

    return NULL;
}

void ENG_API Eng::ParticleEmitter::respawnParticle(Particle* particle, const glm::vec3& position) const
{
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle->position = position + glm::vec3(0.0f, 0.0f, 0.0f); // TODO(jan): learnopengl has an offset parameter here. Do we need it?
    particle->color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle->life = 2.0f;
    particle->velocity = glm::vec3(((float)rand() / RAND_MAX) * 2.0f - 1.0f, 2.0f, 0.0f); // TODO(jan): calculate better initial velocity
    particle->acceleration = glm::vec3(0.0f, -2.8f, 0.0f); // TODO(jan): calculate better initial velocity
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
    auto renderData = *(Eng::ParticleEmitter::RenderData*)data;
    //THINGS TO DO IN COMPUTE SHADER
    glm::vec3 position = glm::vec3(renderData.position[3]);
    // Spawn new particles
    for (unsigned int i = 0; i < reserved->newParticlesPerFrame; i++) {
        Particle* particle = getFreeParticle();
        if (particle != NULL) {
            respawnParticle(particle, position);
        }
    }
    // Update all particles
    float dT = renderData.dt;
#ifdef PE_CUSTOM_CONTAINER
    ParticleArrayNode* node = reserved->particles;
    for (unsigned int i = 0; i < reserved->maxParticles; i++) {
        if (!node->isFree) {
            node->particle.life -= dT;
            if (node->particle.life < 0.0f) {
                node->isFree = true;
                node->nextFree = reserved->particleArrayFreeListHead;
                reserved->particleArrayFreeListHead = node;
            } else {
                node->particle.position -= node->particle.velocity * dT;
                node->particle.color.a -= dT * 2.5f;
            }
        }

        node++;
    }

    ParticleArrayNode* drawNode = reserved->particles;
    for (unsigned int i = 0; i < reserved->maxParticles; i++) {
        if (!drawNode->isFree) {
            glm::mat4 model(1.0f);
            model[3] = glm::vec4(drawNode->particle.position,1.0f);
            std::cout << glm::to_string(model[3])<<std::endl;
            reserved->particlePipe.setModel(model);
            reserved->particlePipe.render(reserved->texture.getDefault(true));
        }
        drawNode++;
    }
#else
    unsigned int particleIndex = 0;
    while (particleIndex < reserved->particles.size()) {
        Particle& particle = reserved->particles[particleIndex];
        particle.life -= dT;
        if (particle.life < 0.0f) {
            reserved->particles[particleIndex] = reserved->particles[reserved->particles.size() - 1];
            reserved->particles.resize(reserved->particles.size() - 1);
        } else {
            particle.position = particle.position + particle.velocity*dT;
            particle.velocity = particle.velocity + particle.acceleration*dT;

            particle.color.a -= dT * 2.5f;

            particleIndex++;
        }
    }
    //THINGS TO DO WHEN DRAW IN FRAGMENT SHADER
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (const Particle& particle : reserved->particles) {
        glm::mat4 model=renderData.position;
        model[3] = glm::vec4(particle.position,1.0f);
        //std::cout << glm::to_string(model[3])<<std::endl;
        reserved->particlePipe.setModel(model);
        reserved->particlePipe.render(reserved->texture);
    }
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

    // Done:
    return true;
}

void ENG_API Eng::ParticleEmitter::setTexture(const Eng::Bitmap& sprite)
{
    reserved->texture.load(sprite);
}
