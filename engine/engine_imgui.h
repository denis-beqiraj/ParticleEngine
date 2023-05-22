
/**
 * @file		engine_pipeline_fullscreen2d.h
 * @brief	A pipeline for rendering a texture fullscreen in 2D
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once


 /**
  * @brief Fullscreen rendering 2D.
  */
class ENG_API ImGuiEngine : public Eng::Object
{
	//////////
public: //
	//////////

	   // Const/dest:
	ImGuiEngine(void* window);
	void newFrame();
	void newBar(std::string type,float& value,float min,float max);
	void newText(std::string text);
	void render();
};






