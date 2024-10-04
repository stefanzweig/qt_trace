#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <QDebug>

enum class State {
	INIT,
	START,
	PAUSE,
	STOPPED,
	RESUMED,
	COMPLETE,
};

class StateManager {
public:
	StateManager() : currentState(State::INIT) {}

	void changeState(State newState) {
		currentState = newState;
		recordState(newState);
	}

	void recordState(State state) {
		std::string stateString;
		switch (state) {
			case State::INIT: stateString = "INIT"; break;
			case State::START: stateString = "START"; break;
			case State::PAUSE: stateString = "PAUSE"; break;
			case State::STOPPED: stateString = "STOPPED"; break;
			case State::RESUMED: stateString = "RESUMED"; break;
			case State::COMPLETE: stateString = "COMPLETE"; break;
		}
		statesHistory.push_back(stateString);
		qDebug() << "State changed to: " << QString::fromStdString(stateString);
	}

	void printHistory() const {
		qDebug() << "State History:";
		for (const auto& state : statesHistory) {
			qDebug() << QString::fromStdString(state);
		}
	}

	void printCurrentState() {
		recordState(currentState);
	}

	QString previous_state()
	{
		if (statesHistory.size() >= 2) {
			std::string secondToLast = statesHistory[statesHistory.size() - 2];
			return QString::fromStdString(secondToLast);
		}
		else {
			qDebug() << "vector 中没有足够的元素。";
			return "";
		}
	}

private:
	State currentState;
	std::vector<std::string> statesHistory;
};


/* Usage
int main() {
	StateManager manager;

	manager.changeState(State::PROCESSING);
	manager.changeState(State::COMPLETED);
	manager.changeState(State::ERROR);

	manager.printHistory();

	return 0;
}*/