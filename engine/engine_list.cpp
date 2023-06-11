/**
 * @file		engine_list.cpp
 * @brief	List of objects after the scenegraph traversal
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"
   #include <algorithm>
   #include "GLFW/glfw3.h"


////////////
// STATIC //
////////////

   // Special values:
   Eng::List Eng::List::empty("[empty]");   



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief List reserved structure.
 */
struct Eng::List::Reserved
{    
   std::vector<Eng::List::RenderableElem> renderableElem;   ///< List of rendering elements
   uint32_t nrOfLights;                                     ///< Number of lights in the list (lights come first)
   uint32_t nrOfOpaqueMeshes;                               ///< Number of opaque meshes in the list
   uint32_t nrOfTransparentMeshes;                          ///< Number of transparent meshes in the list

   /**
    * Constructor. 
    */
   Reserved() : nrOfLights{ 0 }, nrOfOpaqueMeshes{ 0 }, nrOfTransparentMeshes{ 0 }
   {}
};



////////////////////////
// BODY OF CLASS List //
////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::List::List() : reserved(std::make_unique<Eng::List::Reserved>())
{		
   ENG_LOG_DETAIL("[+]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::List::List(const std::string &name) : Eng::Object(name),  reserved(std::make_unique<Eng::List::Reserved>())
{	   
   ENG_LOG_DETAIL("[+]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::List::List(List &&other) : Eng::Object(std::move(other)), reserved(std::move(other.reserved))
{  
   ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::List::~List()
{
   ENG_LOG_DETAIL("[-]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Reset internal list.
 */
void ENG_API Eng::List::reset()
{	
   reserved->renderableElem.clear();
   reserved->nrOfLights = 0;
   reserved->nrOfOpaqueMeshes = 0;
   reserved->nrOfTransparentMeshes = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the number of currently loaded renderable elements. 
 * @return number of loaded renderable elements
 */
uint32_t ENG_API Eng::List::getNrOfRenderableElems() const
{	
   return static_cast<uint32_t>(reserved->renderableElem.size());
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the number of lights currently loaded in the list.
 * @return number of loaded lights
 */
uint32_t ENG_API Eng::List::getNrOfLights() const
{	
   return reserved->nrOfLights;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets internal list of renderable elements.
 * @return list of renderable elements
 */
const std::vector<Eng::List::RenderableElem> ENG_API &Eng::List::getRenderableElems() const
{
   return reserved->renderableElem;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets a reference to the specified element in the list. 
 * @return element at the given position
 */
const Eng::List::RenderableElem ENG_API &Eng::List::getRenderableElem(uint32_t elemNr) const
{   
   return reserved->renderableElem.at(elemNr);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Recursively parses the scenegraph starting at the given node and append the parsed elements to this list. 
 * @param node starting node
 * @param prevMatrix previous node matrix
 * @return TF
 */
bool ENG_API Eng::List::process(const Eng::Node &node, const glm::mat4 &prevMatrix)
{
   // Safety net:
   if (node == Eng::Node::empty)
   {
      ENG_LOG_ERROR("Invalid params");
      return false;
   }
   
   RenderableElem re;
   re.matrix = prevMatrix * node.getMatrix();
   re.reference = node;   
   
   // Store only renderable elements:
   if (dynamic_cast<const Eng::Light *>(&node)) // Lights first
   {
      reserved->renderableElem.insert(reserved->renderableElem.begin(), 1, re);
      reserved->nrOfLights++;
   }
   else if (dynamic_cast<const Eng::ParticleEmitter *>(&node))
   {
      reserved->renderableElem.push_back(re);
   }
   else if (dynamic_cast<const Eng::Mesh*>(&node)) // Only meshes
   {
       if (dynamic_cast<const Eng::Mesh*>(&node)->getMaterial().getOpacity() >= 1.0f && !dynamic_cast<const Eng::Mesh*>(&node)->getMaterial().getTexture().getTrasparent()) 
       {
            reserved->renderableElem.insert(reserved->renderableElem.begin() + reserved->nrOfLights, 1, re);
            reserved->nrOfOpaqueMeshes++;
       }
       else 
       {
            reserved->renderableElem.insert(reserved->renderableElem.begin() + reserved->nrOfLights + reserved->nrOfOpaqueMeshes, 1, re);
            reserved->nrOfTransparentMeshes++;
       }
   }

   // Parse hierarchy recursively:
   for (auto &n : node.getListOfChildren())
      if (process(n, re.matrix) == false)
         return false;

	// Done:
   return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Parses the list and call the render method of each renderable.
 * @param cameraMatrix camera (also view) matrix (must be already inverted) 
 * @param pass type of pass
 * @return number of loaded renderable elements
 */
bool ENG_API Eng::List::render(const glm::mat4 &cameraMatrix, Eng::List::Pass pass) const
{	
   // Define range:
   size_t startRange = 0;
   size_t endRange = reserved->renderableElem.size();
   bool isTrasparent = false;
   bool isParticle = false;
   switch (pass)
   {
      //////////////////
      case Pass::all: //
         break;
      
      /////////////////////
      case Pass::lights: //  
         endRange = reserved->nrOfLights;
         break;

      /////////////////////
      case Pass::meshes: //
         startRange = reserved->nrOfLights;
         endRange = startRange + reserved->nrOfOpaqueMeshes;
         break;

      /////////////////////////
      case Pass::trasparent: //
          startRange = reserved->nrOfLights + reserved->nrOfOpaqueMeshes;
          endRange = startRange + reserved->nrOfTransparentMeshes;
          std::sort(reserved->renderableElem.begin() + startRange, reserved->renderableElem.begin() + endRange, [cameraMatrix](RenderableElem a, RenderableElem b) {
              return glm::length(a.matrix[3] - glm::inverse(cameraMatrix)[3]) > glm::length(b.matrix[3] - glm::inverse(cameraMatrix)[3]);
              });
          isTrasparent = true;
          break;

      /////////////////////////
      case Pass::allmeshes: //
          startRange = reserved->nrOfLights;
          endRange = startRange + reserved->nrOfOpaqueMeshes + reserved->nrOfTransparentMeshes;
          break;

      case Pass::particleemitters:
          startRange = reserved->nrOfLights + reserved->nrOfOpaqueMeshes + reserved->nrOfTransparentMeshes;
          std::sort(reserved->renderableElem.begin() + startRange, reserved->renderableElem.begin() + endRange, [cameraMatrix](RenderableElem a, RenderableElem b) {
              return glm::length(a.matrix[3] - glm::inverse(cameraMatrix)[3]) > glm::length(b.matrix[3] - glm::inverse(cameraMatrix)[3]);
              });
          isParticle = true;
          break;
   }
   if (isTrasparent) {
       glDepthMask(false);
   }
   if (isParticle) {
       // Iterate through the range:
       for (size_t c = startRange; c < endRange; c++)
       {
           RenderableElem& re = reserved->renderableElem.at(c);
           Eng::ParticleEmitter::ParticleModelView modelView;
           modelView.model = re.matrix;
           modelView.view = cameraMatrix;
           //glm::mat4 finalMatrix = cameraMatrix * re.matrix;
           re.reference.get().render(0, &modelView);
       }
   }
   else {
       for (size_t c = startRange; c < endRange; c++)
       {
           RenderableElem& re = reserved->renderableElem.at(c);
           glm::mat4 modelViewMat = cameraMatrix * re.matrix;
           re.reference.get().render(0, &modelViewMat);
       }
   }
   if (isTrasparent) {
       glDepthMask(true);
   }
   // Done:
   return true;
}
