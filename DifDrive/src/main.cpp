#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#define DEFAULT_WHEEL_RADIUS (float)0.05
#define DEFAULT_WHEELBASE ((float)0.1)*2

class Wheel {
	public:
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
			return this->tangentialVel;
		}
};

class Vehicle {
	public:
		Wheel lWheel = Wheel(DEFAULT_WHEEL_RADIUS);
		Wheel rWheel = Wheel(DEFAULT_WHEEL_RADIUS);

		float wheelbase;
		float angularVel;
		float tangentialVel;

		float angle;
		float x;
		float y;

	public:
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

			this->x = this->x + deltaX;
			this->y = this->y + deltaY;
		}

		void printData(double time) {
			printf("%5.2f %2.2f %2.2f\n", time, this->x, this->y);
		}
};


int main()
{
	sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML Line");

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		
		Vehicle vehicle(DEFAULT_WHEELBASE);
		vehicle.lWheel.setAngularVel(1);

		double prevTime = 0, time = prevTime + 1;
		while (true) {
			sf::VertexArray line(sf::LineStrip, 2);
			line[0].position = sf::Vector2f((vehicle.x*500) + 300, (vehicle.y * 500) + 200);
			line[0].color = sf::Color::Red;

			vehicle.calculateXY(time - prevTime);
			vehicle.printData(time);
			
			line[1].position = sf::Vector2f((vehicle.x * 500) + 300, (vehicle.y * 500) + 200);
			line[1].color = sf::Color::Red;

			window.draw(line);
			window.display();

			prevTime = time;
			time += 0.005;
			//std::this_thread::sleep_for(std::chrono::milliseconds(0));

			if (time > 30)
				vehicle.rWheel.setAngularVel(2);

			if (time > 40)
				vehicle.lWheel.setAngularVel(4);


			if (time > 5000)
				break;
		}
		window.clear();
		window.display();
		break;
	}

	return 0;
}