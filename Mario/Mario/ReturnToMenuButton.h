#pragma once

#include "Actor.h"
#include "ButtonComponent.h"
#include "TextSpriteComponent.h"

class ReturnToMenuButton : public Actor, public ButtonComponent
{
public:
	ReturnToMenuButton(class Game* game, Vector2 position);

	void OnMouseButtonDown() override;
};

