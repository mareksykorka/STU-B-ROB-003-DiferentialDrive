#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <functional>
#include <climits>
#include <deque>

#define DEFAULT_WHEEL_RADIUS 0.05f					//0.05[m] -> 0.5[dm] -> 5 [cm] -> 50 [mm]
#define DEFAULT_WHEELDIST 0.1f						//0.10[m] -> 1.0[dm] -> 10[cm] -> 100[mm]
#define DEFAULT_WHEELBASE (DEFAULT_WHEELDIST * 2)	//0.20[m] -> 2.0[dm] -> 20[cm] -> 200[mm]
#define DEFAULT_TRAIL_LEN 10000
#define DEFAULT_WTRAIL_LEN 100
#define DEFAULT_SCALE 100.f							//[cm]
#define GRID_SPACING 10.f							//[dm] (najmensi dielik)

#define TIME_mS 0.001f
#define TIME_uS (TIME_mS * TIME_mS)

/*#define SIMULATION_FIXED_STEP 0.005f
#define SIMULATION_SECOND_STEP_AMOUNT 200.f*/

#define SIMULATION_FIXED_STEP 0.005f
#define SIMULATION_SECOND_STEP_AMOUNT 200.f

#define DEFAULT_ZOOM 1.f				//?
#define UIPANEL_SIZE 160.f				//pixels
#define BUTTON_PADDING 5.f				//pixels
#define BUTTON_SIZE (UIPANEL_SIZE*0.5f)	//pixels

enum class ApplicationMode {
	GAME_MODE,
	SIMULATION_MODE
};

enum class SimulationMode {
	VECTOR,
	RECTANGLE,
	CURVE,
	GAME
};

class AppConfig {
public:
	static AppConfig& getInstance() {
		static AppConfig instance;
		return instance;
	}

	bool getChangeStatus() {
		return this->appConfigChanged;
	}
	void setChangeStatus(bool status) {
		this->appConfigChanged = status;
	}

	bool getPositionResetStatus() {
		return this->resetVehiclePos;
	}
	void setPositionResetStatus(bool status) {
		this->resetVehiclePos = status;
	}

	bool getTimerResetStatus() {
		return this->resetTimer;
	}
	void setTimerResetStatus(bool status) {
		this->resetTimer = status;
	}

	bool getDataStatus() {
		return this->loadData;
	}
	void setDataStatus(bool status) {
		this->loadData = status;
	}

	sf::Font getAppFont() {
		return this->font;
	}
	void loadDefFont() {
		if (!(this->font.loadFromFile("include/arial.ttf"))) {
			exit(-1);
		}
	}

	sf::Color getColBackground() {
		return colBackground;
	}
	sf::Color getColPrimary() {
		return colPrimary;
	}
	sf::Color getColButtonHigh() {
		return colButtonHigh;
	}
	sf::Color getColIndicatorHigh() {
		return colIndicatorHigh;
	}
	void setDarkMode() {
		this->colBackground = sf::Color::Black;
		this->colPrimary = sf::Color::White;
		this->colButtonHigh = sf::Color(255, 163, 163, 255 * 0.75f);
		this->colIndicatorHigh = sf::Color::Yellow;
	}
	void setLightMode() {
		this->colBackground = sf::Color::White;
		this->colPrimary = sf::Color::Black;
		this->colButtonHigh = sf::Color(255, 163, 163, 255 * 0.75f);
		this->colIndicatorHigh = sf::Color::Yellow;
	}

	float getZoomLevel() {
		return this->zoomLevel;
	}
	void setZoomLevel(float newZoomLevel) {
		this->zoomLevel = newZoomLevel;
	}

	ApplicationMode getAppMode() {
		return this->appMode;
	}
	void setSimulationMode() {
		this->appMode = ApplicationMode::SIMULATION_MODE;
	}
	void setGameMode() {
		this->appMode = ApplicationMode::GAME_MODE;
	}

	SimulationMode getSimMode() {
		return this->simMode;
	}
	void setVectorSimulation() {
		this->simMode = SimulationMode::VECTOR;
	}
	void setRectangleSimulation() {
		this->simMode = SimulationMode::RECTANGLE;
	}
	void setCurveSimulation() {
		this->simMode = SimulationMode::CURVE;
	}
	void setGameSimulation() {
		this->simMode = SimulationMode::GAME;
	}

private:
	AppConfig() {
		this->loadDefFont();
		this->setDarkMode();
		this->setZoomLevel(DEFAULT_ZOOM);
		this->setGameMode();
		this->setGameSimulation();
	}
	AppConfig(const AppConfig&) = delete;
	AppConfig& operator=(const AppConfig&) = delete;

	bool appConfigChanged;
	bool resetVehiclePos;
	bool resetTimer;

	bool loadData;

	sf::Font font;

	sf::Color colBackground;
	sf::Color colPrimary;
	sf::Color colButtonHigh;
	sf::Color colIndicatorHigh;

	float zoomLevel;

	ApplicationMode appMode;

	SimulationMode simMode;
};

class Grid {
private:
	std::vector<sf::VertexArray> grid;
	std::vector<sf::Text> xRuler;
	std::vector<sf::Text> yRuler;
	sf::Vector2f lastPos;
	sf::VertexArray line;
	int gridSpacing = GRID_SPACING;
	sf::Font gridFont;
	sf::Color color;

public:
	Grid(sf::Font font) {
		AppConfig& config = AppConfig::getInstance();
		grid = std::vector<sf::VertexArray>();
		xRuler = std::vector<sf::Text>();
		yRuler = std::vector<sf::Text>();
		line = sf::VertexArray(sf::Lines, 2);
		lastPos = sf::Vector2f();
		gridFont = font;
		color = config.getColPrimary();
	}

	void checkRecalculate(sf::Vector2f vehiclePos, sf::Vector2u windowSize) {
		if ((abs(lastPos.x - vehiclePos.x)*DEFAULT_SCALE > (windowSize.x / 2)) || (abs(lastPos.y - vehiclePos.y) * DEFAULT_SCALE > (windowSize.y / 2)))
			recalculate(vehiclePos, windowSize);
	}

	void recalculate(sf::Vector2f vehiclePos, sf::Vector2u windowSize) {
		grid.clear();
		xRuler.clear();
		yRuler.clear();
		long shiftedCenterX = round((vehiclePos.x * DEFAULT_SCALE) / (float)gridSpacing) * gridSpacing;
		long shiftedCenterY = round((vehiclePos.y * DEFAULT_SCALE) / (float)gridSpacing) * gridSpacing;
		double shiftedMaximumX = (windowSize.x);
		double shiftedMaximumY = (windowSize.y);

		for (long i = -((shiftedMaximumX) - shiftedCenterX); i <= ((shiftedMaximumX) + shiftedCenterX); i += gridSpacing)
		{
			if (i == 0 || i % 100 == 0) {
				color.a = 255 * 0.75;
			}
			else {
				color.a = 255 * 0.25;
			}
			line.clear();
			line.append(sf::Vertex(sf::Vector2f(i, (-shiftedMaximumY) + shiftedCenterY), color));
			line.append(sf::Vertex(sf::Vector2f(i, (shiftedMaximumY) + shiftedCenterY), color));
			grid.push_back(line);
		}

		for (long i = -((shiftedMaximumY) - shiftedCenterY); i <= ((shiftedMaximumY) + shiftedCenterY); i += gridSpacing)
		{
			if (i == 0 || i % 100 == 0) {
				color.a = 255 * 0.75;
			}
			else {
				color.a = 255 * 0.25;
			}
			line.clear();
			line.append(sf::Vertex(sf::Vector2f((-shiftedMaximumX) + shiftedCenterX, i), color));
			line.append(sf::Vertex(sf::Vector2f((shiftedMaximumX) + shiftedCenterX, i), color));
			grid.push_back(line);
		}
		lastPos = vehiclePos;
	}
	
	void recolor() {
		AppConfig& config = AppConfig::getInstance();
		this->color = config.getColPrimary();
	}

	void draw(sf::RenderWindow& window) {
		for (sf::VertexArray& line : grid) {
			window.draw(line);
		}
	}
};

class Ruler {
public:
	Ruler() {
		AppConfig& config = AppConfig::getInstance();
		xLabels = std::vector<sf::Text>();
		yLabels = std::vector<sf::Text>();
		xDimension = sf::Text(); //[m]
		yDimension = sf::Text(); //[m]
		font = config.getAppFont();
		color = config.getColPrimary();
	}

	void recalculate(sf::Vector2f vehiclePos, sf::Vector2u windowSize, sf::Vector2f offset) {
		xLabels.clear();
		yLabels.clear();
		double shiftedCenterX = round((vehiclePos.x * DEFAULT_SCALE) / GRID_SPACING) * GRID_SPACING;
		double shiftedCenterY = round((vehiclePos.y * DEFAULT_SCALE) / GRID_SPACING) * GRID_SPACING;
		double shiftedMaximumX = (windowSize.x);
		double shiftedMaximumY = (windowSize.y);

		sf::Text xLabel = sf::Text("", font, 30U);
		xLabel.setOrigin(sf::Vector2f(xLabel.getLocalBounds().width / 2, xLabel.getLocalBounds().height / 2));
		xLabel.setFillColor(color);

		for (long i = -((shiftedMaximumX)-shiftedCenterX); i <= ((shiftedMaximumX)+shiftedCenterX); i += GRID_SPACING)
		{
			if (i % 100 == 0) {
				xLabel.setString(std::to_string((int)(i / DEFAULT_SCALE)));
				xLabel.setPosition(sf::Vector2f(i, (shiftedMaximumY / 2) + (vehiclePos.y * DEFAULT_SCALE) - (offset.y + xLabel.getLocalBounds().height*2.5f)));
				xLabels.push_back(xLabel);
			}
		}

		sf::Text yLabel = sf::Text("", font, 30U);
		yLabel.setOrigin(sf::Vector2f(yLabel.getLocalBounds().width / 2, yLabel.getLocalBounds().height / 2));
		yLabel.setFillColor(color);

		for (long i = -((shiftedMaximumY)-shiftedCenterY); i <= ((shiftedMaximumY)+shiftedCenterY); i += GRID_SPACING)
		{
			if (i % 100 == 0) {
				yLabel.setString(std::to_string((int)(-i / DEFAULT_SCALE)));
				yLabel.setPosition(sf::Vector2f((-shiftedMaximumX / 2) + (vehiclePos.x * DEFAULT_SCALE) + 25, i));
				yLabels.push_back(yLabel);
			}
		}
	}

	void recolor() {
		AppConfig& config = AppConfig::getInstance();
		this->color = config.getColPrimary();
	}

	void draw(sf::RenderWindow& window) {
		for (sf::Text& xText : xLabels) {
			window.draw(xText);
		}
		for (sf::Text& yText : yLabels) {
			window.draw(yText);
		}
	}

private:
	std::vector<sf::Text> xLabels;
	std::vector<sf::Text> yLabels;
	sf::Text xDimension;
	sf::Text yDimension;
	sf::Font font;
	sf::Color color;
};

class Button {
public:
	Button(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, std::function<void()> callback) {
		AppConfig& config = AppConfig::getInstance();
		this->buttonOutline = config.getColPrimary();
		this->buttonBackground = sf::Color::Transparent;
		this->buttonText = config.getColPrimary();
		this->buttonHighlight = config.getColButtonHigh();

		background.setSize(sf::Vector2f(size.x - 2 * BUTTON_PADDING, size.y - 2 * BUTTON_PADDING));
		background.setPosition(sf::Vector2f(position.x + BUTTON_PADDING, position.y + BUTTON_PADDING));
		background.setOutlineThickness(2);

		label.setFont(font);
		label.setString(text);

		float xSize = (background.getSize().x - BUTTON_PADDING * 2.0f) / label.getLocalBounds().width;
		float ySize = (background.getSize().y - BUTTON_PADDING * 2.0f) / label.getLocalBounds().height;
		// Set the font size to fit within the button rectangle
		float textSize = std::min(xSize, ySize) * label.getCharacterSize();
		label.setCharacterSize(static_cast<unsigned int>(textSize));

		// Center the text within the button rectangle
		label.setOrigin(label.getLocalBounds().left + label.getLocalBounds().width / 2.0f, label.getLocalBounds().top + label.getLocalBounds().height / 2.0f);
		label.setPosition(background.getPosition() + background.getSize() / 2.0f);

		this->recolor();
		callback_fcn = callback;
	}

	void handleEvent(sf::Event event, sf::RenderWindow& window) {
		switch (event.type) {
		case sf::Event::MouseMoved:
		{
			sf::FloatRect bounds = background.getGlobalBounds();
			if (bounds.contains(event.mouseMove.x, event.mouseMove.y)) {
				isHovered = true;
				background.setFillColor(this->buttonHighlight);
			}
			else {
				isHovered = false;
				background.setFillColor(this->buttonBackground);
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

	void recolor() {
		AppConfig& config = AppConfig::getInstance();
		this->buttonOutline = config.getColPrimary();
		this->buttonBackground = sf::Color::Transparent;
		this->buttonText = config.getColPrimary();
		this->buttonHighlight = config.getColButtonHigh();

		background.setFillColor(this->buttonBackground);
		background.setOutlineColor(this->buttonOutline);
		label.setFillColor(this->buttonText);
	}

	void draw(sf::RenderWindow& window) {
		window.draw(background);
		window.draw(label);
	}

private:
	sf::RectangleShape background;
	sf::Text label;

	sf::Color buttonOutline;
	sf::Color buttonBackground;
	sf::Color buttonText;
	sf::Color buttonHighlight;

	std::function<void()> callback_fcn;
	bool isHovered;
};

class Indicator {
public:
	Indicator(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, sf::Keyboard::Key key, std::function<void()> callback) {
		AppConfig& config = AppConfig::getInstance();
		this->indicatorOutline = config.getColPrimary();
		this->indicatorBackground = sf::Color::Transparent;
		this->indicatorText = config.getColPrimary();
		this->indicatorHighlight = config.getColIndicatorHigh();

		background.setSize(sf::Vector2f(size.x - 2 * BUTTON_PADDING, size.y - 2 * BUTTON_PADDING));
		background.setPosition(sf::Vector2f(position.x + BUTTON_PADDING, position.y + BUTTON_PADDING));
		background.setOutlineThickness(2);

		label.setFont(font);
		label.setString(text);

		float xSize = (background.getSize().x - BUTTON_PADDING * 2.0f) / label.getLocalBounds().width;
		float ySize = (background.getSize().y - BUTTON_PADDING * 2.0f) / label.getLocalBounds().height;
		// Set the font size to fit within the button rectangle
		float textSize = std::min(xSize, ySize) * label.getCharacterSize();
		label.setCharacterSize(static_cast<unsigned int>(textSize));
		// Center the text within the button rectangle
		label.setOrigin(label.getLocalBounds().left + label.getLocalBounds().width / 2.0f, label.getLocalBounds().top + label.getLocalBounds().height / 2.0f);
		label.setPosition(background.getPosition() + background.getSize() / 2.0f);

		this->recolor();
		keyBind = key;
		callback_fcn = callback;
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

	void recolor() {
		AppConfig& config = AppConfig::getInstance();
		this->indicatorOutline = config.getColPrimary();
		this->indicatorBackground = sf::Color::Transparent;
		this->indicatorText = config.getColPrimary();
		this->indicatorHighlight = config.getColIndicatorHigh();

		background.setFillColor(this->indicatorBackground);
		background.setOutlineColor(this->indicatorOutline);
		label.setFillColor(this->indicatorText);
	}

	void draw(sf::RenderWindow& window) {
		window.draw(background);
		window.draw(label);
	}

private:
	sf::RectangleShape background;
	sf::Text label;

	sf::Color indicatorOutline;
	sf::Color indicatorBackground;
	sf::Color indicatorText;
	sf::Color indicatorHighlight;

	sf::Keyboard::Key keyBind;
	std::function<void()> callback_fcn;
};

class Label {

};

class UIPanel {
public:
	UIPanel(sf::Vector2f position, sf::Vector2f size) {
		AppConfig& config = AppConfig::getInstance();
		this->panelOutline = config.getColPrimary();
		this->panelBackground = config.getColBackground();
		this->panelBackground.a = 255 * 0.75;

		panel.setPosition(position);
		panel.setSize(size);
		panel.setOutlineThickness(2);
		this->recolor();
	}

	sf::Vector2f getSize() {
		return this->panel.getSize();
	}

	void addButton(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, std::function<void()> callback) {
		Button button(panel.getPosition() + position, size, text, font, callback);
		buttons.push_back(button);
	}

	void addIndicator(sf::Vector2f position, sf::Vector2f size, std::string text, sf::Font& font, sf::Keyboard::Key key, std::function<void()> callback) {
		Indicator indicator(panel.getPosition() + position, size, text, font, key, callback);
		indicators.push_back(indicator);
	}

	void recolor() {
		AppConfig& config = AppConfig::getInstance();
		this->panelOutline = config.getColPrimary();
		this->panelBackground = config.getColBackground();
		this->panelBackground.a = 255 * 0.75;

		panel.setFillColor(this->panelBackground);
		panel.setOutlineColor(this->panelOutline);

		for (Button& button : buttons) {
			button.recolor();
		}
		for (Indicator& indicator : indicators) {
			indicator.recolor();
		}
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

	sf::Color panelOutline;
	sf::Color panelBackground;

	/*std::vector<Label> labels;*/
};

sf::VideoMode resolutionPicker() {
	// Get all available video modes
	std::vector<sf::VideoMode> allModes = sf::VideoMode::getFullscreenModes();
	std::vector<sf::VideoMode> modes = std::vector<sf::VideoMode>();

	for (const auto& mode : allModes) {
		if (mode.width >= 1280 && mode.height >= 720) {
			modes.push_back(mode);
		}
	}

	int count = 1;
	if (modes.size() != 0) {
		std::cout << "Available Resolutions:\n";
		for (const auto& mode : allModes) {
			if (mode.width >= 1280 && mode.height >= 720) {
				std::cout << count << ". " << mode.width << "x" << mode.height << std::endl;
				count++;
			}
		}
	} else {
		std::cout << "This device does not have any Available Compatible Resolutions. Program will be ended.\n";
		exit(-1);
	}

	int selection;
	do {
		std::cout << "Enter selection (1-" << count - 1 << "): ";
		std::cin >> selection;
	} while (selection < 1 || selection > count - 1);

	return modes[selection - 1];
}

class Wheel {
public:
	Wheel (float wheelRadius, double phi) {
		r = wheelRadius;
		omegaR = 0;
		vR = 0;

		phiOffset = phi;
		x = DEFAULT_WHEELBASE * 0.5f * cos(phiOffset);
		y = DEFAULT_WHEELBASE * 0.5f * sin(phiOffset);

		oldX = std::deque<double>();
		oldY = std::deque<double>();
	}

	double getAngularVel() {
		return this->omegaR;
	}

	void setAngularVel(double omega) {
		this->omegaR = omega;
		this->calcTangencialVel();
	}

	double getTangencialVel() {
		return this->vR;
	}

	void setTangencialVel(double velocity) {
		this->vR = velocity;
		this->calcAngularVel();
	}

	void recalcWheelPos(double xCenter, double yCenter, double phi) {
		oldX.push_front(x);
		oldY.push_front(y);
		if (oldX.size() > DEFAULT_TRAIL_LEN) {
			oldX.pop_back();
			oldY.pop_back();
		}

		x = xCenter + DEFAULT_WHEELBASE * 0.5f * cos(phi + this->phiOffset);
		y = yCenter + DEFAULT_WHEELBASE * 0.5f * sin(phi + this->phiOffset);
	}

	void draw(sf::RenderWindow& window, sf::Color color) {
		sf::CircleShape point = sf::CircleShape(1.f);
		point.setOrigin(sf::Vector2f(point.getRadius(), point.getRadius()));
		point.setPosition(x * DEFAULT_SCALE, -y * DEFAULT_SCALE);
		point.setFillColor(color);
		window.draw(point);
	}

	void drawTrail(sf::RenderWindow& window, sf::Color color) {
		sf::CircleShape point = sf::CircleShape(1.f);
		point.setOrigin(sf::Vector2f(point.getRadius(), point.getRadius()));
		for (int i = 0; i < oldX.size(); i++) {
			point.setPosition(oldX[i] * DEFAULT_SCALE, -oldY[i] * DEFAULT_SCALE);
			point.setFillColor(color);
			window.draw(point);
		}
	}

	void deleteTrail() {
		oldX.clear();
		oldY.clear();
	}

private:
	double r;
	double omegaR;
	double vR;

	double phiOffset;
	double x;
	double y;

	sf::CircleShape point;

	std::deque<double> oldX;
	std::deque<double> oldY;

	void calcAngularVel() {
		this->omegaR = this->vR / this->r;
	}

	void calcTangencialVel() {
		this->vR = this->omegaR * this->r;
	}
};

class Vehicle {
public:
	Wheel lWheel = Wheel(DEFAULT_WHEEL_RADIUS, (-3.1415 / 2));
	Wheel rWheel = Wheel(DEFAULT_WHEEL_RADIUS, (3.1415 / 2));

	Vehicle(double wheelbase) {
		l = wheelbase;
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

		oldX = std::deque<double>();
		oldY = std::deque<double>();
	}

	double getX() {
		return this->x;
	}

	double getY() {
		return this->y;
	}

	void resetPosition() {
		this->setAngularVel(0);
		this->setTangencialVel(0);
		this->x = 0;
		this->y = 0;
		this->phiT = 0;
		lWheel.recalcWheelPos(x, y, phiT);
		rWheel.recalcWheelPos(x, y, phiT);
	}

	double getAngularVel() {
		return this->omegaT;
	}

	void setAngularVel(double omega) {
		this->omegaT = omega;
		this->rWheel.setTangencialVel(((2 * this->vT) + (this->l * this->omegaT)) / 2);
		this->lWheel.setTangencialVel(((2 * this->vT) - (this->l * this->omegaT)) / 2);
	}

	double getTangencialVel() {
		return this->vT;
	}

	void setTangencialVel(double velocity) {
		this->vT = velocity;
		this->rWheel.setTangencialVel(((2 * this->vT) + (l * this->omegaT)) / 2);
		this->lWheel.setTangencialVel(((2 * this->vT) - (l * this->omegaT)) / 2);
	}

	void recalculate(double deltaTime) {
		oldX.push_front(x);
		oldY.push_front(y);
		if (oldX.size() > DEFAULT_TRAIL_LEN) {
			oldX.pop_back();
			oldY.pop_back();
		}

		this->omegaT = (rWheel.getTangencialVel() - lWheel.getTangencialVel()) / this->l;
		this->phiT += this->omegaT * deltaTime;

		this->vT = (rWheel.getTangencialVel() + lWheel.getTangencialVel()) / 2.0;
		double vX = vT * cos(phiT);
		double vY = vT * sin(phiT);
		d_x = vX * deltaTime;
		d_y = vY * deltaTime;

		x += d_x;
		y += d_y;

		lWheel.recalcWheelPos(x, y, phiT);
		rWheel.recalcWheelPos(x, y, phiT);
	}

	void printData(double timeDelta, double time) {
		printf("dt = %f | t = %3.4f | x = %f | dx = %f | y = %f | Δy = %f | L_v = %f | R_v = %f | T_v = %f | om_T = %f\n", timeDelta, time, this->x, this->d_x, this->y, this->d_y, this->lWheel.getTangencialVel(), this->rWheel.getTangencialVel(), this->vT, this->omegaT);
	}

	void draw(sf::RenderWindow& window) {
		this->drawTrail(window, sf::Color::White);

		sf::CircleShape point = sf::CircleShape(1.f);
		point.setOrigin(sf::Vector2f(point.getRadius(), point.getRadius()));
		point.setPosition(x * DEFAULT_SCALE, -y * DEFAULT_SCALE);
		point.setFillColor(sf::Color::White);
		window.draw(point);

		lWheel.drawTrail(window, sf::Color::Red);
		rWheel.drawTrail(window, sf::Color::Green);

		lWheel.draw(window, sf::Color::Red);
		rWheel.draw(window, sf::Color::Green);
	}

	void drawTrail(sf::RenderWindow& window, sf::Color color) {
		sf::CircleShape point = sf::CircleShape(1.f);
		point.setOrigin(sf::Vector2f(point.getRadius(), point.getRadius()));
		for (int i = 0; i < oldX.size(); i++) {
			point.setPosition(oldX[i] * DEFAULT_SCALE, -oldY[i] * DEFAULT_SCALE);
			point.setFillColor(color);
			window.draw(point);
		}
	}

	void deleteTrail() {
		oldX.clear();
		oldY.clear();
		lWheel.deleteTrail();
		rWheel.deleteTrail();
	}

private:
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
	std::deque<double> oldX;
	std::deque<double> oldY;
};

class SimulationData {
public:
	SimulationData() {
		vTleft = std::map<double, double>();
		vTright = std::map<double, double>();

		rectangleSide = 0;

		r1 = 0;
		l = 0;
		r2 = 0;
	}

	void getInputData() {
		if (config.getSimMode() == SimulationMode::VECTOR) {
			setFixedVectorData();
		}
		if (config.getSimMode() == SimulationMode::RECTANGLE) {
			getRectangleData();
			calculateRectangleData();
		}
		if (config.getSimMode() == SimulationMode::CURVE) {
			getCurveData();
			calculateCurvaData();
		}
		config.setDataStatus(false);
	}

	void setFixedVectorData() {
		vTleft.clear();
		vTright.clear();
		vTleft = { {0,2},{5,-1},{10,0},{11,2},{16,1} };
		vTright = { {0,2},{5,1},{10,0},{11,-2},{16,1} };
		//vTleft = { {0,1},{5,-0.1},{6.57,1},{11.57,-0.1},{13.14,1},  {16.14,1.2},{22.42,0}       ,{23.42,1} };
		//vTright = {{0,1},{5, 0.1},{6.57,1},{11.57, 0.1},{13.14,1.1},{16.14,1  },{22.42,3.14*0.2},{23.42,1} };
	}
	void getVectorData() {
		vTleft.clear();
		vTright.clear();
		int n;
		std::cout << "\n\n===========================================================\n\n";
		std::cout << "Enter the amount of speed changes during simulation: ";
		std::cin >> n;

		double prevTime = 0;
		double time;

		for (int i = 0; i < n; i++) {
			do {
				std::cout << "Time [s] " << i+1 << " : ";
				std::cin >> time;
			} while (time < prevTime);
			prevTime = time;
			std::cout << "Tangencial speed vT of Left wheel [m/s]: ";
			std::cin >> vTleft[time];
			std::cout << "Tangencial speed vT of Right wheel [m/s]: ";
			std::cin >> vTright[time];
		}
	}

	void getRectangleData() {
		rectangleSide = 0;
		std::cout << "\n\n===========================================================\n\n";
		do {
			std::cout << "Enter size of rectangle side to draw [m]: ";
			std::cin >> rectangleSide;
		} while (rectangleSide < 0);
	}
	void calculateRectangleData() {
		vTleft.clear();
		vTright.clear();

		double sideTime = 1;
		double turnTime = 1;
		double omegaT = (-90 * (3.1415 / 180.0)) / (turnTime);
		double time = 0;

		for (int i = 0; i < 4; i++) {
			vTleft[time] = this->rectangleSide / (sideTime);
			vTright[time] = this->rectangleSide / (sideTime);

			time += sideTime;

			vTleft[time] = (DEFAULT_WHEELBASE * omegaT) / 2;
			vTright[time] = -(DEFAULT_WHEELBASE * omegaT) / 2;

			time += turnTime;
		}
		vTleft[time] = 0;
		vTright[time] = 0;
	}

	void getCurveData() {
		r1 = 0; l = 0; r2 = 0;
		std::cout << "\n\n===========================================================\n\n";

		std::cout << "Enter radius of the first curve [m]: ";
		std::cin >> r1;

		std::cout << "Enter length betwwn curves [m]: ";
		std::cin >> l;

		std::cout << "Enter radius of the fsecondirst curve [m]: ";
		std::cin >> r2;
	}
	void calculateCurvaData() {
		vTleft.clear();
		vTright.clear();

		double turnTime = 1;
		double omegaT;
		double vT;
		double time = 0;

		omegaT = (-90 * (3.1415 / 180.0)) / (turnTime);
		vT = ((90 * (3.1415 / 180.0)) * r1) / (turnTime);
		vTleft[time] =  ((2 * vT) + (DEFAULT_WHEELBASE * omegaT)) / 2;
		vTright[time] = ((2 * vT) - (DEFAULT_WHEELBASE * omegaT)) / 2;

		time += turnTime;

		vTleft[time] = this->l / (turnTime);
		vTright[time] = this->l / (turnTime);

		time += turnTime;

		omegaT = (+90 * (3.1415 / 180.0)) / (turnTime);
		vT = ((90 * (3.1415 / 180.0)) * r1) / (turnTime);
		vTleft[time] = ((2 * vT) + (DEFAULT_WHEELBASE * omegaT)) / 2;
		vTright[time] = ((2 * vT) - (DEFAULT_WHEELBASE * omegaT)) / 2;
	}

	void setVehicleSpeed(double time, Vehicle& vehicle) {
		std::map<double, double>::iterator it = vTleft.lower_bound(time); // iterator to the first element with key >= key
		if (it != vTleft.begin()) {
			--it;
			vehicle.lWheel.setTangencialVel(it->second);
		}
		
		it = vTright.lower_bound(time); // iterator to the first element with key >= key
		if (it != vTright.begin()) {
			--it;
			vehicle.rWheel.setTangencialVel(it->second);
		}
	}

private:
	AppConfig& config = AppConfig::getInstance();
	std::map<double, double> vTleft;
	std::map<double, double> vTright;
	double rectangleSide;
	double r1;
	double l;
	double r2;
};

void placeholder_action() {
	//std::cout << "Button 1 clicked!" << std::endl;
}

void b_one() {
	AppConfig& config = AppConfig::getInstance();
	config.setVectorSimulation();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
	config.setDataStatus(true);
}
void b_second() {
	AppConfig& config = AppConfig::getInstance();
	config.setRectangleSimulation();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
	config.setDataStatus(true);
}
void b_third() {
	AppConfig& config = AppConfig::getInstance();
	config.setCurveSimulation();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
	config.setDataStatus(true);
}
void b_fourth() {
	AppConfig& config = AppConfig::getInstance();
	config.setGameSimulation();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
}

void b_sim() {
	AppConfig& config = AppConfig::getInstance();
	config.setSimulationMode();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
}
void b_game() {
	AppConfig& config = AppConfig::getInstance();
	config.setGameMode();
	config.setPositionResetStatus(true);
	config.setTimerResetStatus(true);
}

void b_lMode() {
	AppConfig& config = AppConfig::getInstance();
	config.setLightMode();
	config.setChangeStatus(true);
}
void b_dMode() {
	AppConfig& config = AppConfig::getInstance();
	config.setDarkMode();
	config.setChangeStatus(true);
}

void b_posReset() {
	AppConfig& config = AppConfig::getInstance();
	config.setPositionResetStatus(true);
}
void b_timeReset() {
	AppConfig& config = AppConfig::getInstance();
	config.setTimerResetStatus(true);
}

int main() {
	AppConfig& config = AppConfig::getInstance();

	sf::RenderWindow window(resolutionPicker(), "Diferential drive simulation", sf::Style::Close);
	sf::Event event;

	sf::View simulationView(sf::FloatRect(0.f, 0.f, window.getSize().x, window.getSize().y));
	simulationView.setViewport(sf::FloatRect(0, 0, 1, 1));

	sf::Font font = config.getAppFont();

	double topRow = 0.f;
	double botRow = 160.f * 0.5f;
	double oneCol = 160.f * 0.5f;
	double oneRow = 160.f * 0.5f;
	sf::Vector2f oneSize = sf::Vector2f(oneCol * 1.f, oneRow * 1.f);

	UIPanel panel(sf::Vector2f(0.f, window.getSize().y - 160.f), sf::Vector2f(window.getSize().x, 160.f));
	panel.addButton(sf::Vector2f(oneCol * 0.f, topRow), sf::Vector2f(oneCol * 3.f, oneRow * 1.f), "Simulation mode", font, &b_sim);
	panel.addButton(sf::Vector2f(oneCol * 0.f, botRow), oneSize, "Vector", font, &b_one);
	panel.addButton(sf::Vector2f(oneCol * 1.f, botRow), oneSize, "Square", font, &b_second);
	panel.addButton(sf::Vector2f(oneCol * 2.f, botRow), oneSize, "Curve", font, &b_third);

	panel.addButton(sf::Vector2f(oneCol * 3.f, topRow), oneSize, "Game\nmode", font, &b_game);
	panel.addButton(sf::Vector2f(oneCol * 3.f, botRow), oneSize, "Manual\ncontrol", font, &b_fourth);

	panel.addButton(sf::Vector2f(oneCol * 4.f, topRow), oneSize, "Light\nmode", font, &b_lMode);
	panel.addButton(sf::Vector2f(oneCol * 4.f, botRow), oneSize, "Dark\nmode", font, &b_dMode);

	panel.addButton(sf::Vector2f((window.getSize().x - (oneCol * 1.f)), topRow), oneSize, "Reset\nposition", font, &b_posReset);
	panel.addButton(sf::Vector2f((window.getSize().x - (oneCol * 3.f)), topRow), oneSize, "Show\nmenu", font, &b_posReset);

	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 6.f)), topRow), oneSize, "Q", font, sf::Keyboard::Key::Q, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 5.f)), topRow), oneSize, "R", font, sf::Keyboard::Key::R, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 4.f)), topRow), oneSize, "M", font, sf::Keyboard::Key::M, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 6.f)), botRow), sf::Vector2f(oneCol * 3.f, oneRow * 1.f), "\n\n|________|\n", font, sf::Keyboard::Key::Space, &placeholder_action);

	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 3.f)), botRow), oneSize, "A", font, sf::Keyboard::Key::A, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 2.f)), topRow), oneSize, "W", font, sf::Keyboard::Key::W, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 2.f)), botRow), oneSize, "S", font, sf::Keyboard::Key::S, &placeholder_action);
	panel.addIndicator(sf::Vector2f((window.getSize().x - (oneCol * 1.f)), botRow), oneSize, "D", font, sf::Keyboard::Key::D, &placeholder_action);

	// Model logic
	Vehicle vehicle = Vehicle(DEFAULT_WHEELBASE);
	simulationView.setCenter(vehicle.getX(), -vehicle.getY());
	SimulationData data = SimulationData();

	Grid grid = Grid(font);
	grid.recalculate(sf::Vector2f(0, 0), window.getSize());

	Ruler rulers = Ruler();
	rulers.recalculate(sf::Vector2f(0, 0), window.getSize(), panel.getSize());

	// Count timer
	auto calc_timer = std::chrono::high_resolution_clock::now(); // get current time
	auto abso_timer = calc_timer;
	auto draw_timer = calc_timer;
	long stepCounter = 0;

	while (window.isOpen())
	{
		if (config.getChangeStatus()) {
			panel.recolor();
			grid.recolor();
			grid.recalculate(sf::Vector2f(vehicle.getX(), -vehicle.getY()), window.getSize());
			rulers.recolor();
			config.setChangeStatus(false);
		}

		if (config.getPositionResetStatus()) {
			vehicle.resetPosition();
			vehicle.deleteTrail();
			config.setPositionResetStatus(false);
		}

		if (config.getTimerResetStatus()) {
			calc_timer = std::chrono::high_resolution_clock::now();
			abso_timer = calc_timer;
			draw_timer = calc_timer;
			stepCounter = 0;
			vehicle.deleteTrail();
			config.setTimerResetStatus(false);
		}

		if (config.getDataStatus()) {
			data.getInputData();
		}

		while (window.pollEvent(event))
		{
			panel.handleEvent(event ,window); // Handle events in UIPanel
			// Handle key press event
			if (event.type == sf::Event::KeyPressed && config.getSimMode() == SimulationMode::GAME) {
				switch (event.key.code)
				{
				case sf::Keyboard::Key::W:
					vehicle.setTangencialVel(vehicle.getTangencialVel() + 0.1);
					break;
				case sf::Keyboard::Key::A:
					vehicle.setAngularVel(vehicle.getAngularVel() + 0.1);
					break;
				case sf::Keyboard::Key::S:
					vehicle.setTangencialVel(vehicle.getTangencialVel() - 0.1);
					break;
				case sf::Keyboard::Key::D:
					vehicle.setAngularVel(vehicle.getAngularVel() - 0.1);
					break;
				case sf::Keyboard::Key::Q:
					vehicle.setTangencialVel(0);
					break;
				case sf::Keyboard::Key::R:
					vehicle.setAngularVel(0);
					break;
				case sf::Keyboard::Key::M:
					// Show menu
					break;
				case sf::Keyboard::Key::Space:
					vehicle.setTangencialVel(0);
					vehicle.setAngularVel(0);
					break;
				default:
					break;
				}
			}

			// handle zoom in/out events
			if (event.type == sf::Event::MouseWheelMoved)
			{
				if (event.mouseWheel.delta > 0)
				{
					config.setZoomLevel(config.getZoomLevel() * 1.1f);
				}
				else if (event.mouseWheel.delta < 0)
				{
					config.setZoomLevel(config.getZoomLevel() / 1.1f);
				}
				simulationView.setSize(window.getSize().x / config.getZoomLevel(), (window.getSize().y - 0) / config.getZoomLevel());
			}

			if (event.type == sf::Event::Closed) {
				window.close();
				return 0;
			}
		}

		// ==================================================================================================
		// Calculating the simulation with fixed or variable time delta depending on the need of precision
		// ==================================================================================================

		if (config.getAppMode() == ApplicationMode::SIMULATION_MODE) {
			data.setVehicleSpeed(stepCounter / SIMULATION_SECOND_STEP_AMOUNT, vehicle);
			vehicle.recalculate(SIMULATION_FIXED_STEP);
			vehicle.printData(SIMULATION_FIXED_STEP * TIME_uS, stepCounter / SIMULATION_SECOND_STEP_AMOUNT);
			stepCounter++;
		}
		else if (config.getAppMode() == ApplicationMode::GAME_MODE) {
			auto end_time = std::chrono::high_resolution_clock::now(); // get current time again
			auto abso_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - abso_timer); // calculate time difference
			auto calc_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - calc_timer); // calculate time difference

			// Calculate data
			if (calc_duration.count() >= 100) { // check if 10micro/s has elapsed
				vehicle.recalculate(calc_duration.count() * TIME_uS);

				calc_timer = std::chrono::high_resolution_clock::now(); // reset start time
				vehicle.printData(calc_duration.count() * TIME_uS, abso_duration.count() * TIME_mS);
			}
		}

		grid.checkRecalculate(sf::Vector2f(vehicle.getX(), -vehicle.getY()), window.getSize());
		rulers.recalculate(sf::Vector2f(vehicle.getX(), -vehicle.getY()), window.getSize(), panel.getSize());

		// ==================================================================================================
		// Drawing of the application
		// ==================================================================================================

		window.clear(config.getColBackground());

		simulationView.setCenter(vehicle.getX() * DEFAULT_SCALE, -vehicle.getY() * DEFAULT_SCALE);
		window.setView(simulationView);
		grid.draw(window);
		rulers.draw(window);
		vehicle.draw(window);

		window.setView(window.getDefaultView());
		panel.draw(window);

		window.display();

		// =======================================================================================
	}
	return 0;
}