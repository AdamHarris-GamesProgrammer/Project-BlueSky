#include "Actor.h"
#include "Game.h"
#include "Component.h"

#include <algorithm>

Actor::Actor(class Game* game) : mState(EActive), mPosition(Vector2::Zero), mScale(1.0f), mRotation(0.0f), mGame(game)
{
	mGame->GetEngine()->AddActor(this);
}

Actor::~Actor()
{
	mGame->GetEngine()->RemoveActor(this);

	//need to delete components
	while (!mComponents.empty()) {
		delete mComponents.back();
	}
}

void Actor::Update(float deltaTime)
{
	if (mState == EActive) {
		UpdateComponents(deltaTime);
		UpdateActor(deltaTime);
	}
}

void Actor::UpdateComponents(float deltaTime)
{
	for (auto comp : mComponents) {
		comp->Update(deltaTime);
	}
}

//virtual function
void Actor::UpdateActor(float deltaTime)
{
}

void Actor::AddComponent(class Component* component)
{
	int myOrder = component->GetUpdateOrder();
	auto iter = mComponents.begin();
	for (; iter != mComponents.end(); ++iter) {
		if (myOrder < (*iter)->GetUpdateOrder()) {
			break;
		}
	}
	//inserts component before the position of the iterator
	mComponents.insert(iter, component);
}

void Actor::RemoveComponent(class Component* component)
{
	//searches mComponent vector for the component and deletes it
	auto iter = std::find(mComponents.begin(), mComponents.end(), component);
	if (iter != mComponents.end()) {
		mComponents.erase(iter);
	}
}
