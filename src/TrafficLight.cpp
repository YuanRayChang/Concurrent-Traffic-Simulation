#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <future>
/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
	// The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
	// to wait for and receive new messages and pull them from the queue using move semantics. 
	// The received object should then be returned by the receive function. 
	std::unique_lock<std::mutex> uLock(_mtx);
	_cv.wait(uLock, [this] { return !_queue.empty(); });

	T msg = std::move(_queue.back());
	_queue.clear();

	return msg; // will not be copied due to return value optimization (RVO) in C++
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
	// The method send should use the mechanisms std::lock_guard<std::mutex> 
	// as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
	std::lock_guard<std::mutex> lock(_mtx);
	_queue.push_back(std::move(msg));
	_cv.notify_one();
}


/* Implementation of class "TrafficLight" */
TrafficLight::TrafficLight()
{
	_currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
	// Add the implementation of the method waitForGreen, in which an infinite while-loop 
	// runs and repeatedly calls the receive function on the message queue. 
	// Once it receives TrafficLightPhase::green, the method returns.
	TrafficLightPhase phase;
	while(true) {
		phase = _queue_phase.receive();
		if(phase == TrafficLightPhase::green) {
			return;
		}
	}
}

void TrafficLight::simulate()
{
	// Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
	return _currentPhase;
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
	// Implement the function with an infinite loop that measures the time between two loop cycles 
	// and toggles the current phase of the traffic light between red and green and sends an update method 
	// to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
	// Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<int> distribution(4,6);
	while(true) {
		int duration = distribution(generator); 
		std::this_thread::sleep_for(std::chrono::seconds(duration));
		if(_currentPhase == TrafficLightPhase::red) {
			_currentPhase = TrafficLightPhase::green;
		} 
		else {
			_currentPhase = TrafficLightPhase::red;
		}

		// Sends an update method to the message queue using move semantics
		_queue_phase.send(std::move(_currentPhase));
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

}