#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#define DEFAULT_WHEEL_RADIUS (float)0.05
#define DEFAULT_WHEELBASE ((float)0.1)*2

#define TIME_MILI ((float)0.001)

class Wheel {
	private:
		float wheelRadius;
		float angularVel;
		float tangentialVel;

	public:
		Wheel(float _wheelRadius) {
			this->wheelRadius = _wheelRadius;
			this->angularVel = 0;
			this->tangentialVel = 0;
		}

		void setAngularVel(float _angularVel) {
			this->angularVel = _angularVel;
			tangentialVel = _angularVel * wheelRadius;
		}

		float getTangentialVel() {
			tangentialVel = this->angularVel * wheelRadius;
			return this->tangentialVel;
		}
};

class Vehicle {
	public:
		float wheelbase;
		float angularVel;
		float tangentialVel;

		float angle;
		float x;
		float y;

	public:
		Wheel lWheel = Wheel(DEFAULT_WHEEL_RADIUS);
		Wheel rWheel = Wheel(DEFAULT_WHEEL_RADIUS);

		Vehicle(float _wheelbase) {
			this->wheelbase = _wheelbase;
			this->angularVel = 0;
			this->tangentialVel = 0;

			this->angle = 0;
			this->x = 0;
			this->y= 0;
		}

		void calculateAngularVel() {
			this->angularVel = (lWheel.getTangentialVel() - rWheel.getTangentialVel()) / this->wheelbase;
		}

		void calculateTangentialVel() {
			this->tangentialVel = (lWheel.getTangentialVel() + rWheel.getTangentialVel()) / (float)2.0;
		}

		void calculateXY(double deltaTime) {
			this->calculateAngularVel();
			double deltaAngle = this->angularVel * deltaTime;
			this->angle = this->angle + deltaAngle;
			this->calculateTangentialVel();
			double vX = this->tangentialVel * cos(this->angle);
			double vY = this->tangentialVel * sin(this->angle);
			double deltaX = vX * deltaTime;
			double deltaY = vY * deltaTime;

			this->x += deltaX;
			this->y += deltaY;
		}

		void printData(double timeDelta, double time) {
			printf("%f | %3.4f | %f | %f \n", timeDelta, time, this->x, this->y);
		}
};

int main() {
	//Declare and create a new render-window
	sf::RenderWindow window(sf::VideoMode(1280,720), "SFML window");
	window.display();

	// Event processing
	sf::Event event;

	// Count timer
	auto absolute_start_time = std::chrono::high_resolution_clock::now(); // get current time
	auto start_time = std::chrono::high_resolution_clock::now(); // get current time

	// Model logic
	Vehicle vehicle(DEFAULT_WHEELBASE);
	vehicle.lWheel.setAngularVel(200);
	vehicle.rWheel.setAngularVel(200);

	double i = 0;

	// The main loop - ends as soon as the window is closed
	while (window.isOpen())
	{
		auto end_time = std::chrono::high_resolution_clock::now(); // get current time again
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time); // calculate time difference
		double absolute_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - absolute_start_time).count() * TIME_MILI; // calculate time difference

		// Handle Events
		while (window.pollEvent(event))
		{
			// Request for closing the window
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed) {
				// Handle key press event

				switch (event.key.code)
				{
					case sf::Keyboard::Key::W:
						printf("W");
						break;
					case sf::Keyboard::Key::A:
						printf("A");
						break;
					case sf::Keyboard::Key::S:
						printf("S");
						break;
					case sf::Keyboard::Key::D:
						printf("D");
						break;
					case sf::Keyboard::Key::Q:
						printf("Q");
						break;
					case sf::Keyboard::Key::E:
						printf("E");
						break;
					default:
						break;
				}
			}
		}

		if (absolute_duration >= 0) {
			vehicle.lWheel.setAngularVel(2);
		 	vehicle.rWheel.setAngularVel(2);
		}
		if (absolute_duration >= 5) {
			vehicle.lWheel.setAngularVel(-1);
			vehicle.rWheel.setAngularVel(1);
		}
		if (absolute_duration >= 10) {
			vehicle.lWheel.setAngularVel(0);
			vehicle.rWheel.setAngularVel(0);
		}
		if (absolute_duration >= 15) {
			vehicle.lWheel.setAngularVel(2);
			vehicle.rWheel.setAngularVel(-2);
		}
		if (absolute_duration >= 20) {
			vehicle.lWheel.setAngularVel(1);
			vehicle.rWheel.setAngularVel(1);
		}
		if (absolute_duration >= 25) {
			vehicle.lWheel.setAngularVel(2);
			vehicle.rWheel.setAngularVel(0);
		}
		if (absolute_duration >= 35) {
			vehicle.lWheel.setAngularVel(0);
			vehicle.rWheel.setAngularVel(-2);
		}
		if (absolute_duration >= 50) {
			vehicle.lWheel.setAngularVel(10000);
			vehicle.rWheel.setAngularVel(0);
		}

		// Calculate data
		if (duration.count() >= 10) { // check if 10ms has elapsed
			vehicle.calculateXY(duration.count() * TIME_MILI);
			vehicle.printData(duration.count() * TIME_MILI, absolute_duration);
			start_time = std::chrono::high_resolution_clock::now(); // reset start time

			// Draw some graphical entities
			sf::CircleShape point(4.f);
			point.setPosition(((vehicle.x) * 500) + 600, ((vehicle.y) * 500) + 300);
			point.setFillColor(sf::Color::Red);
			
			//window.clear();
			window.draw(point);
			//window.display();
		}
		window.display();
	}
	return 0;
}