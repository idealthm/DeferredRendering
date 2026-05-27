#include "Scene.h"

#include "Lights/Light.h"


Scene::Scene()
{
}

const std::set<std::shared_ptr<Actor>>& Scene::GetActors() const
{
	return Actors;
}
