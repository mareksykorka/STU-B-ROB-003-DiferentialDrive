/*#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

#define DEFAULT_WHEEL_RADIUS (float)0.05
#define DEFAULT_WHEELBASE ((float)0.1)*2

#define TIME_MILI ((float)0.001)
#define TIME_MICRO ((float)0.000001)

using namespace sf;
using namespace std;
using namespace std::chrono;

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

	void setTangencialVel(float velocity) {
		vR = velocity;
		omegaR = vR / r;
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
		lWheel = Wheel(DEFAULT_WHEEL_RADIUS, -3.1415 / 2);
		rWheel = Wheel(DEFAULT_WHEEL_RADIUS, 3.1415 / 2);

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
		printf("dt = %f | t = %3.4f | x = %f | dx = %f | y = %f | Δy = %f | L_v = %f | R_v = %f | T_v = %f | om_T = %f\n", timeDelta, time, this->x, this->d_x, this->y, this->d_y, this->lWheel.vR, this->rWheel.vR, this->vT, this->omegaT);
	}
};

VideoMode resolutionPicker() {
	// Get all available video modes
	vector<VideoMode> modes = VideoMode::getFullscreenModes();

	// Display a list of resolutions to the user
	cout << "Available Resolutions:\n";
	for (int i = 0; i < modes.size(); i++)
	{
		cout << i + 1 << ". " << modes[i].width << "x" << modes[i].height << " (" << modes[i].bitsPerPixel << " bpp)\n";
	}

	// Ask the user to select a resolution
	int selection;
	do
	{
		cout << "Enter selection (1-" << modes.size() << "): ";
		cin >> selection;
	} while (selection < 1 || selection > modes.size());

	return modes[selection - 1];
}

int majn()
{
	// Generate new window
	RenderWindow window(resolutionPicker(), "Diferential drive simulation", Style::Close);
	View simulationView(Vector2f(window.getSize().x / 2, window.getSize().y / 2), Vector2f(window.getSize().x, window.getSize().y));
	window.setView(simulationView);

	//UIPanel menu(window);

	RectangleShape rect(Vector2f(window.getSize().x, 100.f));
	rect.setPosition(0.f, window.getSize().y - 100.f);
	rect.setFillColor(Color(255, 255, 255, 255 * 0.3f)); // Set the fill color to a dark color with 50% transparency

	// Create a button
	sf::RectangleShape button(sf::Vector2f(100.f, 80.f));
	button.setPosition(10.f, window.getSize().y - 90.f);
	button.setFillColor(sf::Color::Blue);

	// Create a text label for the button
	sf::Font font;
	font.loadFromFile("include\\arial.ttf");
	sf::Text buttonText("Click me", font, 30);
	buttonText.setFillColor(sf::Color::White);
	buttonText.setPosition(10.f, window.getSize().y - 90.f);


	bool dragging = false;
	const float sensitivity = 1.0f;
	Vector2f lastMousePosition;

	// Model logic
	Vehicle vehicle(DEFAULT_WHEELBASE);

	// Count timer
	auto calc_timer = high_resolution_clock::now(); // get current time
	auto draw_timer = calc_timer;
	auto abso_timer = calc_timer;

	while (window.isOpen())
	{
		Event event;

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
					simulationView.zoom(0.9f);
				}
				else if (event.mouseWheelScroll.delta < 0)
				{
					// zoom out
					simulationView.zoom(1.1f);
				}
				window.setView(simulationView);
			}

			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
					if (simulationView.getViewport().contains(mousePosition.x / window.getSize().x, mousePosition.y / window.getSize().y))
					{
						dragging = true;
						lastMousePosition = mousePosition;
					}
				}

				// Check if the mouse button was pressed on the button
				if (button.getGlobalBounds().contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
				{
					// Button was clicked, do something
					// For example, change the color of the button
					button.setFillColor(sf::Color::Red);
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
				simulationView.move(delta * sensitivity);
				window.setView(simulationView);
				lastMousePosition = mousePosition;
			}

			if (event.type == sf::Event::KeyPressed) {
				// Handle key press event

				switch (event.key.code)
				{
				case sf::Keyboard::Key::W:
					vehicle.lWheel.setTangencialVel(vehicle.lWheel.vR + 0.1);
					vehicle.rWheel.setTangencialVel(vehicle.rWheel.vR + 0.1);
					break;
				case sf::Keyboard::Key::A:
					vehicle.lWheel.setAngularVel(vehicle.lWheel.omegaR + 0.1);
					vehicle.rWheel.setAngularVel(vehicle.rWheel.omegaR - 0.1);
					break;
				case sf::Keyboard::Key::S:
					vehicle.lWheel.setTangencialVel(vehicle.lWheel.vR - 0.1);
					vehicle.rWheel.setTangencialVel(vehicle.rWheel.vR - 0.1);
					break;
				case sf::Keyboard::Key::D:
					vehicle.lWheel.setAngularVel(vehicle.lWheel.omegaR - 0.1);
					vehicle.rWheel.setAngularVel(vehicle.rWheel.omegaR + 0.1);
					break;
				case sf::Keyboard::Key::Q:
					vehicle.lWheel.setTangencialVel(0);
					vehicle.rWheel.setTangencialVel(0);
					break;
				case sf::Keyboard::Key::E:
					vehicle.lWheel.setAngularVel(0);
					vehicle.rWheel.setAngularVel(0);
					break;
				default:
					break;
				}
			}
		}

		auto end_time = high_resolution_clock::now(); // get current time again
		auto calc_duration = duration_cast<chrono::microseconds>(end_time - calc_timer); // calculate time difference
		auto draw_duration = duration_cast<chrono::milliseconds>(end_time - draw_timer); // calculate time difference
		auto abso_duration = duration_cast<chrono::milliseconds>(end_time - abso_timer); // calculate time difference

		// Calculate data
		if (calc_duration.count() >= 400) { // check if 10ms has elapsed
			end_time = high_resolution_clock::now(); // get current time again
			calc_duration = duration_cast<chrono::microseconds>(end_time - calc_timer); // calculate time difference

			vehicle.recalculate(calc_duration.count() * TIME_MICRO);

			calc_timer = high_resolution_clock::now(); // reset start time
			vehicle.printData(calc_duration.count() * TIME_MICRO, abso_duration.count() * TIME_MILI);
		}

		// Displat data
		if (draw_duration.count() >= 10) { // check if 10ms has elapsed

			window.clear();
			window.setView(simulationView);

			sf::VertexArray line(sf::Lines, 2);
			line[0].position = sf::Vector2f(((vehicle.lWheel.x) * 100), ((vehicle.lWheel.y) * 100));
			line[1].position = sf::Vector2f(((vehicle.rWheel.x) * 100), ((vehicle.rWheel.y) * 100));
			line[0].color = sf::Color::Magenta;
			line[1].color = sf::Color::Magenta;
			window.draw(line);

			CircleShape point(2.f);
			point.setPosition(((vehicle.lWheel.x) * 100), ((vehicle.lWheel.y) * 100));
			point.setFillColor(sf::Color::Green);
			window.draw(point);

			point.setPosition(((vehicle.x) * 100), ((vehicle.y) * 100));
			point.setFillColor(sf::Color::White);
			window.draw(point);

			point.setPosition(((vehicle.rWheel.x) * 100), ((vehicle.rWheel.y) * 100));
			point.setFillColor(sf::Color::Red);
			window.draw(point);

			window.setView(window.getDefaultView());
			window.draw(rect);

			// Draw the button and label
			window.draw(button);
			window.draw(buttonText);

			window.display();
			window.setView(simulationView);

			draw_timer = high_resolution_clock::now(); // reset start time
		}
	}
	return 0;
}
*/