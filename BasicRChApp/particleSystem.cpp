#include "particleSystem.h"

#include "dxDevice.h"
#include "exceptions.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

const D3D11_INPUT_ELEMENT_DESC ParticleVertex::Layout[3] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"POSITION", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0} };

const XMFLOAT3 ParticleSystem::EMITTER_DIR = XMFLOAT3(0.0f, 1.0f, 0.0f);
const XMFLOAT3 ParticleSystem::GRAVITY = XMFLOAT3(0.0f, -5.0f, 0.0f);
const float ParticleSystem::TIME_TO_LIVE = 1.2f;
const float ParticleSystem::EMISSION_RATE = 200.0f;
const float ParticleSystem::MAX_ANGLE = XM_PIDIV2 / 9.0f;
const float ParticleSystem::MIN_VELOCITY = 0.2f;
const float ParticleSystem::MAX_VELOCITY = 0.33f;
const float ParticleSystem::PARTICLE_SIZE = 0.08f;
const float ParticleSystem::PARTICLE_SCALE = 1.0f;
const float ParticleSystem::MIN_ANGLE_VEL = -XM_PI;
const float ParticleSystem::MAX_ANGLE_VEL = XM_PI;
const int ParticleSystem::MAX_PARTICLES = 500;

ParticleSystem::ParticleSystem(DirectX::XMFLOAT3 emmiterPosition, DirectX::XMFLOAT3 emmiterDir)
    : m_emitterPos(emmiterPosition),
	  m_emitterDir(emmiterDir),
      m_particlesToCreate(0.0f),
      m_random(random_device{}()) {}

vector<ParticleVertex> ParticleSystem::Update(
	float dt, DirectX::XMFLOAT4 cameraPosition)
{
	size_t removeCount = 0;
	for (auto& p : m_particles) {
		UpdateParticle(p, dt);
		if (p.Vertex.Age >= TIME_TO_LIVE) ++removeCount;
	}
	m_particles.erase(m_particles.begin(), m_particles.begin() + removeCount);

	m_particlesToCreate += dt * EMISSION_RATE;
	while (m_particlesToCreate >= 1.0f) {
		--m_particlesToCreate;
		if (m_particles.size() < MAX_PARTICLES)
			m_particles.push_back(RandomParticle());
	}
	return GetParticleVerts(cameraPosition);
}

XMFLOAT3 ParticleSystem::RandomVelocity()
{
	static const uniform_real_distribution<float> upSpeed(-3.0, 3.0);
	static const uniform_real_distribution<float> sideSpeed(-2.0, 2.0);
	XMFLOAT3 v = m_emitterDir;
	v.y += upSpeed(m_random);
	v.x += sideSpeed(m_random);
	v.z += sideSpeed(m_random);
	return v;
}

Particle ParticleSystem::RandomParticle()
{
	Particle p; 

	p.Vertex.Pos = m_emitterPos;
	p.Vertex.LastPos = p.Vertex.Pos;
	p.Velocity = RandomVelocity();

	return p;
}

void ParticleSystem::UpdateParticle(Particle& p, float dt)
{
	p.Vertex.Age += dt;
	p.Vertex.LastPos = p.Vertex.Pos;
	p.Velocity.x += GRAVITY.x * dt;
	p.Velocity.y += GRAVITY.y * dt;
	p.Velocity.z += GRAVITY.z * dt;
	p.Vertex.Pos.x += p.Velocity.x * dt;
	p.Vertex.Pos.y += p.Velocity.y * dt;
	p.Vertex.Pos.z += p.Velocity.z * dt;
}

vector<ParticleVertex> ParticleSystem::GetParticleVerts(
	DirectX::XMFLOAT4 cameraPosition)
{
	XMFLOAT4 cameraTarget(0.0f, 0.0f, 0.0f, 1.0f);

	//vector<ParticleVertex> vertices;
	// TODO : 1.29 Copy particles' vertex data to a vector and sort them
	vector<ParticleVertex> vertices;
	for (auto pVer : m_particles) {
		vertices.push_back(pVer.Vertex);
	}
	std::sort(vertices.begin(), vertices.end(),
		[cameraPosition](ParticleVertex a, ParticleVertex b)
		{
			XMFLOAT3 posA = a.Pos;
			XMFLOAT3 posB = b.Pos;
			float distA =
				sqrt((posA.x - cameraPosition.x) * (posA.x - cameraPosition.x) +
					(posA.y - cameraPosition.y) * (posA.y - cameraPosition.y) +
					(posA.z - cameraPosition.z) * (posA.z - cameraPosition.z));
			float distB =
				sqrt((posB.x - cameraPosition.x) * (posB.x - cameraPosition.x) +
					(posB.y - cameraPosition.y) * (posB.y - cameraPosition.y) +
					(posB.z - cameraPosition.z) * (posB.z - cameraPosition.z));
			return distA > distB;
		});

	return vertices;
}
