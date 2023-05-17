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

	// Const/dest:
	ParticleEmitter();
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

	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;
};





