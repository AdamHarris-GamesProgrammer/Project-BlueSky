#pragma once
#include "Actor.h"
#include "ButtonComponent.h"
class RetryButton : public Actor, public ButtonComponent
{
public:
	RetryButton(class Game* game, Vector2 position);

	void OnMouseButtonDown() override;
};

