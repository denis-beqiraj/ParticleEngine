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
   // ENG_LOG_DEBUG("x: %.1f, y: %.1f", mouseX, mouseY);
   float deltaY = (float) (mouseX - oldMouseX);
   oldMouseX = mouseX;
   if (mouseBR)
      rotY += deltaY;

   float deltaX = (float) (mouseY - oldMouseY);
   oldMouseY = mouseY;
   if (mouseBR)
      rotX += deltaX;
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
      case 0: mouseBL = (bool) action; break;
      case 1: mouseBR = (bool) action; break;
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
   transZ -= (float) scrollY;
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
   //ENG_LOG_DEBUG("key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);
   if (cameraMode == CameraMode_Default) {
      switch (key) {
         case 'C': if (action == 0) cameraMode = CameraMode_FirstPerson; break;
         case 'W': if (action == 0) dfltPipe.setWireframe(!dfltPipe.isWireframe()); break;         
      }
   } else if (cameraMode == CameraMode_FirstPerson) {
      switch (key) {
         case 'C': if (action == 0) cameraMode = CameraMode_Default; break;
         case 'A': {
               if (action == 0) {
                  firstPersonDesiredRotVelocity = 0.0f;
               } else {
                  firstPersonDesiredRotVelocity = 5.0f;
               }
         } break;
         case 'D': {
               if (action == 0) {
                  firstPersonDesiredRotVelocity = 0.0f;
               } else {
                  firstPersonDesiredRotVelocity = -5.0f;
               }
         } break;
         case 'W': {
            if (action == 0) {
               firstPersonDesiredVelocity = 0.0f;
            } else {
               firstPersonDesiredVelocity = -120.0f;
            }
         } break;
         case 'S': {
            if (action == 0) {
               firstPersonDesiredVelocity = 0.0f;
            } else {
               firstPersonDesiredVelocity = 120.0f;
            }
         } break;
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
        particle.initAcceleration = glm::vec4(startAcceleration,0.0f);
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

float lerp(float a, float b, float t) {
   t = glm::clamp<float>(t, 0.0f, 1.0f);
   return a * (1.0f - t) + b * t;
}

void updateParticles(std::vector<Eng::ParticleEmitter::Particle>& particles) {
    for (int i = 0; i < particles.size(); i++) {
        particles.at(i).initAcceleration = glm::vec4(startAcceleration, 0.0f);
        particles.at(i).currentPosition = particles.at(i).initPosition;
        particles.at(i).currentVelocity = particles.at(i).initVelocity;
        particles.at(i).currentAcceleration = particles.at(i).initAcceleration;
        particles.at(i).initLife = ((float)rand() / RAND_MAX) * initLife.x;
        particles.at(i).currentLife = particles.at(i).initLife;
        particles.at(i).minLife = ((float)rand() / RAND_MAX) * initLife.y;
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
int main(int argc, char *argv[])
{
   // Credits:
   std::cout << "Engine demo, A. Peternier (C) SUPSI" << std::endl;
   std::cout << std::endl;

   // Init engine:
   Eng::Base &eng = Eng::Base::getInstance();
   eng.init();

   // Register callbacks:
   eng.setMouseCursorCallback(mouseCursorCallback);
   eng.setMouseButtonCallback(mouseButtonCallback);
   eng.setMouseScrollCallback(mouseScrollCallback);
   eng.setKeyboardCallback(keyboardCallback);
   eng.initImgui();

   cameraMode = CameraMode_Default;
   firstPersonPosition = glm::vec3(0.0f, 17.5f, 0.0f);
   firstPersonVelocity = 0.0f;
   firstPersonDesiredVelocity = 0.0f;
   firstPersonRot = 0.0f;
   firstPersonRotVelocity = 0.0f;
   firstPersonDesiredRotVelocity = 0.0f;

   /////////////////
   // Loading scene:   
   Eng::Ovo ovo; 

   std::reference_wrapper<Eng::Node> root = ovo.load("demo1.ovo");
   std::vector<Eng::ParticleEmitter::Particle> particles;
   std::vector<Eng::ParticleEmitter::Particle> fireParticles;
   glm::mat4 pos(1.0f);
   pos = glm::translate(pos, glm::vec3(0.0f, 10.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
   float value;
   value = 120;
   startVelocity = glm::vec3(8, -2, 8);
   startAcceleration = glm::vec3(0, 2.8, 0);
   color = glm::vec3(1,0,0);
   initLife = glm::vec2(2, -5);
   createParticlesSmoke(particles, value, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
   createParticlesFire(fireParticles, value, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 0.0f));

   std::cout << "Scene graph:\n" << root.get().getTreeAsString() << std::endl;
   
   // Get light ref:
   std::reference_wrapper<Eng::Light> light = dynamic_cast<Eng::Light &>(Eng::Container::getInstance().find("Omni001"));      
   light.get().setAmbient({ 0.3f, 0.3f, 0.3f });
   light.get().setColor({ 1.5f, 1.5f, 1.5f });
   light.get().setProjMatrix(glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 1000.0f)); // Orthographic projection
   // light.get().setProjMatrix(glm::perspective(glm::radians(75.0f), 1.0f, 1.0f, 1000.0f)); // Perspective projection         
   // Get torus knot ref:
   std::reference_wrapper<Eng::Mesh> tknot = dynamic_cast<Eng::Mesh &>(Eng::Container::getInstance().find("Torus Knot001"));

   // Rendering elements:
   Eng::List list;
   Eng::Camera camera;
   camera.setProjMatrix(glm::perspective(glm::radians(45.0f), eng.getWindowSize().x / (float) eng.getWindowSize().y, 1.0f, 1000.0f));      
   
   /////////////
   // Main loop:
   std::cout << "Entering main loop..." << std::endl;      
   std::chrono::high_resolution_clock timer;
   float fpsFactor = 1.0f;
   float currentFps = 0.0f;
   float bounciness = 0.8f;
   Eng::ParticleEmitter particleEmitter(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particles));
   Eng::Bitmap sprite;
   sprite.load("smoke.dds");
   particleEmitter.setTexture(sprite);
   particleEmitter.setMatrix(pos);
   particleEmitter.setProjection(camera.getProjMatrix());
   root.get().addChild(particleEmitter);
   //computePipe.convert(particles);

   // fire
   Eng::ParticleEmitter fireParticleEmitter(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(fireParticles));
   sprite.load("flame.dds");
   fireParticleEmitter.setTexture(sprite);
   fireParticleEmitter.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 7.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)));
   fireParticleEmitter.setProjection(camera.getProjMatrix());
   root.get().addChild(fireParticleEmitter);

   float seconds = 0.0f;
   float deltaTimeS = 0.0f;
   while (eng.processEvents())
   {
      auto start = timer.now();

      // Update viewpoint:
      if (cameraMode == CameraMode_Default) {
         camera.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, transZ)));
         root.get().setMatrix(glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(rotX), { 1.0f, 0.0f, 0.0f }), glm::radians(rotY), { 0.0f, 1.0f, 0.0f }));
      } else if (cameraMode == CameraMode_FirstPerson) {
         // Update rotation according to desired velocity
         firstPersonRotVelocity = lerp(firstPersonRotVelocity, firstPersonDesiredRotVelocity, deltaTimeS * firstPersonVelocityTransitionSpeed);
         firstPersonRot += firstPersonRotVelocity * deltaTimeS;

         // Update position according to desired velocity
         glm::vec3 cameraFront = glm::rotate(glm::quat(glm::vec3(0.0f, firstPersonRot, 0.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
         firstPersonVelocity = lerp(firstPersonVelocity, firstPersonDesiredVelocity, deltaTimeS * firstPersonVelocityTransitionSpeed);
         firstPersonPosition += cameraFront * firstPersonVelocity * deltaTimeS;

         // Calculate camera matrix
         glm::mat4 cameraMat = glm::translate(glm::mat4(1.0f), firstPersonPosition) * glm::rotate(glm::mat4(1.0f), firstPersonRot, glm::vec3(0.0f, 1.0f, 0.0f));
         camera.setMatrix(cameraMat);

         root.get().setMatrix(glm::mat4(1.0f));
      }

      // Animate torus knot:      
      tknot.get().setMatrix(glm::rotate(tknot.get().getMatrix(), glm::radians(15.0f * fpsFactor), glm::vec3(0.0f, 1.0f, 0.0f)));
      // Update list:
      list.reset();
      list.process(root);

      // Main rendering:
      eng.clear();
         //particleEmitter.render(0U,(void*)&data);
         dfltPipe.render(camera, list);
         particleEmitter.setDt(currentFps);
         particleEmitter.setPlaneMinimum(-5.0f);

         fireParticleEmitter.setDt(currentFps);
         fireParticleEmitter.setPlaneMinimum(-5.0f);
         //particlePipe.render(tknot.get().getMaterial().getTexture(), list);
         // Uncomment the following two lines for displaying the shadow map:
         // eng.clear();      
         //  full2dPipe.render(dfltPipe.getShadowMappingPipeline().getShadowMap(), list);
         eng.getImgui()->newFrame();
         eng.getImgui()->newText("Fps: " + std::to_string(1.0f / fpsFactor));
         if (eng.getImgui()->newBar("Number particles", value, 1.0f, 200000.0f)) {
             createParticlesSmoke(particles, value, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
             particleEmitter.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particles));
         }
         eng.getImgui()->newText("Start velocity");
         if (eng.getImgui()->newBar("XV", startVelocity.x, -100.0f, 100.0f) | eng.getImgui()->newBar("YV", startVelocity.y, -100.0f, 100.0f) | eng.getImgui()->newBar("ZV", startVelocity.z, -100.0f, 100.0f)) {
             updateParticles(particles);
             particleEmitter.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particles));
         }
         eng.getImgui()->newText("Start acceleration");
         if (eng.getImgui()->newBar("XA", startAcceleration.x, -100.0f, 100.0f) | eng.getImgui()->newBar("YA", startAcceleration.y, -100.0f, 100.0f) | eng.getImgui()->newBar("ZA", startAcceleration.z, -100.0f, 100.0f)) {
             updateParticles(particles);
             particleEmitter.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particles));
         }
         eng.getImgui()->newText("Life");
         if (eng.getImgui()->newBar("Init life", initLife.x, -100.0f, 100.0f) | eng.getImgui()->newBar("End life", initLife.y, -100.0f, 100.0f)) {
             updateParticles(particles);
             particleEmitter.setParticles(std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>(particles));
         }
         eng.getImgui()->newBar("Bounciness", bounciness, 0.0f, 1.0f);
         particleEmitter.setBounciness(bounciness);
         eng.getImgui()->render();
      eng.swap();

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
