#include <iostream>

#include "ConsoleObserver.h"
#include "helper.h"
#include <thread>

using namespace std;

void ConsoleObserver::onGameStart(State state, GameConfiguration config) {
	cout << "ConsoleObserver (" << this_thread::get_id() << "): Game started" << endl;
	cout << config << endl;
	cout << endl << state << endl << endl;
}

void ConsoleObserver::onTurnStart(PlayerColor who) {
	cout << "ConsoleObserver (" << this_thread::get_id() << "): " << who << " should now perform his turn" << endl;
}

void ConsoleObserver::onTurnEnd(PlayerColor who, Turn turn, State newState) {
	cout << "ConsoleObserver (" << this_thread::get_id() << "): " << who << " performed: " << turn << endl;
	cout << endl << newState << endl << endl;
}

void ConsoleObserver::onTurnTimeout(PlayerColor who, std::chrono::seconds timeout) {
	cout << "ConsoleObserver (" << this_thread::get_id() << "): " << who << " timed out after " << timeout.count() << " seconds" << endl;
}

void ConsoleObserver::onGameOver(State state, PlayerColor winner) {
	cout << "ConsoleObserver (" << this_thread::get_id() << "): Game Over. Winner: " << winner << endl;
	cout << endl << state << endl << endl;
}

