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
	struct RenderData {
		float dt;
		glm::mat4 position;
	};
	// Const/dest:
	ParticleEmitter(unsigned int maxParticles, unsigned int newParticlesPerFrame);
	ParticleEmitter(ParticleEmitter&& other);
	ParticleEmitter(ParticleEmitter const&) = delete;
	~ParticleEmitter();

	// Operators:
	void operator=(ParticleEmitter const&) = delete;

	// Rendering methods:   
	bool render(uint32_t value = 0, void* data = nullptr) const;

	///////////
	private: //
	///////////

	struct Particle {
		glm::vec3 position, velocity;
		glm::vec4 color;
		float life;

		Particle() : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) {}
	};

	struct ParticleArrayNode {
		bool isFree;

		union {
			Particle particle;
			ParticleArrayNode* nextFree;
		};
	};

	Particle* getFreeParticle() const;
	void respawnParticle(Particle* particle, const glm::vec3& position) const;

	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;
};





