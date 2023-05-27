#pragma once

 /**
  * @brief Class for particle emission.
  */
class ENG_API ParticleEmitter final : public Eng::Node
{
	//////////
	public: //
	//////////
	static ParticleEmitter empty;
	struct Particle {
		glm::vec4 initPosition, initVelocity,initAcceleration;
		glm::vec4 currentPosition, currentVelocity, currentAcceleration;
		glm::vec4 color;
		float initLife;
		float currentLife;
		float minLife;
		Particle() : initPosition(0.0f), initVelocity(0.0f), initAcceleration(1.0f), currentPosition(0.0f), currentVelocity(0.0f), currentAcceleration(1.0f), color(1.0f), initLife(0.0f), currentLife(0.0f) {}
	};
	// Const/dest:
	ParticleEmitter(std::shared_ptr<std::vector<Particle>> particles);
	ParticleEmitter(ParticleEmitter&& other);
	ParticleEmitter(ParticleEmitter const&) = delete;
	~ParticleEmitter();

	// Operators:
	void operator=(ParticleEmitter const&) = delete;
	// Rendering methods:   
	bool render(uint32_t value = 0, void* data = nullptr) const;

	void setTexture(const Eng::Bitmap& sprite);
	void setProjection(glm::mat4 projection);
	void setParticles(std::shared_ptr<std::vector<Particle>> particles);
	void setDt(float dT);

	///////////
	private: //
	///////////


	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;
};





