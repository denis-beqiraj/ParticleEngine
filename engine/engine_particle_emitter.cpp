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

Eng::ParticleEmitter Eng::ParticleEmitter::empty = Eng::ParticleEmitter(std::shared_ptr<std::vector<Particle>>());



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
    std::shared_ptr<std::vector<Particle>> particles;
#endif
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

#endif

    return NULL;
}

void ENG_API Eng::ParticleEmitter::respawnParticle(Particle* particle) const
{
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle->currentPosition = particle->initPosition; // TODO(jan): learnopengl has an offset parameter here. Do we need it?
    particle->color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle->currentLife = particle->initLife;
    particle->currentVelocity = particle->initVelocity; // TODO(jan): calculate better initial velocity
    //particle->velocity = glm::vec3(((float)rand() / RAND_MAX) * 2.0f - 1.0f, 2.0f, 0.0f); // TODO(jan): calculate better initial velocity
    particle->currentAcceleration = particle->initAcceleration; // TODO(jan): calculate better initial 
    //particle->acceleration = glm::vec3(0.0f, -2.8f, 0.0f); // TODO(jan): calculate better initial velocity
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
    glm::vec3 position = glm::vec3(getMatrix()[3]);
    // Spawn new particles
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
    auto particleSsbo= reserved->computePipe.render();
    //THINGS TO DO WHEN DRAW IN FRAGMENT SHADER
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for (int i = 0; i < reserved->particles->size();i++) {
        glm::mat4 model = renderData.modelViewMat;
        model = glm::translate(model,glm::vec3(particleSsbo[i].currentPosition));
        //std::cout << glm::to_string(model[3])<<std::endl;

        reserved->particlePipe.setModel(model);
        reserved->particlePipe.render(reserved->texture);
    }
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //reserved->computePipe.render();
#endif
    // Done:
    return true;
}

void ENG_API Eng::ParticleEmitter::setTexture(const Eng::Bitmap& sprite)
{
    reserved->texture.load(sprite);
}
