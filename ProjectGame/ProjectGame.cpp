#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <stdexcept>


using namespace sf;

// Helper functions for common operations
class Helper {
public:
    // Converts an integer to a string
    static std::string toString(int value) {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
};

// Enum to manage the current game state
enum GameState {
    MENU,
    DIFFICULTY,  // Màn hình chọn độ khó
    PLAY,
    GAME_OVER
};

// Player class to manage player properties and movements
class Player {
public:
    Sprite sprite;
    Texture texture;
    int lives;

    Player() {
        if (!texture.loadFromFile("ship.png")) {
            throw std::runtime_error("Failed to load ship.png");
        }
        sprite.setTexture(texture);
        sprite.setPosition(640.f, 700.f); // Start position for the player
        lives = 3; // Player starts with 3 lives
    }

    void moveLeft() {
        if (sprite.getPosition().x > 0)
            sprite.move(-5.f, 0.f);
    }

    void moveRight() {
        if (sprite.getPosition().x < 1200.f - sprite.getGlobalBounds().width)
            sprite.move(5.f, 0.f);
    }

    void moveUp() {
        if (sprite.getPosition().y > 0)
            sprite.move(0.f, -5.f);
    }

    void moveDown() {
        if (sprite.getPosition().y < 800.f - sprite.getGlobalBounds().height)
            sprite.move(0.f, 5.f);
    }
};

// Bullet class for player projectiles
class Bullet {
public:
    Sprite sprite;
    Texture texture;

    Bullet(float x, float y) {
        if (!texture.loadFromFile("bullet.png")) {
            throw std::runtime_error("Failed to load bullet.png");
        }
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
    }

    void move() {
        sprite.move(0.f, -10.f);
    }

    bool isOffScreen() const {
        return sprite.getPosition().y < -10.f;
    }
};

// Base class for chickens (enemies)
class Chicken {
public:
    sf::Sprite sprite;

    Chicken(float x, float y) {
        sprite.setPosition(x, y); // Đặt vị trí cho sprite
    }

    virtual void move() {
        sprite.move(0.f, 1.f); // Di chuyển sprite
    }
};

class Chicken2 : public Chicken {
public:
    static sf::Texture texture2; // Biến tĩnh (chỉ nạp 1 lần cho tất cả Chicken2)

    Chicken2(float x, float y) : Chicken(x, y) {
        if (!texture2.loadFromFile("chicken2.png")) {
            throw std::runtime_error("Failed to load chicken2.png");
        }
        sprite.setTexture(texture2);
        sprite.setPosition(x, y); // Đặt vị trí ban đầu
    }

    void move() override {
        sprite.move(1.5f, 2.f);
    }
};

// Khởi tạo biến tĩnh (bắt buộc phải làm thế này)
sf::Texture Chicken2::texture2;

int main() {
    sf::RenderWindow window(sf::VideoMode(1200, 800), "Chicken Game");

    try {
        Chicken2 chicken(100.f, 100.f); // Khởi tạo đối tượng Chicken2

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            chicken.move();

            window.clear();
            window.draw(chicken.sprite);
            window.display();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}




class Boss : public Chicken {
public:
    int health;
    Texture bossTexture;

    Boss(float x, float y) : Chicken(x, y), health(100) {
        if (!bossTexture.loadFromFile("boss.png")) {
            throw std::runtime_error("Failed to load boss.png");
        }
        sprite.setTexture(bossTexture);
    }

    void move() override {
        sprite.move(0.f, 2.f);
    }

    void spawnChickens(std::vector<Chicken*>& chickens) {
        if (rand() % 50 == 0) {
            if (rand() % 2 == 0) {
                chickens.push_back(new Chicken(rand() % 1100, -50.f));
            }
            else {
                chickens.push_back(new Chicken2(rand() % 1100, -50.f));
            }
        }
    }

    void takeDamage() {
        health -= 10;
    }

    bool isDead() const {
        return health <= 0;
    }
};

class Obstacle {
public:
    Sprite sprite;
    Texture texture;

    Obstacle(float x, float y) {
        if (!texture.loadFromFile("obstacle.png")) {
            throw std::runtime_error("Failed to load obstacle.png");
        }
        sprite.setTexture(texture);
        sprite.setPosition(x, y);
    }

    // Phương thức move không có tham số
    void move() {
        sprite.move(0.f, 5.f); // Tốc độ cố định hoặc có thể thay đổi nếu muốn
    }

    bool isOffScreen() const {
        return sprite.getPosition().y > 800.f;
    }
};


class Game {
private:
    RenderWindow window;
    GameState gameState;
    Player player;
    int difficulty; // 0: Dễ, 1: Vừa, 2: Khó (Mặc định là 1 - Vừa)
    float chickenSpeedMultiplier; // Tốc độ của gà
    float obstacleSpeedMultiplier; // Tốc độ của chướng ngại vật
    int playerLives; // Số mạng của người chơi
    std::vector<Bullet> bullets;
    std::vector<Chicken*> chickens;
    std::vector<Obstacle> obstacles;
    Texture backgroundTexture;
    Sprite background;
    Font font;
    Text titleText, playText, instructionsText, exitText;
    Text scoreText, livesText, gameOverText, retryText;
    Text finalScoreText;
    Text easyText, mediumText, hardText;
    Clock bulletClock, chickenSpawnClock, obstacleSpawnClock;
    int score;
    Boss* boss;

public:
    Game() : window(VideoMode(1200, 800), "Chicken Shooter", Style::Close), boss(nullptr) {
        window.setFramerateLimit(60);
        gameState = MENU;
        score = 0;

        if (!backgroundTexture.loadFromFile("background.png")) {
            throw std::runtime_error("Failed to load background.png");
        }
        background.setTexture(backgroundTexture);

        if (!font.loadFromFile("arial.ttf")) {
            throw std::runtime_error("Failed to load arial.ttf");
        }

        titleText.setFont(font);
        titleText.setString("Chicken Shooter");
        titleText.setCharacterSize(50);
        titleText.setPosition(400.f, 100.f);

        playText.setFont(font);
        playText.setString("1. Play Game");
        playText.setCharacterSize(30);
        playText.setPosition(540.f, 300.f);

        instructionsText.setFont(font);
        instructionsText.setString("2. Instructions");
        instructionsText.setCharacterSize(30);
        instructionsText.setPosition(540.f, 350.f);

        exitText.setFont(font);
        exitText.setString("3. Exit");
        exitText.setCharacterSize(30);
        exitText.setPosition(540.f, 400.f);

        easyText.setFont(font);
        easyText.setString("4. Easy");
        easyText.setCharacterSize(30);
        easyText.setPosition(540.f, 450.f);

        mediumText.setFont(font);
        mediumText.setString("5. Medium");
        mediumText.setCharacterSize(30);
        mediumText.setPosition(540.f, 500.f);

        hardText.setFont(font);
        hardText.setString("6. Hard");
        hardText.setCharacterSize(30);
        hardText.setPosition(540.f, 550.f);

        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setPosition(10.f, 10.f);

        livesText.setFont(font);
        livesText.setCharacterSize(20);
        livesText.setPosition(10.f, 40.f);

        gameOverText.setFont(font);
        gameOverText.setString("Game Over");
        gameOverText.setCharacterSize(50);
        gameOverText.setPosition(450.f, 300.f);

        retryText.setFont(font);
        retryText.setString("Press R to Retry or M for Menu");
        retryText.setCharacterSize(20);
        retryText.setPosition(450.f, 400.f);

        finalScoreText.setFont(font);
        finalScoreText.setCharacterSize(30);
        finalScoreText.setPosition(500.f, 350.f);
        finalScoreText.setFillColor(Color::White);

        difficulty = 1; // Mặc định là "Medium"
        setDifficulty(difficulty);  // Cài đặt độ khó mặc định
    }

    void setDifficulty(int level) {
        if (level == 0) { // Dễ
            chickenSpeedMultiplier = 0.8f;  // Gà di chuyển chậm hơn
            obstacleSpeedMultiplier = 0.8f; // Chướng ngại vật di chuyển chậm hơn
            playerLives = 5; // Người chơi có 5 mạng
        }
        else if (level == 1) { // Vừa
            chickenSpeedMultiplier = 1.0f;  // Tốc độ gốc của gà
            obstacleSpeedMultiplier = 1.0f; // Tốc độ gốc của chướng ngại vật
            playerLives = 3; // Người chơi có 3 mạng
        }
        else if (level == 2) { // Khó
            chickenSpeedMultiplier = 1.5f;  // Gà di chuyển nhanh hơn
            obstacleSpeedMultiplier = 1.5f; // Chướng ngại vật di chuyển nhanh hơn
            playerLives = 2; // Người chơi chỉ có 2 mạng
        }
    }


    void run() {
        while (window.isOpen()) {
            handleEvents();
            update();
            render();
        }
    }

private:
    void handleEvents() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();
        }

        if (gameState == MENU) {
            if (Keyboard::isKeyPressed(Keyboard::Num1)) {  // Nhấn 1 để vào màn hình chọn độ khó
                gameState = DIFFICULTY;  // Chuyển sang màn hình chọn độ khó
            }
            else if (Keyboard::isKeyPressed(Keyboard::Num2)) {  // Nhấn 2 để xem hướng dẫn
                instructionsText.setString("Use arrows to move, space to shoot.");
            }
            else if (Keyboard::isKeyPressed(Keyboard::Num3)) {  // Nhấn 3 để thoát
                window.close();
            }
        }
        else if (gameState == DIFFICULTY) {  // Màn hình chọn độ khó
            if (Keyboard::isKeyPressed(Keyboard::Num4)) {  // Chọn Dễ
                difficulty = 0;
                setDifficulty(difficulty);
                startGame();  // Bắt đầu game
            }
            else if (Keyboard::isKeyPressed(Keyboard::Num5)) {  // Chọn Vừa
                difficulty = 1;
                setDifficulty(difficulty);
                startGame();  // Bắt đầu game
            }
            else if (Keyboard::isKeyPressed(Keyboard::Num6)) {  // Chọn Khó
                difficulty = 2;
                setDifficulty(difficulty);
                startGame();  // Bắt đầu game
            }
        }

        if (gameState == PLAY) {
            if (Keyboard::isKeyPressed(Keyboard::Left))
                player.moveLeft();
            if (Keyboard::isKeyPressed(Keyboard::Right))
                player.moveRight();
            if (Keyboard::isKeyPressed(Keyboard::Up))
                player.moveUp();
            if (Keyboard::isKeyPressed(Keyboard::Down))
                player.moveDown();
            if (Keyboard::isKeyPressed(Keyboard::Space)) {
                if (bulletClock.getElapsedTime().asMilliseconds() > 250) {
                    bullets.emplace_back(player.sprite.getPosition().x + player.sprite.getGlobalBounds().width / 2 - 5,
                        player.sprite.getPosition().y);
                    bulletClock.restart();
                }
            }
        }

        if (gameState == GAME_OVER) {
            if (Keyboard::isKeyPressed(Keyboard::R)) {
                startGame();
            }
            else if (Keyboard::isKeyPressed(Keyboard::M)) {
                gameState = MENU;
            }
        }
    }



    void update() {
        if (gameState == PLAY) {
            updateBullets();
            updateChickens();
            updateObstacles();
            updateCollisions();
            updateScore();

            if (player.lives <= 0) {
                gameState = GAME_OVER;
                finalScoreText.setString("Your Score: " + Helper::toString(score));
            }
        }
    }

    void render() {
        window.clear();
        if (gameState == MENU) {
            window.draw(background);
            window.draw(titleText);
            window.draw(playText);
            window.draw(instructionsText);
            window.draw(exitText);
        }
        else if (gameState == DIFFICULTY) {  // Màn hình chọn độ khó
            window.draw(background);
            window.draw(titleText);
            window.draw(easyText);   // Hiển thị lựa chọn Dễ
            window.draw(mediumText); // Hiển thị lựa chọn Vừa
            window.draw(hardText);   // Hiển thị lựa chọn Khó
        }
        else if (gameState == PLAY) {
            window.draw(background);
            window.draw(player.sprite);
            for (const auto& bullet : bullets) {
                window.draw(bullet.sprite);
            }
            for (const auto* chicken : chickens) {
                window.draw(chicken->sprite);
            }
            for (const auto& obstacle : obstacles) {
                window.draw(obstacle.sprite);
            }
            window.draw(scoreText);
            window.draw(livesText);
        }
        else if (gameState == GAME_OVER) {
            window.draw(gameOverText);
            window.draw(finalScoreText);
            window.draw(retryText);
        }
        window.display();
    }



    void startGame() {
        bullets.clear();
        chickens.clear();
        obstacles.clear();
        player.lives = playerLives;  // Sử dụng số mạng tùy theo độ khó
        score = 0;
        if (boss) {
            delete boss;
            boss = nullptr;
        }
        gameState = PLAY;
    }

    void updateBullets() {
        for (auto& bullet : bullets) {
            bullet.move();
        }
        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
            [](const Bullet& b) { return b.isOffScreen(); }),
            bullets.end());
    }

    void updateChickens() {
        if (chickenSpawnClock.getElapsedTime().asMilliseconds() > 1000) {
            chickens.push_back(new Chicken(rand() % 1100, -50.f));
            chickenSpawnClock.restart();
        }
        for (auto* chicken : chickens) {
            chicken->move();
        }
        chickens.erase(std::remove_if(chickens.begin(), chickens.end(),
            chickens.end()));
    }

    void updateObstacles() {
        if (obstacleSpawnClock.getElapsedTime().asMilliseconds() > 2000) {
            obstacles.emplace_back(rand() % 1100, -50.f);
            obstacleSpawnClock.restart();
        }
        for (auto& obstacle : obstacles) {
            obstacle.move();
        }
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
            [](const Obstacle& o) { return o.isOffScreen(); }),
            obstacles.end());
    }

    void updateCollisions() {
        // Xử lý va chạm với gà
        for (auto& bullet : bullets) {
            for (auto it = chickens.begin(); it != chickens.end(); ++it) {
                if (bullet.sprite.getGlobalBounds().intersects((*it)->sprite.getGlobalBounds())) {
                    score += 10;
                    delete* it;
                    chickens.erase(it);
                    break;
                }
            }
        }

        // Xử lý va chạm với chướng ngại vật
        for (auto& bullet : bullets) {
            for (auto& obstacle : obstacles) {
                if (bullet.sprite.getGlobalBounds().intersects(obstacle.sprite.getGlobalBounds())) {
                    bullet.sprite.setPosition(-100.f, -100.f);  // Xóa viên đạn
                    break;
                }
            }
        }

        // Xử lý va chạm với người chơi
        for (auto& obstacle : obstacles) {
            if (player.sprite.getGlobalBounds().intersects(obstacle.sprite.getGlobalBounds())) {
                player.lives--;
                obstacle.sprite.setPosition(-100.f, -100.f); // Xóa chướng ngại vật sau va chạm
            }
        }
    }

    void updateScore() {
        scoreText.setString("Score: " + Helper::toString(score));
        livesText.setString("Lives: " + Helper::toString(player.lives));
    }
};
