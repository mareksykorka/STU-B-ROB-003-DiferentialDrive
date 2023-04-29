#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#define DEFAULT_WHEEL_RADIUS (float)0.05
#define DEFAULT_WHEELBASE ((float)0.1)*2

#define TIME_MILI ((float)0.001)
#define TIME_MICRO ((float)0.000001)

class Wheel {
	public:
		double r;
		double omegaR;
		double vR;

		double phiOffset;
		double x;
		double y;

		Wheel() {
			Wheel(0, 0);
		}

		Wheel(float wheelRadius, double phi) {
			r = wheelRadius;
			omegaR = 0;
			vR = 0;

			phiOffset = phi;

			x = r * cos(phiOffset);
			y = r * sin(phiOffset);
		}

		void recalculate(double xCenter, double yCenter, double phi) {
			vR = omegaR * r;

			x = xCenter + r * cos(phi + phiOffset);
			y = yCenter + r * sin(phi + phiOffset);
		}

		void setAngularVel(float omega) {
			omegaR = omega;
			vR = omegaR * r;
		}
};

class Vehicle {
	public:
		Wheel lWheel;
		Wheel rWheel;

		double l;
		double omegaT;
		double vT;

		double phiT;
		double d_x;
		double d_y;
		double x;
		double y;

		double rICR;
		double xR;
		double yR;

		Vehicle(float wheelbase) {
			l = wheelbase;
			lWheel = Wheel(DEFAULT_WHEEL_RADIUS, -3.1415/2);
			rWheel = Wheel(DEFAULT_WHEEL_RADIUS, 3.1415/2);
			
			omegaT = 0;
			vT = 0;

			phiT = 0;
			d_x = 0;
			d_y = 0;
			x = 0;
			y = 0;

			rICR = 0;
			xR = 0;
			yR = 0;
		}

		void recalculate(double deltaTime) {
			omegaT = (lWheel.vR - rWheel.vR) / l;
			phiT += omegaT * deltaTime;

			lWheel.recalculate(x, y, phiT);
			rWheel.recalculate(x, y, phiT);

			vT = (lWheel.vR + rWheel.vR) / 2.0;
			double vX = vT * cos(phiT);
			double vY = vT * sin(phiT);
			d_x = vX * deltaTime;
			d_y = vY * deltaTime;

			x += d_x;
			y += d_y;

			if (omegaT != 0) {
				rICR = (l / 2) * ((rWheel.vR + lWheel.vR) / (rWheel.vR - lWheel.vR));
			}
		}

		void printData(double timeDelta, double time) {
			printf("dt = %f | t = %3.4f | x = %f | dx = %f | y = %f | Δy = %f | L_v = %f | R_v = %f | T_v = %f |\n", timeDelta, time, this->x, this->d_x, this->y, this->d_y, this->lWheel.vR, this->rWheel.vR, this->vT);
		}
};

int main() {
	//Declare and create a new render-window
	sf::RenderWindow window(sf::VideoMode(1280,720), "SFML window");
	sf::View view(sf::FloatRect(0, 0, 1280, 720));
	window.setView(view);
	bool dragging = false;
	sf::Vector2f lastMousePosition;
	const float sensitivity = 1.0f;

	// Create a vertex array to hold the lines
	sf::VertexArray lines(sf::Lines);

	// Define the spacing and number of lines in the grid
	int gridSize = 50;
	int numLines = window.getSize().x / gridSize;

	// Create the vertical lines
	for (int i = 0; i < numLines; i++) {
		lines.append(sf::Vertex(sf::Vector2f(i * gridSize, 0), sf::Color::White));
		lines.append(sf::Vertex(sf::Vector2f(i * gridSize, window.getSize().y), sf::Color::White));
	}

	// Create the horizontal lines
	for (int i = 0; i < numLines; i++) {
		lines.append(sf::Vertex(sf::Vector2f(0, i * gridSize), sf::Color::White));
		lines.append(sf::Vertex(sf::Vector2f(window.getSize().x, i * gridSize), sf::Color::White));
	}
	window.draw(lines);
	window.display();
	// Draw some graphical entities
	sf::CircleShape point(2.f);

	// Event processing
	sf::Event event;

	// Model logic
	Vehicle vehicle(DEFAULT_WHEELBASE);
	vehicle.lWheel.setAngularVel(2);
	vehicle.rWheel.setAngularVel(2);

	// Count timer
	auto calc_timer = std::chrono::high_resolution_clock::now(); // get current time
	auto draw_timer = calc_timer;
	auto abso_timer = calc_timer;

	// The main loop - ends as soon as the window is closed
	while (window.isOpen())
	{
		// Handle Events
		while (window.pollEvent(event))
		{
			// Request for closing the window
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::MouseWheelScrolled)
			{
				if (event.mouseWheelScroll.delta > 0)
				{
					// zoom in
					view.zoom(0.9f);
				}
				else if (event.mouseWheelScroll.delta < 0)
				{
					// zoom out
					view.zoom(1.1f);
				}
				window.setView(view);
			}

			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
					if (view.getViewport().contains(mousePosition.x / window.getSize().x, mousePosition.y / window.getSize().y))
					{
						dragging = true;
						lastMousePosition = mousePosition;
					}
				}
			}

			if (event.type == sf::Event::MouseButtonReleased)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					dragging = false;
				}
			}

			if (dragging && event.type == sf::Event::MouseMoved)
			{
				sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
				sf::Vector2f delta = lastMousePosition - mousePosition;
				view.move(delta * sensitivity);
				window.setView(view);
				lastMousePosition = mousePosition;
			}

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

		auto end_time = std::chrono::high_resolution_clock::now(); // get current time again
		auto calc_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - calc_timer); // calculate time difference
		auto draw_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - draw_timer); // calculate time difference
		auto abso_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - abso_timer); // calculate time difference

		if (abso_duration.count() * TIME_MILI >= 0) {
			vehicle.lWheel.setAngularVel(2);
			vehicle.rWheel.setAngularVel(2);
		}
		if (abso_duration.count() * TIME_MILI > 5) {
			vehicle.lWheel.setAngularVel(-4);
			vehicle.rWheel.setAngularVel(-4);
		}
		if (abso_duration.count() * TIME_MILI >= 10) {
			vehicle.lWheel.setAngularVel(2);
			vehicle.rWheel.setAngularVel(2);
		}
		if (abso_duration.count() * TIME_MILI >= 15) {
			vehicle.lWheel.setAngularVel(2);
			vehicle.rWheel.setAngularVel(-2);
		}
		if (abso_duration.count() * TIME_MILI >= 20) {
			vehicle.lWheel.setAngularVel(0);
			vehicle.rWheel.setAngularVel(0);
		}
		if (abso_duration.count() * TIME_MILI >= 25) {
			vehicle.lWheel.setAngularVel(0);
			vehicle.rWheel.setAngularVel(0);
		}
		if (abso_duration.count() * TIME_MILI >= 35) {
			vehicle.lWheel.setAngularVel(0);
			vehicle.rWheel.setAngularVel(0);
		}
		if (abso_duration.count() * TIME_MILI >= 50) {
			vehicle.lWheel.setAngularVel(0.5);
			vehicle.rWheel.setAngularVel(0.5);
		}

		
		draw_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - draw_timer); // calculate time difference
		abso_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - abso_timer); // calculate time difference

		// Calculate data
		if (calc_duration.count() >= 400) { // check if 10ms has elapsed
			end_time = std::chrono::high_resolution_clock::now(); // get current time again
			calc_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - calc_timer); // calculate time difference

			vehicle.recalculate(calc_duration.count() * TIME_MICRO);

			calc_timer = std::chrono::high_resolution_clock::now(); // reset start time
			vehicle.printData(calc_duration.count() * TIME_MICRO, abso_duration.count() * TIME_MILI);
		}

		// Displat data
		if (draw_duration.count() >= 100) { // check if 10ms has elapsed

			window.clear();

			window.draw(lines);

			sf::VertexArray line(sf::Lines, 2);
			line[0].position = sf::Vector2f(((vehicle.lWheel.x) * 500) + 600, ((vehicle.lWheel.y) * 500) + 300);
			line[1].position = sf::Vector2f(((vehicle.rWheel.x) * 500) + 600, ((vehicle.rWheel.y) * 500) + 300);
			line[0].color = sf::Color::Magenta;
			line[1].color = sf::Color::Magenta;
			window.draw(line);

			point.setPosition(((vehicle.lWheel.x) * 500) + 600, ((vehicle.lWheel.y) * 500) + 300);
			point.setFillColor(sf::Color::Green);
			window.draw(point);
			
			point.setPosition(((vehicle.x) * 500) + 600, ((vehicle.y) * 500) + 300);
			point.setFillColor(sf::Color::White);
			window.draw(point);

			point.setPosition(((vehicle.rWheel.x) * 500) + 600, ((vehicle.rWheel.y) * 500) + 300);
			point.setFillColor(sf::Color::Red);
			window.draw(point);	

			window.display();
						
			draw_timer = std::chrono::high_resolution_clock::now(); // reset start time
		}
	}
	return 0;
}