#ifndef ABSTRACTOBSERVER_H
#define ABSTRACTOBSERVER_H

#include <memory>
#include <chrono>

#include "chesstypes.h"
#include "GameConfiguration.h"

class AbstractGameObserver {
public:
	virtual ~AbstractGameObserver() { /* Nothing */ }

	virtual void onGameStart(State /*state*/, GameConfiguration /*config*/ ) { /* Nothing */ }
	virtual void onTurnStart(PlayerColor /*who*/ ) { /* Nothing */ }
	virtual void onTurnEnd(PlayerColor /*who*/, Turn /*turn*/, State /*newState*/) { /* Nothing */ }
	virtual void onTurnTimeout(PlayerColor /*who*/, std::chrono::seconds /*timeout*/ ) { /* Nothing */ }
	virtual void onGameOver(State /*state*/, PlayerColor /*winner*/) { /* Nothing */ }
};

using AbstractGameObserverPtr = std::shared_ptr<AbstractGameObserver>;

#endif // ABSTRACTOBSERVER_H
