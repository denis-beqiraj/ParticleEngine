
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
class ENG_API PipelineCompute : public Eng::Pipeline
{
	//////////
public: //
	//////////
	ENG_ALIGNED_TYPE(struct, 16) ComputeParticle
	{
		glm::vec4 position;
		glm::vec4 color;
	};
	   // Const/dest:
	PipelineCompute();
	PipelineCompute(PipelineCompute&& other);
	PipelineCompute(PipelineCompute const&) = delete;
	virtual ~PipelineCompute();
	void setModel(glm::mat4 model);
	// Rendering methods:
	// bool render(uint32_t value = 0, void *data = nullptr) const = delete;
	bool convert(std::vector<Eng::ParticleEmitter::Particle> particles);
	bool render();

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
	PipelineCompute(const std::string& name);
};






