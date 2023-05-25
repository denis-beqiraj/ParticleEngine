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
   #include <imgui_impl_glfw.h>
   #include <imgui_impl_opengl3.h>
//////////   
// VARS //
//////////

   // Mouse status:
   double oldMouseX, oldMouseY;
   float rotX, rotY;
   bool mouseBR, mouseBL;
   float transZ = 50.0f;

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
   switch (button)
   {
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
   // ENG_LOG_DEBUG("key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);
   switch (key)
   {
      case 'W': if (action == 0) dfltPipe.setWireframe(!dfltPipe.isWireframe()); break;         
   }
}
std::shared_ptr<std::vector<Eng::ParticleEmitter::Particle>> particles;

void createParticles(int maxParticles) {
    particles->clear();
    for (int i = 0; i < maxParticles; i++) {
        Eng::ParticleEmitter::Particle particle;
        particle.initPosition = glm::vec3(0.0f);
        particle.initVelocity = glm::vec3(((float)rand() / RAND_MAX) * 8.0f - 4.0f, -((float)rand() / RAND_MAX)*2, ((float)rand() / RAND_MAX) * 8.0f - 4.0f);
        particle.initAcceleration = glm::vec3(0.0f, 2.8f, 0.0f);
        particle.initLife = ((float)rand() / RAND_MAX) * 10.0f;
        particle.minLife = -((float)rand() / RAND_MAX) * 50.0f;
        particles->push_back(particle);
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

   /////////////////
   // Loading scene:   
   Eng::Ovo ovo; 

   std::reference_wrapper<Eng::Node> root = ovo.load("simple3dSceneWithTransp.ovo");
   particles=std::make_shared<std::vector<Eng::ParticleEmitter::Particle>>();
   glm::mat4 pos(1.0f);
   pos = glm::translate(pos, glm::vec3(0.0f, 10.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));
   float value;
   value = 120;
   float currentValue;
   currentValue = value;
   createParticles(value);

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
   float fpsFactor = 0.0f;
   Eng::ParticleEmitter particleEmitter(particles);
   Eng::Bitmap sprite;
   sprite.load("grass.dds");
   particleEmitter.setTexture(sprite);
   particleEmitter.setMatrix(pos);
   particleEmitter.setProjection(camera.getProjMatrix());
   root.get().addChild(particleEmitter);
   Eng::ParticleEmitter::RenderData data;
   //computePipe.convert(particles);
   while (eng.processEvents())
   {      
      auto start = timer.now();

      // Update viewpoint:
      camera.setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, transZ)));
      root.get().setMatrix(glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(rotX), { 1.0f, 0.0f, 0.0f }), glm::radians(rotY), { 0.0f, 1.0f, 0.0f }));

      // Animate torus knot:      
      tknot.get().setMatrix(glm::rotate(tknot.get().getMatrix(), glm::radians(15.0f * fpsFactor), glm::vec3(0.0f, 1.0f, 0.0f)));
      
      // Update list:
      list.reset();
      list.process(root);
      
      // Main rendering:
      eng.clear();
         data.position = pos;
         data.dt = fpsFactor;
         //particleEmitter.render(0U,(void*)&data);
         dfltPipe.render(camera, list);
         //particlePipe.render(tknot.get().getMaterial().getTexture(), list);
         // Uncomment the following two lines for displaying the shadow map:
         // eng.clear();      
         // full2dPipe.render(dfltPipe.getShadowMappingPipeline().getShadowMap(), list);
         eng.getImgui()->newFrame();
         eng.getImgui()->newText("Fps: " + std::to_string(1.0f / fpsFactor));
         eng.getImgui()->newBar("Number particles", value, 1.0f, 200000.0f);
         if (currentValue != value) {
             createParticles(value);
             particleEmitter.setParticles(particles);
         }
         currentValue = value;
         eng.getImgui()->render();
      eng.swap();    

      auto stop = timer.now();
      auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000.0f;
      float fps = (1.0f / deltaTime) * 1000.0f;
      fpsFactor = 1.0f / fps;
   }
   std::cout << "Leaving main loop..." << std::endl;

   // Release engine:
   eng.free();

   // Done:
   std::cout << "[application terminated]" << std::endl;
   return 0;
}
