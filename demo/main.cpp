/**
 * @file		main.cpp
 * @brief	Engine usage example
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



 //////////////
 // #INCLUDE //
 //////////////

    // Main engine header:
#include "engine.h"

// C/C++:
#include <iostream>
#include <chrono>


#include <imgui.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

enum CameraMode {
    CameraMode_None,
    CameraMode_Default,
    CameraMode_FirstPerson
};

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 10.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

//////////   
// VARS //
//////////

   // Mouse status:
double oldMouseX, oldMouseY;
float rotX, rotY;
bool mouseBR, mouseBL;
float transZ = 50.0f;
glm::vec3 startVelocity;
glm::vec3 startAcceleration;
glm::vec3 color;
glm::vec2 initLife;

// Camera:
CameraMode cameraMode;
glm::vec3 firstPersonPosition;
float firstPersonDesiredVelocity;
float firstPersonVelocity;
float firstPersonRot;
float firstPersonRotVelocity;
float firstPersonDesiredRotVelocity;
const float firstPersonVelocityTransitionSpeed = 40.0f;

// Pipelines:
Eng::PipelineDefault dfltPipe;
Eng::PipelineFullscreen2D full2dPipe;

///////////////
// CALLBACKS //
///////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse cursor callback.
 * @param mouseX updated mouse X coordinate
 * @param mouseY updated mouse Y coordinate
 */
void mouseCursorCallback(double mouseX, double mouseY)
{
    if (cameraMode == CameraMode_FirstPerson) {
        // ENG_LOG_DEBUG("x: %.1f, y: %.1f", mouseX, mouseY);
        float xpos = static_cast<float>(mouseX);
        float ypos = static_cast<float>(mouseY);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse button callback.
 * @param button mouse button id
 * @param action action
 * @param mods modifiers
 */
void mouseButtonCallback(int button, int action, int mods)
{
    // ENG_LOG_DEBUG("button: %d, action: %d, mods: %d", button, action, mods);
    switch (button) {
    case 0: mouseBL = (bool)action; break;
    case 1: mouseBR = (bool)action; break;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse scroll callback.
 * @param scrollX updated mouse scroll X coordinate
 * @param scrollY updated mouse scroll Y coordinate
 */
void mouseScrollCallback(double scrollX, double scrollY)
{
    // ENG_LOG_DEBUG("x: %.1f, y: %.1f", scrollX, scrollY);
    transZ -= (float)scrollY;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Keyboard callback.
 * @param key key code
 * @param scancode key scan code
 * @param action action
 * @param mods modifiers
 */
void keyboardCallback(int key, int scancode, int action, int mods)
{
    float cameraSpeed = static_cast<float>(5.0f);
    //ENG_LOG_DEBUG("key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);
    if (cameraMode == CameraMode_Default) {
        switch (key) {
        case 'C': if (action == 0) cameraMode = CameraMode_FirstPerson; break;
        case 'W': if (action == 0) dfltPipe.setWireframe(!dfltPipe.isWireframe()); break;
        }
    }
    else if (cameraMode == CameraMode_FirstPerson) {
        switch (key) {
        case 'C': if (action == 0) cameraMode = CameraMode_Default; break;
        case 'W':
            cameraPos += cameraSpeed * cameraFront;
            break;
        case 'S':
            cameraPos -= cameraSpeed * cameraFront;
            break;
        case 'A':
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            break;
        case 'D':
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            break;
        }
    }
}

float rand01() {
    return (float)rand() / RAND_MAX;
}

float randXY(float x, float y) {
    return rand01() * (y - x) + x;
}

void createParticlesSmoke(std::vector<Eng::ParticleEmitter::Particle>& particles, int maxParticles, glm::vec4 colorStart, glm::vec4 colorEnd) {
    particles.clear();
    for (int i = 0; i < maxParticles; i++) {
        Eng::ParticleEmitter::Particle particle;
        particle.initPosition = glm::vec4(0.0f);
        float alpha = rand01() * glm::two_pi<float>();
        particle.initVelocity = glm::vec4(glm::sin(alpha), 0.0f, glm::cos(alpha), 0.0f);
        particle.initAcceleration = glm::vec4(startAcceleration, 0.0f);
        particle.currentPosition = particle.initPosition;
        particle.currentVelocity = particle.initVelocity;
        particle.currentAcceleration = particle.initAcceleration;
        particle.initLife = rand01() * initLife.x;
        particle.minLife = rand01() * initLife.y;
        particle.currentLife = particle.initLife;
        particle.colorStart = colorStart;
        particle.colorEnd = colorEnd;
        particle.scaleStart = randXY(7.0f, 9.0f);
        particle.scaleEnd = randXY(12.0f, 15.0f);
        particles.push_back(particle);
    }
}

void createParticlesFire(std::vector<Eng::ParticleEmitter::Particle>& particles, int maxParticles, glm::vec4 colorStart, glm::vec4 colorEnd) {
    particles.clear();
    for (int i = 0; i < maxParticles; i++) {
        Eng::ParticleEmitter::Particle particle;
        float alpha = rand01() * glm::two_pi<float>();
        particle.initPosition = glm::vec4(glm::sin(alpha), 0.0f, glm::cos(alpha), 0.0f);
        alpha = rand01() * glm::two_pi<float>();
        particle.initVelocity = glm::vec4(glm::sin(alpha), 0.0f, glm::cos(alpha), 0.0f);
        alpha = rand01() * glm::two_pi<float>();
        particle.initAcceleration = glm::vec4(startAcceleration, 0.0f);
        particle.currentPosition = particle.initPosition;
        particle.currentVelocity = particle.initVelocity;
        particle.currentAcceleration = particle.initAcceleration;
        particle.initLife = rand01() * initLife.x;
        particle.minLife = rand01() * initLife.y * 0.25f;
        particle.currentLife = particle.initLife;
        particle.colorStart = colorStart;
        particle.colorEnd = colorEnd;
        particle.scaleStart = randXY(3.0f, 5.0f);
        particle.scaleEnd = randXY(5.5f, 6.0f);
        particles.push_back(particle);
    }
}

void createParticlesFirework(std::vector<Eng::ParticleEmitter::Particle>& particles, int maxParticles, glm::vec4 colorStart, glm::vec4 colorEnd) {
    particles.clear();
    for (int i = 0; i < maxParticles; i++) {
        Eng::ParticleEmitter::Particle particle;
        particle.initPosition = glm::vec4(0.0f);
        float alpha = rand01() * glm::two_pi<float>();
        particle.initVelocity = glm::vec4(glm::sin(alpha), 0.0f, glm::cos(alpha), 0.0f);
        particle.initAcceleration = glm::vec4(-startAcceleration, 0.0f);
        particle.currentPosition = particle.initPosition;
        particle.currentVelocity = particle.initVelocity;
        particle.currentAcceleration = particle.initAcceleration;
        particle.initLife = rand01() * 20.5f;
        particle.minLife = rand01() * -50.5f;
        particle.currentLife = rand01() * (particle.initLife - particle.minLife) + particle.minLife;
        particle.colorStart = colorStart;
        particle.colorEnd = colorEnd;
        particle.scaleStart = randXY(7.0f, 9.0f);
        particle.scaleEnd = randXY(12.0f, 15.0f);
        particles.push_back(particle);
    }
}

void createParticlesWater(std::vector<Eng::ParticleEmitter::Particle>& particles, int maxParticles, glm::vec4 colorStart, glm::vec4 colorEnd) {
    particles.clear();
    for (int i = 0; i < maxParticles; i++) {
        Eng::ParticleEmitter::Particle particle;
        particle.initPosition = glm::vec4(0.0f);
        particle.initVelocity = glm::vec4(rand01() * 2 - 1, 10.0f, rand01() * 2 - 1, rand01() * 2 - 1);
        particle.initAcceleration = glm::vec4(-startAcceleration, 0.0f);
        particle.currentPosition = particle.initPosition;
        particle.currentVelocity = particle.initVelocity;
        particle.currentAcceleration = particle.initAcceleration;
        particle.initLife = rand01() * 2.5f;
        particle.minLife = rand01() * -10.5f;
        particle.currentLife = particle.minLife + rand01() * (particle.initLife - particle.minLife);
        particle.colorStart = colorStart;
        particle.colorEnd = colorEnd;
        particle.scaleStart = 0.5f;
        particle.scaleEnd = 0.5f;
        particles.push_back(particle);
    }
}

float lerp(float a, float b, float t) {
    t = glm::clamp<float>(t, 0.0f, 1.0f);
    return a * (1.0f - t) + b * t;
}

void updateParticles(std::vector<Eng::ParticleEmitter::Particle>& particles) {
    for (int i = 0; i < particles.size(); i++) {
        particles.at(i).initPosition = glm::vec4(0.0f);
        particles.at(i).initVelocity = glm::vec4(rand01() * 2 - 1, 10.0f, rand01() * 2 - 1, rand01() * 2 - 1);
        particles.at(i).initAcceleration = glm::vec4(-startAcceleration, 0.0f);
        particles.at(i).currentPosition = particles.at(i).initPosition;
        particles.at(i).currentVelocity = particles.at(i).initVelocity;
        particles.at(i).currentAcceleration = particles.at(i).initAcceleration;
        particles.at(i).initLife = rand01() * 2.5f;
        particles.at(i).minLife = rand01() * -10.5f;
        particles.at(i).currentLife = particles.at(i).minLife + rand01() * (particles.at(i).initLife - particles.at(i).minLife);
        particles.at(i).scaleStart = 0.5f;
        particles.at(i).scaleEnd = 0.5f;
    }
}

//////////
// MAIN //
//////////

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char* argv[])
{
    // Credits:
    std::cout << "Engine demo, A. Peternier (C) SUPSI" << std::endl;
    std::cout << std::endl;

    // Init engine:
    Eng::Base& eng = Eng::Base::getInstance();
    eng.init();

    // Register callbacks:
    eng.setMouseCursorCallback(mouseCursorCallback);
    eng.setMouseButtonCallback(mouseButtonCallback);
    eng.setMouseScrollCallback(mouseScrollCallback);
    eng.setKeyboardCallback(keyboardCallback);
    eng.initImgui();

    cameraMode = CameraMode_FirstPerson;
    firstPersonPosition = glm::vec3(0.0f, 17.5f, 0.0f);
    firstPersonVelocity = 0.0f;
    firstPersonDesiredVelocity = 0.0f;
    firstPersonRot = 0.0f;
    firstPersonRotVelocity = 0.0f;
    firstPersonDesiredRotVelocity = 0.0f;

    /////////////////
    // Loading scene:   
    Eng::Ovo ovo;

    std::reference_wrapper<Eng::Node> root = ovo.load("demo.ovo");
    std::vector<Eng::ParticleEmitter::Particle> particlesSmoke;
    std::vector<Eng::ParticleEmitter::Particle> particlesFire;
    std::vector<Eng::ParticleEmitter::Particle> particlesFireRed;
    std::vector<Eng::ParticleEmitter::Particle> particlesFireGreen;
    std::vector<Eng::ParticleEmitter::Particle> particlesFireBlue;
    std::vector<Eng::ParticleEmitter::Particle> particlesFireYellow;
    std::vector<Eng::ParticleEmitter::Particle> particlesWater;

    float value;
    value = 120;
    startVelocity = glm::vec3(8, -2, 8);
    startAcceleration = glm::vec3(0, 2.8, 0);
    color = glm::vec3(1, 0, 0);
    initLife = glm::vec2(2, -5);
    createParticlesSmoke(particlesSmoke, value, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    createParticlesFire(particlesFire, value, glm::vec4(1.0f, 0.9f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
    createParticlesFirework(particlesFireRed, 2000.0f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    createParticlesFirework(particlesFireGreen, 2000.0f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    createParticlesFirework(particlesFireBlue, 2000.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    createParticlesFirework(particlesFireYellow, 2000.0f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
    createParticlesWater(particlesWater, 10000.0f, glm::vec4(0.5f, 0.4f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));


    std::cout << "Scene graph:\n" << root.get().getTreeAsString() << std::endl;

    // Get light ref:
    std::reference_wrapper<Eng::Light> light = dynamic_cast<Eng::Light&>(Eng::Container::getInstance().find("Omni001"));
    light.get().setAmbient({ 0.3f, 0.3f, 0.3f });
    light.get().setColor({ 1.5f, 1.5f, 1.5f });
    light.get().setProjMatrix(glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 1000.0f)); // Orthographic projection
    // light.get().setProjMatrix(glm::perspective(glm::radians(75.0f), 1.0f, 1.0f, 1000.0f)); // Perspective projection         
    // Get torus knot ref:
    std::reference_wrapper<Eng::Mesh> torch = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Box001"));
    std::reference_wrapper<Eng::Mesh> torchBase = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Arm"));
    std::reference_wrapper<Eng::Mesh> firework = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Box002"));
    std::reference_wrapper<Eng::Mesh> firework1 = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Box003"));
    std::reference_wrapper<Eng::Mesh> firework2 = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Box004"));
    std::reference_wrapper<Eng::Mesh> firework3 = dynamic_cast<Eng::Mesh&>(Eng::Container::getInstance().find("Box005"));

    // Rendering elements:
    Eng::List list;
    Eng::Camera camera;
    camera.setProjMatrix(glm::perspective(glm::radians(45.0f), eng.getWindowSize().x / (float)eng.getWindowSize().y, 1.0f, 1000.0f));

    /////////////
    // Main loop:
    std::cout << "Entering main loop..." << std::endl;
    std::chrono::high_resolution_clock timer;
    float fpsFactor = 1.0f;
    float currentFps = 0.0f;
    float bounciness = 0.8f;
    Eng::ParticleEmitter smokeParticleEmitter(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesSmoke));
    Eng::Bitmap sprite;
    sprite.load("smoke.dds");
    smokeParticleEmitter.setTexture(sprite);
    smokeParticleEmitter.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 12.0f, 0.0f)));
    smokeParticleEmitter.setProjection(camera.getProjMatrix());
    torch.get().addChild(smokeParticleEmitter);
    //computePipe.convert(particles);

    // fire
    Eng::ParticleEmitter fireParticleEmitter(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesFire));
    sprite.load("flame.dds");
    fireParticleEmitter.setTexture(sprite);
    fireParticleEmitter.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, 0.0f)));
    fireParticleEmitter.setProjection(camera.getProjMatrix());
    smokeParticleEmitter.addChild(fireParticleEmitter);

    Eng::ParticleEmitter fireworkParticleEmitterRed(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesFireRed));
    sprite.load("flame.dds");
    fireworkParticleEmitterRed.setTexture(sprite);
    fireworkParticleEmitterRed.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 1.5f)));
    fireworkParticleEmitterRed.setProjection(camera.getProjMatrix());
    firework.get().addChild(fireworkParticleEmitterRed);

    Eng::ParticleEmitter fireworkParticleEmitterBlue(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesFireGreen));
    sprite.load("flame.dds");
    fireworkParticleEmitterBlue.setTexture(sprite);
    fireworkParticleEmitterBlue.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 1.5f)));
    fireworkParticleEmitterBlue.setProjection(camera.getProjMatrix());
    firework1.get().addChild(fireworkParticleEmitterBlue);

    Eng::ParticleEmitter fireworkParticleEmitterGreen(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesFireBlue));
    sprite.load("flame.dds");
    fireworkParticleEmitterGreen.setTexture(sprite);
    fireworkParticleEmitterGreen.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 1.5f)));
    fireworkParticleEmitterGreen.setProjection(camera.getProjMatrix());
    firework2.get().addChild(fireworkParticleEmitterGreen);

    Eng::ParticleEmitter fireworkParticleEmitterYellow(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesFireYellow));
    sprite.load("flame.dds");
    fireworkParticleEmitterYellow.setTexture(sprite);
    fireworkParticleEmitterYellow.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 1.5f)));
    fireworkParticleEmitterYellow.setProjection(camera.getProjMatrix());
    firework3.get().addChild(fireworkParticleEmitterYellow);

    Eng::ParticleEmitter waterBounce(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesWater));
    sprite.load("flame.dds");
    waterBounce.setTexture(sprite);
    waterBounce.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    waterBounce.setProjection(camera.getProjMatrix());
    root.get().addChild(waterBounce);


    float seconds = 0.0f;
    float deltaTimeS = 0.0f;
    glm::vec3 torchOffset = glm::vec3(2.0f, 0.0f, 0.0f);
    float plane = -2.0f;
    bool startFireworks = false;
    glm::vec3 velocity = glm::vec3(0.0f, 10.0f, 0.0f);
    startAcceleration.y = -startAcceleration.y;
    value = 10000.0f;
    while (eng.processEvents())
    {
        auto start = timer.now();

        // Update viewpoint:
        if (cameraMode == CameraMode_Default) {
            glm::mat4 cameraMat = glm::inverse(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp));
            camera.setMatrix(cameraMat);
        }
        else if (cameraMode == CameraMode_FirstPerson) {

            // Calculate camera matrix
            glm::mat4 cameraMat = glm::inverse(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp));
            camera.setMatrix(cameraMat);
            torchBase.get().setMatrix(cameraMat * glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -5.0f, -17.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(190.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.7f)));
        }

        // Animate torus knot:      
        // Update list:
        list.reset();
        list.process(root);

        // Main rendering:
        eng.clear();
        //particleEmitter.render(0U,(void*)&data);
        dfltPipe.render(camera, list);
        smokeParticleEmitter.setDt(currentFps);
        smokeParticleEmitter.setPlaneMinimum(-5.0f);

        fireParticleEmitter.setDt(currentFps);
        fireParticleEmitter.setPlaneMinimum(-5.0f);
        if (!startFireworks) {
            fireworkParticleEmitterRed.setDt(currentFps);
            fireworkParticleEmitterRed.setPlaneMinimum(plane);

            fireworkParticleEmitterBlue.setDt(currentFps);
            fireworkParticleEmitterBlue.setPlaneMinimum(plane);

            fireworkParticleEmitterGreen.setDt(currentFps);
            fireworkParticleEmitterGreen.setPlaneMinimum(plane);

            fireworkParticleEmitterYellow.setDt(currentFps);
            fireworkParticleEmitterYellow.setPlaneMinimum(plane);
        }
        else {
            plane = plane - velocity.y * currentFps;
            fireworkParticleEmitterRed.setPlaneMinimum(plane);
            fireworkParticleEmitterBlue.setPlaneMinimum(plane);
            fireworkParticleEmitterGreen.setPlaneMinimum(plane);
            fireworkParticleEmitterYellow.setPlaneMinimum(plane);
        }

        waterBounce.setDt(currentFps);
        waterBounce.setPlaneMinimum(-1.0f);
        //particlePipe.render(tknot.get().getMaterial().getTexture(), list);
        // Uncomment the following two lines for displaying the shadow map:
        // eng.clear();      
        //  full2dPipe.render(dfltPipe.getShadowMappingPipeline().getShadowMap(), list);
        eng.getImgui()->newFrame();
        eng.getImgui()->newText("Fps: " + std::to_string(1.0f / fpsFactor));
        if (eng.getImgui()->newBar("Number particles", value, 1.0f, 200000.0f)) {
            createParticlesWater(particlesWater, value, glm::vec4(0.5f, 0.4f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));
            waterBounce.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesWater));
        }
        eng.getImgui()->newText("Start velocity");
        if (eng.getImgui()->newBar("XV", startVelocity.x, -100.0f, 100.0f) | eng.getImgui()->newBar("YV", startVelocity.y, -100.0f, 100.0f) | eng.getImgui()->newBar("ZV", startVelocity.z, -100.0f, 100.0f)) {
            updateParticles(particlesWater);
            waterBounce.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesWater));
        }
        eng.getImgui()->newText("Start acceleration");
        if (eng.getImgui()->newBar("XA", startAcceleration.x, -100.0f, 100.0f) | eng.getImgui()->newBar("YA", startAcceleration.y, -100.0f, 100.0f) | eng.getImgui()->newBar("ZA", startAcceleration.z, -100.0f, 100.0f)) {
            updateParticles(particlesWater);
            waterBounce.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesWater));
        }
        eng.getImgui()->newText("Life");
        if (eng.getImgui()->newBar("Init life", initLife.x, -100.0f, 100.0f) | eng.getImgui()->newBar("End life", initLife.y, -100.0f, 100.0f)) {
            updateParticles(particlesWater);
            waterBounce.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particlesWater));
        }
        eng.getImgui()->newBar("Bounciness", bounciness, 0.0f, 1.0f);
        waterBounce.setBounciness(bounciness);
        fireworkParticleEmitterRed.setBounciness(-1.0f);
        fireworkParticleEmitterBlue.setBounciness(-1.0f);
        fireworkParticleEmitterGreen.setBounciness(-1.0f);
        fireworkParticleEmitterYellow.setBounciness(-1.0f);
        if (eng.getImgui()->newButton("Start fireworks")) {
            startFireworks = true;
        }
        eng.getImgui()->render();
        eng.swap();
        if (startFireworks) {
            firework.get().setMatrix(firework.get().getMatrix() * glm::translate(glm::mat4(1.0f), velocity * currentFps));
            firework1.get().setMatrix(firework1.get().getMatrix() * glm::translate(glm::mat4(1.0f), velocity * currentFps));
            firework2.get().setMatrix(firework2.get().getMatrix() * glm::translate(glm::mat4(1.0f), velocity * currentFps));
            firework3.get().setMatrix(firework3.get().getMatrix() * glm::translate(glm::mat4(1.0f), velocity * currentFps));
        }
        auto stop = timer.now();
        std::chrono::duration<float> fs = stop - start;
        deltaTimeS = fs.count();
        auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(fs).count() / 1000.0f;
        float fps = (1.0f / deltaTime) * 1000.0f;
        seconds = seconds + deltaTime;
        currentFps = 1.0f / fps;
        if (seconds > 1000.0f) {
            fpsFactor = 1.0f / fps;
            seconds = 0.0f;
        }
    }
    std::cout << "Leaving main loop..." << std::endl;

    // Release engine:
    eng.free();

    // Done:
    std::cout << "[application terminated]" << std::endl;
    return 0;
}
