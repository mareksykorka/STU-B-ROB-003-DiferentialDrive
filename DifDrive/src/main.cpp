#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <functional>

#define DEFAULT_WHEEL_RADIUS (float)0.05
#define DEFAULT_WHEELBASE ((float)0.1)*2

#define TIME_MILI ((float)0.001)
#define TIME_MICRO ((float)0.000001)

#define BUTTON_PADDING 5.f

class Button {
public:
	Button(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, std::function<void()> callback) {
		background.setSize(size);
		background.setPosition(position);
		background.setFillColor(sf::Color::Transparent);
		background.setOutlineThickness(2);
		background.setOutlineColor(sf::Color::White);

		label.setFont(font);
		label.setString(text);
		label.setFillColor(sf::Color::White);

		// Set the font size to fit within the button rectangle
		float textSize = std::min((background.getSize().x - BUTTON_PADDING * 2.0f) / label.getLocalBounds().width, (background.getSize().y - BUTTON_PADDING * 2.0f) / label.getLocalBounds().height) * label.getCharacterSize();
		label.setCharacterSize(static_cast<unsigned int>(textSize));

		// Center the text within the button rectangle
		label.setOrigin(label.getLocalBounds().left + label.getLocalBounds().width / 2.0f, label.getLocalBounds().top + label.getLocalBounds().height / 2.0f);
		label.setPosition(background.getPosition() + background.getSize() / 2.0f);

		callback_fcn = callback;
	}

	void draw(sf::RenderWindow& window) {
		window.draw(background);
		window.draw(label);
	}

	void handleEvent(sf::Event event, sf::RenderWindow& window) {
		switch (event.type) {
		case sf::Event::MouseMoved:
		{
			sf::FloatRect bounds = background.getGlobalBounds();
			if (bounds.contains(event.mouseMove.x, event.mouseMove.y)) {
				isHovered = true;
				background.setFillColor(sf::Color(255, 163, 163, 255));
			}
			else {
				isHovered = false;
				background.setFillColor(sf::Color::Transparent);
			}
			break;
		}
		case sf::Event::MouseButtonPressed:
		{
			if (event.mouseButton.button == sf::Mouse::Left && isHovered && callback_fcn) {
				callback_fcn();
			}
			break;
		}
		default:
			break;
		}
	}

private:
	sf::RectangleShape background;
	sf::Text label;
	std::function<void()> callback_fcn;
	bool isHovered;
};

class Indicator {
public:
	Indicator(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, sf::Keyboard::Key key, std::function<void()> callback) {
		background.setSize(size);
		background.setPosition(position);
		background.setFillColor(sf::Color::Transparent);
		background.setOutlineThickness(2);
		background.setOutlineColor(sf::Color::White);

		label.setFont(font);
		label.setString(text);
		label.setFillColor(sf::Color::White);

		// Set the font size to fit within the button rectangle
		float textSize = std::min((background.getSize().x - BUTTON_PADDING * 2.0f) / label.getLocalBounds().width, (background.getSize().y - BUTTON_PADDING * 2.0f) / label.getLocalBounds().height) * label.getCharacterSize();
		label.setCharacterSize(static_cast<unsigned int>(textSize));

		// Center the text within the button rectangle
		label.setOrigin(label.getLocalBounds().left + label.getLocalBounds().width / 2.0f, label.getLocalBounds().top + label.getLocalBounds().height / 2.0f);
		label.setPosition(background.getPosition() + background.getSize() / 2.0f);

		keyBind = key;

		callback_fcn = callback;
	}

	void draw(sf::RenderWindow& window) {
		window.draw(background);
		window.draw(label);
	}

	void handleEvent(sf::Event event, sf::RenderWindow& window) {
		if (event.type == sf::Event::KeyPressed && event.key.code == keyBind && callback_fcn)
		{
			background.setFillColor(sf::Color(242, 255, 0, 255 * 0.5));
			callback_fcn();
		} 
		else if (event.type == sf::Event::KeyReleased && event.key.code == keyBind) 
		{
			background.setFillColor(sf::Color::Transparent);
		}
		else if (event.type == sf::Event::LostFocus)
		{
			background.setFillColor(sf::Color::Transparent);
		}
	}

private:
	sf::RectangleShape background;
	sf::Text label;
	sf::Keyboard::Key keyBind;
	std::function<void()> callback_fcn;
};

class Label {

};

class UIPanel {
public:
	UIPanel(sf::Vector2f position, sf::Vector2f size) {
		panel.setPosition(position);
		panel.setSize(size);
		panel.setFillColor(sf::Color(0,0,0,255*0.6));
		panel.setOutlineThickness(5);
		panel.setOutlineColor(sf::Color::Yellow);
	}

	void addButton(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, std::function<void()> callback) {
		Button button(panel.getPosition() + position, size, text, font, callback);
		buttons.push_back(button);
	}

	void addIndicator(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, sf::Keyboard::Key key, std::function<void()> callback) {
		Indicator indicator(panel.getPosition() + position, size, text, font, key, callback);
		indicators.push_back(indicator);
	}

	void draw(sf::RenderWindow& window) {
		window.draw(panel);
		for (Button& button : buttons) {
			button.draw(window);
		}
		for (Indicator& indicator : indicators) {
			indicator.draw(window);
		}
	}

	void handleEvent(sf::Event event, sf::RenderWindow& window) {
		for (Button& button : buttons) {
			button.handleEvent(event, window);
		}
		for (Indicator& indicator : indicators) {
			indicator.handleEvent(event, window);
		}
	}

private:
	sf::RectangleShape panel;
	std::vector<Button> buttons = std::vector<Button>();
	std::vector<Indicator> indicators = std::vector<Indicator>();
	/*std::vector<Label> labels;*/
};

sf::VideoMode resolutionPicker() {
	// Get all available video modes
	std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();

	// Display a list of resolutions to the user
	std::cout << "Available Resolutions:\n";
	for (int i = 0; i < modes.size(); i++)
	{
		std::cout << i + 1 << ". " << modes[i].width << "x" << modes[i].height << " (" << modes[i].bitsPerPixel << " bpp)\n";
	}

	// Ask the user to select a resolution
	int selection;
	do
	{
		std::cout << "Enter selection (1-" << modes.size() << "): ";
		std::cin >> selection;
	} while (selection < 1 || selection > modes.size());

	return modes[selection - 1];
}

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

void placeholder_action() {
	std::cout << "Button 1 clicked!" << std::endl;
}

int main()
{
	sf::RenderWindow window(resolutionPicker(), "Diferential drive simulation", sf::Style::Close);
	sf::Event windowEvent;

	sf::View simulationView(sf::FloatRect(0.f, 0.f, window.getSize().x, window.getSize().y-0));
	simulationView.setViewport(sf::FloatRect(0, 0, 1, 1 - (0 / window.getSize().y)));

	// set the minimum zoom level to the window resolution
	const float minZoom = std::min(window.getSize().x / simulationView.getSize().x, window.getSize().y / simulationView.getSize().y);
	float zoomLevel = 1.0f;

	sf::Font font;
	if (!font.loadFromFile("include/arial.ttf")) {
		// handle font loading error
	}

	UIPanel panel(sf::Vector2f(0.f, window.getSize().y - 100.f), sf::Vector2f(window.getSize().x, 100.f));
	panel.addButton(sf::Vector2f(5.f, 5.f), sf::Vector2f(90.f, 90.f), "Button 1", font, &placeholder_action);
	panel.addButton(sf::Vector2f(105.f, 5.f), sf::Vector2f(90.f, 90.f), "Button 2", font, &placeholder_action);
	panel.addButton(sf::Vector2f(205.f, 5.f), sf::Vector2f(90.f, 90.f), "Button 3", font, &placeholder_action);
	panel.addButton(sf::Vector2f(305.f, 5.f), sf::Vector2f(90.f, 90.f), "Button 4", font, &placeholder_action);
	panel.addButton(sf::Vector2f(405.f, 5.f), sf::Vector2f(90.f, 90.f), "Button 5", font, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - 95.f), 5.f), sf::Vector2f(90.f, 90.f), "D", font, sf::Keyboard::Key::D, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - 195.f), 5.f), sf::Vector2f(90.f, 90.f), "S", font, sf::Keyboard::Key::S, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - 295.f), 5.f), sf::Vector2f(90.f, 90.f), "A", font, sf::Keyboard::Key::A, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - 395.f), 5.f), sf::Vector2f(90.f, 90.f), "W", font, sf::Keyboard::Key::W, &placeholder_action);

	sf::RectangleShape rect(sf::Vector2f(window.getSize().x, window.getSize().y));
	rect.setFillColor(sf::Color::Blue);

	// Model logic
	Vehicle vehicle(DEFAULT_WHEELBASE);

	// Count timer
	auto calc_timer = std::chrono::high_resolution_clock::now(); // get current time
	auto draw_timer = calc_timer;
	auto abso_timer = calc_timer;

	while (window.isOpen())
	{
		while (window.pollEvent(windowEvent))
		{
			panel.handleEvent(windowEvent ,window);

			if (windowEvent.type == sf::Event::KeyPressed) {
				// Handle key press event

				switch (windowEvent.key.code)
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

			// handle zoom in/out events
			if (windowEvent.type == sf::Event::MouseWheelMoved)
			{
				if (windowEvent.mouseWheel.delta > 0)
				{
					zoomLevel *= 1.1f;
				}
				else if (windowEvent.mouseWheel.delta < 0)
				{
					// prevent zooming out beyond the minimum zoom level
					if (true)//zoomLevel / 1.1f >= minZoom)
					{
						zoomLevel /= 1.1f;
					}
					else
					{
						zoomLevel = minZoom;
					}
				}

				simulationView.setSize(window.getSize().x / zoomLevel, (window.getSize().y - 0) / zoomLevel);
			}

			if (windowEvent.type == sf::Event::Closed) {
				window.close();
				return 0;
			}
		}

		auto end_time = std::chrono::high_resolution_clock::now(); // get current time again
		auto calc_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - calc_timer); // calculate time difference
		auto draw_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - draw_timer); // calculate time difference
		auto abso_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - abso_timer); // calculate time difference

		// Calculate data
		if (calc_duration.count() >= 400) { // check if 10ms has elapsed
			end_time = std::chrono::high_resolution_clock::now(); // get current time again
			calc_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - calc_timer); // calculate time difference

			vehicle.recalculate(calc_duration.count() * TIME_MICRO);

			calc_timer = std::chrono::high_resolution_clock::now(); // reset start time
			vehicle.printData(calc_duration.count() * TIME_MICRO, abso_duration.count() * TIME_MILI);
		}

		window.clear();

		window.setView(simulationView);
		window.draw(rect); 

		sf::VertexArray line(sf::Lines, 2);
		line[0].position = sf::Vector2f(((vehicle.lWheel.x) * 100), ((vehicle.lWheel.y) * 100));
		line[1].position = sf::Vector2f(((vehicle.rWheel.x) * 100), ((vehicle.rWheel.y) * 100));
		line[0].color = sf::Color::Magenta;
		line[1].color = sf::Color::Magenta;
		window.draw(line);

		sf::CircleShape point(2.f);
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
		panel.draw(window);

		window.setView(simulationView);

		window.display();
	}

	return 0;
}
