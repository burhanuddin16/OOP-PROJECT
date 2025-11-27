#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <array>
#include <map>
#include <string>
#include <random>
#include <ctime>
#include <fstream>
#include <optional>
#include <vector>
#include <algorithm>
#include <cstdio>
using namespace std ;
random_device rd ;
mt19937 gen(rd()) ;
class Spaceship ;
class ShipUpgrade 
{
public:
    virtual void apply(Spaceship& ship) = 0 ;
    virtual ~ShipUpgrade() = default ;
} ;
class ShieldUpgrade : public ShipUpgrade 
{
private:
    int shieldBonus ;
public:
    ShieldUpgrade(int bonus) : shieldBonus(bonus) {}
    void apply(Spaceship& ship) override ;
} ;
class EngineUpgrade : public ShipUpgrade 
{
private:
    float speedBonus ;
public:
    EngineUpgrade(float bonus) : speedBonus(bonus) {}
    void apply(Spaceship& ship) override ;
} ;
class CargoUpgrade : public ShipUpgrade 
{
private:
    int capacityBonus ;
public:
    CargoUpgrade(int bonus) : capacityBonus(bonus) {}
    void apply(Spaceship& ship) override ;
} ;
enum class ShipType 
{
    FastCourier,
    HeavyFreighter,
    BalancedTrader
} ;
class Event 
{
public:
    virtual void trigger(Spaceship& player) = 0 ;
    virtual ~Event() = default ;
} ;
enum class AsteroidSize 
{
    Tiny,
    Small,
    Medium,
    Large
} ;
class Asteroid 
{
private:
    sf::Sprite sprite ;
    sf::Vector2f velocity ;
    float rotationSpeed ;
    AsteroidSize size ;
    int damage ;
public:
    Asteroid(float x, float y, AsteroidSize asteroidSize, const sf::Texture& texture) : sprite(texture), size(asteroidSize) 
    {
        sprite.setPosition(sf::Vector2f(x, y)) ;
        switch (size) 
        {
            case AsteroidSize::Tiny:
                sprite.setScale(sf::Vector2f(0.2f, 0.2f)) ;
                damage = 20 ;
                break ;
            case AsteroidSize::Small:
                sprite.setScale(sf::Vector2f(0.4f, 0.4f)) ;
                damage = 40 ;
                break ;
            case AsteroidSize::Medium:
                sprite.setScale(sf::Vector2f(0.6f, 0.6f)) ;
                damage = 60 ;
                break ;
            case AsteroidSize::Large:
                sprite.setScale(sf::Vector2f(0.8f, 0.8f)) ;
                damage = 100 ;
                break ;
        }        
        sprite.setOrigin(sf::Vector2f(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f)) ;        
        float centerX = 400.f ;
        float centerY = 300.f ;
        float dx = centerX - x ;
        float dy = centerY - y ;
        float angle = atan2(dy, dx) ;        
        uniform_real_distribution<float> angleVariation(-0.5f, 0.5f) ;
        angle += angleVariation(gen) ;
        float speed = 150.0f - static_cast<float>(static_cast<int>(size) * 20.0f) ;
        velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed) ;        
        uniform_int_distribution<> rotationDist(-50, 50) ;
        rotationSpeed = rotationDist(gen) * (1.0f - static_cast<float>(static_cast<int>(size) * 0.2f)) ;
    }
    void update(float deltaTime) 
    {
        sprite.move(velocity * deltaTime) ;        
        float rotationSpeed = 50.0f - (static_cast<float>(static_cast<int>(size)) * 10.0f) ;
        sprite.rotate(sf::degrees(rotationSpeed * deltaTime)) ;        
        sf::Vector2f pos = sprite.getPosition() ;
        sf::Vector2f textureSize = sf::Vector2f(sprite.getTexture().getSize()) ;
        sf::Vector2f scale = sprite.getScale() ;
        sf::Vector2f size(textureSize.x * scale.x, textureSize.y * scale.y) ;        
        if (pos.x < -size.x) 
        {
            pos.x = 800 + size.x ;
        }
        if (pos.x > 800 + size.x) 
        {
            pos.x = -size.x ;
        }
        if (pos.y < -size.y) 
        {
            pos.y = 600 + size.y ;
        }
        if (pos.y > 600 + size.y) 
        {
            pos.y = -size.y ;
        }
        sprite.setPosition(pos) ;
    }
    void draw(sf::RenderWindow& window) const 
    {
        window.draw(sprite) ;
    }
    bool checkCollision(const sf::Sprite& other) const 
    {
        return sprite.getGlobalBounds().contains(other.getPosition()) ;
    }
    int getDamage() const 
    { 
        return damage ; 
    }
    AsteroidSize getSize() const 
    { 
        return size ; 
    }
} ;
class Planet 
{
private:
    sf::Sprite sprite ;
    sf::Vector2f position ;
    string name ;
    map<string, int> prices ;
    map<string, int> supply ;
    static sf::Font defaultFont ;
    float reputation ;
    sf::Texture texture ;
    static sf::Texture defaultTexture ;

public:
    Planet() : sprite(defaultTexture), name(""), position(sf::Vector2f(0, 0)), reputation(0.0f) 
    {
        sprite.setPosition(position) ;
        sprite.setOrigin(sf::Vector2f(defaultTexture.getSize().x / 2.0f, defaultTexture.getSize().y / 2.0f)) ;
        prices["Space Ore"] = 100 ;
        prices["Alien Fruit"] = 150 ;
        prices["Crystal"] = 200 ;
        supply["Space Ore"] = 100 ;
        supply["Alien Fruit"] = 100 ;
        supply["Crystal"] = 100 ;
    }
    Planet(const string& planetName, const sf::Vector2f& pos, const sf::Texture& planetTexture, const sf::Font& font) : sprite(planetTexture), name(planetName), position(pos), reputation(0.0f), texture(planetTexture) 
    {
        sprite.setPosition(pos) ;
        sprite.setOrigin(sf::Vector2f(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f)) ;
        sprite.setScale(sf::Vector2f(0.2f, 0.2f)) ;
        prices["Space Ore"] = 100 ;
        prices["Alien Fruit"] = 150 ;
        prices["Crystal"] = 200 ;
        supply["Space Ore"] = 100 ;
        supply["Alien Fruit"] = 100 ;
        supply["Crystal"] = 100 ;
    }
    static void initializeDefaultTexture() 
    {
        sf::Image defaultImage ;
        if (!defaultImage.loadFromFile("assets/default_planet.png")) 
        {
            cerr << "Failed to load default planet image!" << endl ;
            return ;
        }   
        if (!defaultTexture.loadFromImage(defaultImage)) 
        {
            cerr << "Failed to load default texture from image!" << endl ;
        }
    }
    void updatePrices(const string& item, int quantitySold) 
    {
        supply[item] -= quantitySold ;
        float priceMultiplier = 1.0f + (reputation * 0.1f) ;
        prices[item] += static_cast<int>(quantitySold * 2 * priceMultiplier) ;
        if (prices[item] < 10) 
        {
            prices[item] = 10 ;
        }
    }
    void modifyReputation(float amount) 
    {
        reputation += amount ;
        if (reputation > 1.0f) 
        {
            reputation = 1.0f ;
        }
        if (reputation < -1.0f) 
        {
            reputation = -1.0f ;
        }
    }
    float getReputation() const 
    { 
        return reputation ; 
    }
    void draw(sf::RenderWindow& window) const 
    {
        window.draw(sprite) ;
    }
    int getPrice(const string& item) const 
    {
        auto it = prices.find(item) ;
        return it != prices.end() ? it->second : 0 ;
    }
    sf::Vector2f getPosition() const 
    { 
        return position ; 
    }
} ;
sf::Font Planet::defaultFont ;
sf::Texture Planet::defaultTexture ;
class PirateAttack : public Event 
{
public:
    void trigger(Spaceship& player) override ;
} ;
class SolarFlare : public Event 
{
public:
    void trigger(Spaceship& player) override 
    {
    }
};
class Spaceship 
{
private:
    sf::Sprite* sprite ;
    sf::Vector2f velocity ;
    float speed ;
    int health ;
    int shield ;
    int credits ;
    int cargoCapacity ;
    map<string, int> cargo ;
    bool isNearPlanet ;
    Planet* currentPlanet ;
    ShipType type ;
    vector<ShipUpgrade*> upgrades ;
    sf::SoundBuffer engineSoundBuffer ;
    sf::Sound engineSound ;
    sf::SoundBuffer collisionSoundBuffer ;
    sf::Sound collisionSound ;
    float shootCooldown ;
    float shootTimer ;
public:
    Spaceship(ShipType shipType = ShipType::BalancedTrader) 
        : sprite(nullptr), speed(200.0f), health(100), shield(50), credits(1000), 
          cargoCapacity(10), isNearPlanet(false), currentPlanet(nullptr), type(shipType),
          engineSound(engineSoundBuffer), collisionSound(collisionSoundBuffer),
          shootCooldown(0.3f), shootTimer(0.0f) 
    {
        if (!engineSoundBuffer.loadFromFile("assets/engine.wav")) 
        {
            cerr << "Failed to load engine sound!" << endl ;
        } 
        else 
        {
            engineSound.setVolume(50.0f) ;
            engineSound.setLooping(true) ;
        }
        if (!collisionSoundBuffer.loadFromFile("assets/collision.wav")) 
        {
            cerr << "Failed to load collision sound!" << endl ;
        } 
        else 
        {
            collisionSound.setVolume(70.0f) ;
        }
        switch (type) 
        {
            case ShipType::FastCourier:
                speed = 300.0f ;
                cargoCapacity = 5 ;
                break ;
            case ShipType::HeavyFreighter:
                speed = 150.0f ;
                cargoCapacity = 20 ;
                break ;
            case ShipType::BalancedTrader:
                speed = 200.0f ;
                cargoCapacity = 10 ;
                break ;
        }
    }
    ~Spaceship() 
    {
        delete sprite ;
        for (size_t i = 0 ; i < upgrades.size() ; ++i) 
        {
            delete upgrades[i] ;
        }
    }
    void setTexture(const sf::Texture& texture) 
    {
        if (sprite) 
        {
            delete sprite ;
        }
        sprite = new sf::Sprite(texture) ;
        sprite->setScale(sf::Vector2f(0.2f, 0.2f)) ;
        sprite->setOrigin(sf::Vector2f(texture.getSize().x / 2.0f, texture.getSize().y / 2.0f)) ;
        sprite->setPosition(sf::Vector2f(400.f, 300.f)) ;
    }
    void move(const sf::Time& deltaTime) 
    {
        bool isMoving = false ;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) 
        { 
            velocity.y = -1 ; 
            isMoving = true ; 
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) 
        { 
            velocity.y = 1 ; 
            isMoving = true ; 
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) 
        { 
            velocity.x = -1 ; 
            isMoving = true ; 
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) 
        { 
            velocity.x = 1 ; 
            isMoving = true ; 
        }
        if (velocity.x != 0 && velocity.y != 0) 
        {
            velocity.x *= 0.7071f ;
            velocity.y *= 0.7071f ;
        }
        if (sprite) 
        {
            sprite->move(velocity * speed * deltaTime.asSeconds()) ;
        }
        if (isMoving) 
        {
            if (engineSound.getStatus() != sf::SoundSource::Status::Playing) 
            {
                engineSound.play() ;
            }
        } 
        else 
        {
            engineSound.stop() ;
        }
        velocity = sf::Vector2f(0, 0) ;
    }
    void takeDamage(int damage) 
    {
        if (shield > 0) 
        {
            shield -= damage ;
            if (shield < 0) 
            {
                health += shield ;
                shield = 0 ;
            }
        } 
        else 
        {
            health -= damage ;
        }        
        if (health < 0) 
        {
            health = 0 ;
        }
        collisionSound.play() ;
    }
    bool buy(const string& item, int quantity) 
    {
        if (currentPlanet && isNearPlanet) 
        {
            int totalCost = currentPlanet->getPrice(item) * quantity ;
            if (credits >= totalCost && getCargoCount() + quantity <= cargoCapacity) 
            {
                credits -= totalCost ;
                cargo[item] += quantity ;
                currentPlanet->updatePrices(item, quantity) ;
                return true ;
            }
        }
        return false ;
    }
    bool sell(const string& item, int quantity) 
    {
        if (currentPlanet && isNearPlanet && cargo[item] >= quantity) 
        {
            credits += currentPlanet->getPrice(item) * quantity ;
            cargo[item] -= quantity ;
            return true ;
        }
        return false ;
    }
    void checkPlanetProximity(const array<Planet, 2>& planets) 
    {
        isNearPlanet = false ;
        currentPlanet = nullptr ;   
        for (unsigned int i = 0 ; i < planets.size() ; ++i) 
        {
            const Planet& planet = planets.at(i) ;
            float dx = getPosition().x - planet.getPosition().x ;
            float dy = getPosition().y - planet.getPosition().y ;
            float distance = sqrt(dx * dx + dy * dy) ;       
            if (distance < 100.0f) 
            {
                isNearPlanet = true ;
                currentPlanet = const_cast<Planet*>(&planet) ;
                break ;
            }
        }
    }
    void draw(sf::RenderWindow& window) const 
    {
        if (sprite) 
        {
            window.draw(*sprite) ;
        }
    }
    sf::Vector2f getPosition() const 
    { 
        return sprite ? sprite->getPosition() : sf::Vector2f(0, 0) ;
    }
    int getCredits() const 
    { 
        return credits ; 
    }
    void addCredits(int amount) 
    { 
        credits += amount ; 
    }
    void deductCredits(int amount) 
    { 
        credits -= amount ; 
    }
    int getCargoCount() const 
    {
        int count = 0 ;
        for (const auto& item : cargo) 
        {
            count += item.second ;
        }
        return count ;
    }
    bool isNearAnyPlanet() const 
    { 
        return isNearPlanet ; 
    }
    Planet* getCurrentPlanet() const 
    { 
        return currentPlanet ; 
    }
    sf::Sprite* getSprite() const 
    { 
        return sprite ; 
    }
    ShipType getType() const 
    { 
        return type ; 
    }
    void addUpgrade(ShipUpgrade* upgrade) 
    {
        upgrades.emplace_back(upgrade) ;
        upgrade->apply(*this) ;
    }
    void increaseShield(int amount) 
    { 
        shield += amount ; 
    }
    void increaseSpeed(float amount) 
    { 
        speed += amount ; 
    }
    void increaseCargoCapacity(int amount) 
    { 
        cargoCapacity += amount ; 
    }
    int getHealth() const 
    { 
        return health ; 
    }
    int getShield() const 
    { 
        return shield ; 
    }
    int getCargoCount(const string& item) const 
    {
        auto it = cargo.find(item) ;
        return it != cargo.end() ? it->second : 0 ;
    }
} ;
void PirateAttack::trigger(Spaceship& player) 
{
    if (player.getCredits() >= 100) 
    {
        player.deductCredits(100) ;
    }
}
class Game 
{
private:
    sf::RenderWindow window ;
    sf::Texture shipTexture ;
    sf::Font font ;
    Spaceship player ;
    array<Planet, 2> planets ;
    array<Event*, 2> events ;
    array<Asteroid*, 12> asteroids ;
    array<bool, 12> activeAsteroids ;
    sf::Text creditsText ;
    sf::Text healthText ;
    sf::Text shieldText ;
    sf::Text tradingText ;
    sf::Text shipTypeText ;
    bool showTradingMenu ;
    bool isUpgradeMenuOpen ;
    sf::Music backgroundMusic ;
    sf::Text upgradeMenuText ;
    bool isGameOver ;
    sf::Text gameOverText ;
    sf::Texture tinyAsteroidTexture ;
    sf::Texture smallAsteroidTexture ;
    sf::Texture mediumAsteroidTexture ;
    sf::Texture largeAsteroidTexture ;
    sf::Texture earthTexture ;
    sf::Texture marsTexture ;
    bool showResourceMenu ;
    sf::Text resourceMenuText ;
    bool isStartMenu ;
    sf::Text titleText ;
    sf::Text startText ;
    sf::RectangleShape startButton ;
    sf::Music menuMusic ;
    int currentScore ;
    sf::Text scoreText ;
    sf::Text highScoreText ;
    sf::RectangleShape scoreButton ;
    sf::Text scoreButtonText ;
    bool showScoreScreen ;
    vector<int> scores ;
    sf::Clock gameClock ;
    sf::Text viewScoresInstruction ;
public:
    Game() : window(sf::VideoMode(sf::Vector2u(800, 600)), "Space Trader"),
             creditsText(font, "", 20),
             healthText(font, "", 20),
             shieldText(font, "", 20),
             tradingText(font, "", 20),
             shipTypeText(font, "", 20),
             upgradeMenuText(font, "", 20),
             gameOverText(font, "", 50),
             resourceMenuText(font, "", 20),
             titleText(font, "SPACE TRADER", 80),
             startText(font, "START GAME", 30),
             scoreText(font, "", 20),
             highScoreText(font, "", 20),
             scoreButtonText(font, "VIEW SCORES", 20),
             viewScoresInstruction(font, "Press S to view scores", 22),
             showTradingMenu(false),
             isUpgradeMenuOpen(false),
             isGameOver(false),
             showResourceMenu(false),
             isStartMenu(true),
             currentScore(0),
             showScoreScreen(false) 
    {
        window.setFramerateLimit(60) ;
        if (!shipTexture.loadFromFile("assets/spaceship.png")) 
        {
            cerr << "Failed to load spaceship texture!" << endl ;
        }
        if (!font.openFromFile("assets/Arial.ttf")) 
        {
            cerr << "Failed to load font!" << endl ;
        }
        if (!menuMusic.openFromFile("assets/background.wav")) 
        {
            cerr << "Failed to load background music!" << endl ;
        }
        titleText.setPosition(sf::Vector2f(100.f, 150.f)) ;
        titleText.setFillColor(sf::Color::Cyan) ;
        titleText.setStyle(sf::Text::Bold) ;
        startButton.setSize(sf::Vector2f(250.f, 60.f)) ;
        startButton.setPosition(sf::Vector2f(275.f, 350.f)) ;
        startButton.setFillColor(sf::Color(0, 100, 200)) ;
        startButton.setOutlineThickness(3.f) ;
        startButton.setOutlineColor(sf::Color::White) ; 
        startText.setFont(font) ;
        startText.setString("START GAME") ;
        startText.setCharacterSize(30) ;
        startText.setPosition(sf::Vector2f(310.f, 360.f)) ;
        startText.setFillColor(sf::Color::White) ;              
        viewScoresInstruction.setPosition(sf::Vector2f(285.f, 420.f)) ;
        viewScoresInstruction.setFillColor(sf::Color::White) ;
        menuMusic.setLooping(true) ;
        menuMusic.play() ;
        player.setTexture(shipTexture) ;
        backgroundMusic.setLooping(true) ;
        backgroundMusic.play() ;
        Planet::initializeDefaultTexture() ;
        if (!earthTexture.loadFromFile("assets/earth.png")) 
        {
            cerr << "Failed to load earth texture!" << endl ;
        }
        if (!marsTexture.loadFromFile("assets/mars.png")) 
        {
            cerr << "Failed to load mars texture!" << endl ;
        }
        planets[0] = Planet("Earth", sf::Vector2f(200.f, 200.f), earthTexture, font) ;
        planets[1] = Planet("Mars", sf::Vector2f(600.f, 400.f), marsTexture, font) ;
        events[0] = new PirateAttack() ;
        events[1] = new SolarFlare() ;
        if (!tinyAsteroidTexture.loadFromFile("assets/tiny_asteroid.png")) 
        {
            cerr << "Failed to load tiny asteroid texture!" << endl ;
        }
        if (!smallAsteroidTexture.loadFromFile("assets/small_asteroid.png")) 
        {
            cerr << "Failed to load small asteroid texture!" << endl ;
        }
        if (!mediumAsteroidTexture.loadFromFile("assets/medium_asteroid.png")) 
        {
            cerr << "Failed to load medium asteroid texture!" << endl ;
        }
        if (!largeAsteroidTexture.loadFromFile("assets/large_asteroid.png")) 
        {
            cerr << "Failed to load large asteroid texture!" << endl ;
        }
        uniform_int_distribution<> xDist(0, 800) ;
        uniform_int_distribution<> yDist(0, 600) ;
        uniform_int_distribution<> sizeDist(0, 3) ;
        for (int i = 0 ; i < 12 ; ++i) 
        {
            float x = xDist(gen) ;
            float y = yDist(gen) ;
            AsteroidSize size = static_cast<AsteroidSize>(sizeDist(gen)) ;
            switch (size) 
            {
                case AsteroidSize::Tiny:
                    asteroids[i] = new Asteroid(x, y, size, tinyAsteroidTexture) ;
                    break ;
                case AsteroidSize::Small:
                    asteroids[i] = new Asteroid(x, y, size, smallAsteroidTexture) ;
                    break ;
                case AsteroidSize::Medium:
                    asteroids[i] = new Asteroid(x, y, size, mediumAsteroidTexture) ;
                    break ;
                case AsteroidSize::Large:
                    asteroids[i] = new Asteroid(x, y, size, largeAsteroidTexture) ;
                    break ;
            }
            activeAsteroids[i] = true ;
        }
        creditsText.setPosition(sf::Vector2f(10.f, 10.f)) ;
        healthText.setPosition(sf::Vector2f(10.f, 40.f)) ;
        shieldText.setPosition(sf::Vector2f(10.f, 70.f)) ;
        tradingText.setPosition(sf::Vector2f(300.f, 10.f)) ;
        shipTypeText.setPosition(sf::Vector2f(10.f, 100.f)) ;
        upgradeMenuText.setPosition(sf::Vector2f(300.f, 50.f)) ;
        resourceMenuText.setPosition(sf::Vector2f(400.f, 50.f)) ;
        gameOverText.setPosition(sf::Vector2f(200.f, 250.f)) ;
        gameOverText.setFillColor(sf::Color::Red) ;
        scoreText.setFont(font) ;
        scoreText.setCharacterSize(20) ;
        scoreText.setPosition(sf::Vector2f(10.f, 100.f)) ;
        highScoreText.setFont(font) ;
        highScoreText.setCharacterSize(20) ;
        highScoreText.setPosition(sf::Vector2f(10.f, 120.f)) ;
        scoreButton.setSize(sf::Vector2f(100.f, 40.f)) ;
        scoreButton.setPosition(sf::Vector2f(300.f, 100.f)) ;
        scoreButton.setFillColor(sf::Color(0, 100, 200)) ;
        scoreButton.setOutlineThickness(3.f) ;
        scoreButton.setOutlineColor(sf::Color::White) ;
        scoreButtonText.setFont(font) ;
        scoreButtonText.setCharacterSize(20) ;
        scoreButtonText.setPosition(sf::Vector2f(310.f, 105.f)) ;
        scoreButtonText.setFillColor(sf::Color::White) ;
    }
    ~Game() 
    {
        for (int i = 0 ; i < 2 ; ++i) 
        {
            delete events[i] ;
        }
        for (int i = 0 ; i < 12 ; ++i) 
        {
            delete asteroids[i] ;
        }
    }
    void run() 
    {
        sf::Clock clock ;
        while (window.isOpen()) 
        {
            sf::Time deltaTime = clock.restart() ;
            handleEvents() ;
            update(deltaTime.asSeconds()) ;
            render() ;
        }
    }
private:
    void handleEvents() 
    {
        while (auto event = window.pollEvent()) 
        {
            if (event->is<sf::Event::Closed>()) 
            {
                if (remove("scores.txt") != 0) 
                {
                    cerr << "Error deleting scores file" << endl ;
                }
                cout << "Scores file deleted on game close" << endl ;
                window.close() ;
            }
            else if (event->is<sf::Event::KeyPressed>()) 
            {
                if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) 
                {
                    if (keyEvent->scancode == sf::Keyboard::Scan::Escape) 
                    {
                        if (showTradingMenu) 
                        {
                            showTradingMenu = false ;
                        }
                        else if (isUpgradeMenuOpen) 
                        {
                            isUpgradeMenuOpen = false ;
                        }
                        else if (showResourceMenu) 
                        {
                            showResourceMenu = false ;
                        }
                        else if (showScoreScreen) 
                        {
                            showScoreScreen = false ;
                            isStartMenu = true ;
                        }
                        else if (isGameOver) 
                        {
                            isGameOver = false ;
                            isStartMenu = true ;
                            menuMusic.play() ;
                        }
                        else if (!isStartMenu) 
                        {
                            isStartMenu = true ;
                            menuMusic.play() ;
                            backgroundMusic.stop() ;
                        }
                    }
                    else if (isStartMenu) 
                    {
                        if (keyEvent->scancode == sf::Keyboard::Scan::Enter) 
                        {
                            isStartMenu = false ;
                            showScoreScreen = false ;
                            menuMusic.stop() ;
                            backgroundMusic.play() ;
                            currentScore = 0 ;
                            gameClock.restart() ;
                            cout << "New game started, score reset to 0" << endl ;
                            player = Spaceship() ;
                            player.setTexture(shipTexture) ;
                            isGameOver = false ;
                        }
                        else if (keyEvent->scancode == sf::Keyboard::Scan::S) 
                        {
                            showScoreScreen = true ;
                            isStartMenu = false ;
                            cout << "Score screen opened!" << endl ;
                        }
                    }
                    else if (!isGameOver && !showScoreScreen) 
                    {
                        if (keyEvent->scancode == sf::Keyboard::Scan::T && player.isNearAnyPlanet()) 
                        {
                            showTradingMenu = !showTradingMenu ;
                            cout << "Trading menu toggled: " << showTradingMenu << endl ;
                        }
                        else if (keyEvent->scancode == sf::Keyboard::Scan::U) 
                        {
                            isUpgradeMenuOpen = !isUpgradeMenuOpen ;
                            cout << "Upgrade menu toggled: " << isUpgradeMenuOpen << endl ;
                        }
                        else if (keyEvent->scancode == sf::Keyboard::Scan::R) 
                        {
                            showResourceMenu = !showResourceMenu ;
                            cout << "Resource menu toggled: " << showResourceMenu << endl ;
                        }
                        else if (showTradingMenu) 
                        {
                            handleTradingInput(keyEvent->scancode) ;
                        }
                        else if (isUpgradeMenuOpen) 
                        {
                            handleUpgradeInput(keyEvent->scancode) ;
                        }
                    }
                }
            }
        }
    }
    void handleTradingInput(sf::Keyboard::Scan key) 
    {
        if (!player.getCurrentPlanet()) 
        {
            return ;
        }
        switch (key) 
        {
            case sf::Keyboard::Scan::Num1:
                player.buy("Space Ore", 1) ;
                break ;
            case sf::Keyboard::Scan::Num2:
                player.sell("Space Ore", 1) ;
                break ;
            case sf::Keyboard::Scan::Num3:
                player.buy("Alien Fruit", 1) ;
                break ;
            case sf::Keyboard::Scan::Num4:
                player.sell("Alien Fruit", 1) ;
                break ;
            case sf::Keyboard::Scan::Num5:
                player.buy("Crystal", 1) ;
                break ;
            case sf::Keyboard::Scan::Num6:
                player.sell("Crystal", 1) ;
                break ;
        }
    }
    void handleUpgradeInput(sf::Keyboard::Scan key) 
    {
        switch (key) 
        {
            case sf::Keyboard::Scan::Num1:
                player.addUpgrade(new ShieldUpgrade(50)) ;
                cout << "Shield upgraded!" << endl ;
                isUpgradeMenuOpen = false ;
                break ;
            case sf::Keyboard::Scan::Num2:
                player.addUpgrade(new EngineUpgrade(50)) ;
                cout << "Engine upgraded!" << endl ;
                isUpgradeMenuOpen = false ;
                break ;
            case sf::Keyboard::Scan::Num3:
                player.addUpgrade(new CargoUpgrade(5)) ;
                cout << "Cargo capacity upgraded!" << endl ;
                isUpgradeMenuOpen = false ;
                break ;
        }
    }
    void update(float deltaTime) 
    {
        if (!isStartMenu && !showScoreScreen && !isGameOver) 
        {
            player.move(sf::seconds(deltaTime)) ;
            player.checkPlanetProximity(planets) ;
            float elapsedTime = gameClock.getElapsedTime().asSeconds() ;
            currentScore = static_cast<int>(elapsedTime) ;
            if (currentScore > 0) 
            {
                cout << "Score Update:" << endl ;
                cout << "Elapsed Time: " << elapsedTime << " seconds" << endl ;
                cout << "Current Score: " << currentScore << endl ;
            }
            if (player.getHealth() <= 0) 
            {
                isGameOver = true ;
                gameOverText.setString("GAME OVER\nScore: " + to_string(currentScore) + "\nPress ESC to exit") ;
                ofstream scoreFile("scores.txt", ios::app) ;
                if (scoreFile.is_open()) 
                {
                    scoreFile << currentScore << "\n" ;
                    scoreFile.close() ;
                    cout << "Final Score Saved: " << currentScore << endl ;
                }
            }
            for (int i = 0 ; i < 12 ; ++i) 
            {
                if (activeAsteroids[i]) 
                {
                    asteroids[i]->update(deltaTime) ;       
                    if (asteroids[i]->checkCollision(*player.getSprite())) 
                    {
                        int damage = asteroids[i]->getDamage() ;
                        player.takeDamage(damage) ;
                        activeAsteroids[i] = false ;
                        if (asteroids[i]->getSize() == AsteroidSize::Large) 
                        {
                            isGameOver = true ;
                            gameOverText.setString("GAME OVER\nScore: " + to_string(currentScore) + "\nPress ESC to exit") ;
                            ofstream scoreFile("scores.txt", ios::app) ;
                            if (scoreFile.is_open()) 
                            {
                                scoreFile << currentScore << "\n" ;
                                scoreFile.close() ;
                                cout << "Final Score Saved: " << currentScore << endl ;
                            }
                        }
                    }
                }
            }
            creditsText.setString("Credits: " + to_string(player.getCredits())) ;
            healthText.setString("Health: " + to_string(player.getHealth())) ;
            shieldText.setString("Shield: " + to_string(player.getShield())) ;
            shipTypeText.setString("Ship Type: " + getShipTypeString(player.getType())) ;
            if (player.isNearAnyPlanet()) 
            {
                tradingText.setString("Press T to trade") ;
            } 
            else 
            {
                tradingText.setString("") ;
                showTradingMenu = false ;
            }
        }
    }
    string getShipTypeString(ShipType type) 
    {
        switch (type) 
        {
            case ShipType::FastCourier: 
                return "Fast Courier" ;
            case ShipType::HeavyFreighter: 
                return "Heavy Freighter" ;
            case ShipType::BalancedTrader: 
                return "Balanced Trader" ;
            default: 
                return "Unknown" ;
        }
    }
    void render() 
    {
        window.clear(sf::Color(0, 0, 0)) ;
        if (isStartMenu) 
        {
            uniform_int_distribution<> xDist(0, 800) ;
            uniform_int_distribution<> yDist(0, 600) ;
            for (int i = 0 ; i < 200 ; ++i) 
            {
                sf::CircleShape star(1) ;
                star.setPosition(sf::Vector2f(xDist(gen), yDist(gen))) ;
                star.setFillColor(sf::Color::White) ;
                window.draw(star) ;
            }
            window.draw(titleText) ;
            window.draw(startButton) ;
            window.draw(startText) ;
            window.draw(viewScoresInstruction) ;
        }
        else if (showScoreScreen) 
        {
            uniform_int_distribution<> xDist(0, 800) ;
            uniform_int_distribution<> yDist(0, 600) ;
            for (int i = 0 ; i < 200 ; ++i) 
            {
                sf::CircleShape star(1) ;
                star.setPosition(sf::Vector2f(xDist(gen), yDist(gen))) ;
                star.setFillColor(sf::Color::White) ;
                window.draw(star) ;
            }
            sf::Text scoreTitle(font, "High Scores", 50) ;
            scoreTitle.setPosition(sf::Vector2f(250.f, 150.f)) ;
            scoreTitle.setFillColor(sf::Color::White) ;
            window.draw(scoreTitle) ;
            ifstream scoreFile("scores.txt", ios::in) ;
            if (scoreFile.is_open()) 
            {
                cout << "Score file opened successfully" << endl ;
                int score ;
                int yPos = 250 ;
                int rank = 1 ;
                vector<int> allScores ;
                while (scoreFile >> score) 
                {
                    allScores.push_back(score) ;
                }
                scoreFile.close() ;
                sort(allScores.begin(), allScores.end(), greater<int>()) ;
                const size_t maxScores = 10 ;
                const size_t numScores = min(allScores.size(), maxScores) ;
                for (size_t i = 0 ; i < numScores ; ++i) 
                {
                    string scoreText = to_string(rank) + ". " + to_string(allScores[i]) ;
                    sf::Text scoreDisplay(font, scoreText, 30) ;
                    scoreDisplay.setPosition(sf::Vector2f(300.f, yPos)) ;
                    scoreDisplay.setFillColor(sf::Color::White) ;
                    window.draw(scoreDisplay) ;
                    yPos += 40 ;
                    rank++ ;
                }
            } 
            else 
            {
                cout << "Failed to open score file" << endl ;
                sf::Text noScoresText(font, "No scores yet!", 30) ;
                noScoresText.setPosition(sf::Vector2f(300.f, 250.f)) ;
                noScoresText.setFillColor(sf::Color::White) ;
                window.draw(noScoresText) ;
            }
            sf::Text returnText(font, "Press ESC to return", 20) ;
            returnText.setPosition(sf::Vector2f(300.f, 500.f)) ;
            returnText.setFillColor(sf::Color::White) ;
            window.draw(returnText) ;
        }
        else 
        {
            uniform_int_distribution<> xDist(0, 800) ;
            uniform_int_distribution<> yDist(0, 600) ;
            for (int i = 0 ; i < 100 ; ++i) 
            {
                sf::CircleShape star(1) ;
                star.setPosition(sf::Vector2f(xDist(gen), yDist(gen))) ;
                star.setFillColor(sf::Color::White) ;
                window.draw(star) ;
            }
            for (int i = 0 ; i < 12 ; ++i) 
            {
                if (activeAsteroids[i]) 
                {
                    asteroids[i]->draw(window) ;
                }
            }
            for (unsigned int i = 0 ; i < planets.size() ; ++i) 
            {
                planets[i].draw(window) ;
            }
            if (!isGameOver) 
            {
                player.draw(window) ;
            }
            window.draw(creditsText) ;
            window.draw(healthText) ;
            window.draw(shieldText) ;
            window.draw(tradingText) ;
            window.draw(shipTypeText) ;
            window.draw(scoreText) ;
            if (showTradingMenu && player.getCurrentPlanet()) 
            {
                renderTradingMenu() ;
            }
            if (isUpgradeMenuOpen) 
            {
                renderUpgradeMenu() ;
            }
            if (showResourceMenu) 
            {
                renderResourceMenu() ;
            }
            if (isGameOver) 
            {
                window.draw(gameOverText) ;
            }
        }
        window.display() ;
    }
    void renderTradingMenu() 
    {
        if (!player.getCurrentPlanet()) 
        {
            return ;
        }
        sf::Text menuText(font) ;
        menuText.setCharacterSize(20) ;
        menuText.setPosition(sf::Vector2f(300.f, 50.f)) ;
        menuText.setFillColor(sf::Color::White) ;
        string menu = "Trading Menu:\n" ;
        menu += "1. Buy Space Ore\n" ;
        menu += "2. Sell Space Ore\n" ;
        menu += "3. Buy Alien Fruit\n" ;
        menu += "4. Sell Alien Fruit\n" ;
        menu += "5. Buy Crystal\n" ;
        menu += "6. Sell Crystal\n" ;
        menu += "Press ESC to close" ;        
        menuText.setString(menu) ;
        window.draw(menuText) ;
    }
    void renderUpgradeMenu() 
    {
        sf::Text menuText(font) ;
        menuText.setCharacterSize(20) ;
        menuText.setPosition(sf::Vector2f(300.f, 50.f)) ;
        menuText.setFillColor(sf::Color::White) ;  
        string menu = "Upgrade Menu:\n" ;
        menu += "1. Shield Upgrade (+50 shield)\n" ;
        menu += "2. Engine Upgrade (+50 speed)\n" ;
        menu += "3. Cargo Upgrade (+5 capacity)\n" ;
        menu += "Press ESC to close" ;
        menuText.setString(menu) ;
        window.draw(menuText) ;
    }
    void renderResourceMenu() 
    {
        sf::Text menuText(font) ;
        menuText.setCharacterSize(20) ;
        menuText.setPosition(sf::Vector2f(300.f, 50.f)) ;
        menuText.setFillColor(sf::Color::White) ;   
        string menu = "Resource Menu:\n" ;
        menu += "Space Ore: " + to_string(player.getCargoCount("Space Ore")) + "\n" ;
        menu += "Alien Fruit: " + to_string(player.getCargoCount("Alien Fruit")) + "\n" ;
        menu += "Crystal: " + to_string(player.getCargoCount("Crystal")) + "\n" ;
        menu += "Press ESC to close" ;
        menuText.setString(menu) ;
        window.draw(menuText) ;
    }
} ;
void ShieldUpgrade::apply(Spaceship& ship) 
{
    ship.increaseShield(shieldBonus) ;
}
void EngineUpgrade::apply(Spaceship& ship) 
{
    ship.increaseSpeed(speedBonus) ;
}
void CargoUpgrade::apply(Spaceship& ship) 
{
    ship.increaseCargoCapacity(capacityBonus) ;
}
int main() 
{
    Game game ;
    game.run() ;
    return 0 ;
} 