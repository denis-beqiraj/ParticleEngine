
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
class ENG_API PipelineParticle : public Eng::Pipeline
{
	//////////
public: //
	//////////

	   // Const/dest:
	PipelineParticle();
	PipelineParticle(PipelineParticle&& other);
	PipelineParticle(PipelineParticle const&) = delete;
	virtual ~PipelineParticle();
	void setModel(glm::mat4 model);
	// Rendering methods:
	// bool render(uint32_t value = 0, void *data = nullptr) const = delete;
	bool render(const Eng::Texture& texture);

	// Managed:
	bool init() override;
	bool free() override;


	/////////////
protected: //
	/////////////

	   // Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;

	// Const/dest:
	PipelineParticle(const std::string& name);
};






