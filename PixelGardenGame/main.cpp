#include <SFML/Graphics.hpp>
#include <cstdlib>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

enum GridTile {
    TILE_NONE = 0,
    TILE_SAND = 1,
    TILE_MUD = 2,
    TILE_WATER = 3,
};

class Game
{
private:
    sf::RenderWindow& window;
    sf::Shader gridShader;
    sf::Texture gridTexture;
    sf::Sprite gridSprite;

    sf::Clock clock;
    sf::Time elapsedTime;
    unsigned int frames = 0;

    sf::Font font;
       
    // UI
    sf::Text fpsText;
    sf::Text pixelCountText;
    sf::Text brushSizeText;

    GridTile grid[WINDOW_WIDTH][WINDOW_HEIGHT];
public:
    int brushSize = 15;
    GridTile selectedTile = TILE_SAND;

    void setTextSettings(sf::Text& t, int x, int y) {
        t.setFont(font);
        t.setCharacterSize(24);
        t.setFillColor(sf::Color::White);
        t.setPosition(static_cast<float>(x), static_cast<float>(y));
    }

    Game(sf::RenderWindow& win) : window(win) {
        if (!font.loadFromFile("default.ttf"))
        {
            printf("Failed to load default font!\n");
            return;
        }

        setTextSettings(fpsText, 5, 5);
        setTextSettings(pixelCountText, 5, 30);
        setTextSettings(brushSizeText, 5, 65);

        if (!gridShader.loadFromFile("grid_shader.frag", sf::Shader::Fragment))
        {
            printf("Failed to load shader grid_shader.frag\n");
            return;
        }

        gridShader.setUniform("resolution", sf::Glsl::Vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
        gridShader.setUniform("time", clock.getElapsedTime().asMilliseconds());

        gridTexture.create(WINDOW_WIDTH, WINDOW_HEIGHT);
        gridSprite.setTexture(gridTexture);
    }

    void poll(sf::Event event) {
        if (event.type == sf::Event::MouseWheelScrolled) {
            brushSize += static_cast<int>(event.mouseWheelScroll.delta);
            if (brushSize < 0) brushSize = 0;
        }

        // Reload shader on F5 press
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F5) {
            if (!gridShader.loadFromFile("grid_shader.frag", sf::Shader::Fragment))
            {
                printf("Failed to load shader grid_shader.frag\n");
                return;
            }

            gridShader.setUniform("resolution", sf::Glsl::Vec2(WINDOW_WIDTH, WINDOW_HEIGHT));
            gridShader.setUniform("time", clock.getElapsedTime().asMilliseconds());
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Num1)
                selectedTile = TILE_NONE;
            if (event.key.code == sf::Keyboard::Num2)
                selectedTile = TILE_SAND;
            if (event.key.code == sf::Keyboard::Num3)
                selectedTile = TILE_MUD;
            if (event.key.code == sf::Keyboard::Num4)
                selectedTile = TILE_WATER;

        }
    }

    void update() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        if (window.hasFocus()) {
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                for (int x = -brushSize; x < brushSize; x++)
                    for (int y = -brushSize; y < brushSize; y++)
                        setGridTile(mousePos.x + x, mousePos.y + y, selectedTile);
            }
        }

        for (int y = WINDOW_HEIGHT - 2; y >= 0; y--) { // Start from second last row to avoid out-of-bounds
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                if (getGridTile(x, y) != TILE_NONE) {
                    int ux = x;
                    int uy = y;

                    // move down
                    if (getGridTile(ux, uy + 1) == TILE_NONE) {
                        setGridTile(ux, uy + 1, getGridTile(ux, uy));
                        setGridTile(ux, uy, TILE_NONE);
                        uy++;
                    }
                    else {
                        int direction = std::rand() % 2; // 0 for left, 1 for right
                        if (direction == 0 && ux > 0 && getGridTile(ux - 1, uy) == TILE_NONE) {
                            setGridTile(ux - 1, uy, getGridTile(ux, uy));
                            setGridTile(ux, uy, TILE_NONE);
                            ux--;
                        }
                        else if (direction == 1 && ux < WINDOW_WIDTH - 1 && getGridTile(ux + 1, uy) == TILE_NONE) {
                            setGridTile(ux + 1, uy, getGridTile(ux, uy));
                            setGridTile(ux, uy, TILE_NONE);
                            ux++;
                        }
                    }
                }
            }
        }
    }

    GridTile getGridTile(int x, int y) const {
        if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            return grid[x][y];
        }
        else {
            return TILE_NONE;
        }
    }

    void setGridTile(int x, int y, GridTile value) {
        if (x >= 0 && x < WINDOW_WIDTH && y >= 0 && y < WINDOW_HEIGHT) {
            grid[x][y] = value;
        }
    }

    void updateGridTexture() {
        sf::Image image;
        image.create(WINDOW_WIDTH, WINDOW_HEIGHT, sf::Color::Black);

        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                switch (getGridTile(x, y)) {
                case TILE_SAND:
                    image.setPixel(x, y, sf::Color::Red); // Red represents sand
                    break;
                case TILE_MUD:
                    image.setPixel(x, y, sf::Color::Green); // Green represents mud
                    break;
                case TILE_WATER:
                    image.setPixel(x, y, sf::Color::Blue); // Blue represents water
                    break;
                case TILE_NONE:
                default:
                    image.setPixel(x, y, sf::Color::Black); // Black for none
                    break;
                }
            }
        }

        gridTexture.update(image);
    }

    void draw() {
        gridShader.setUniform("time", clock.getElapsedTime().asMilliseconds());
        updateGridTexture();
        window.draw(gridSprite, &gridShader);

        // FPS
        elapsedTime += clock.restart();
        if (elapsedTime.asSeconds() >= 0.05f)
        {
            float fps = static_cast<float>(frames) / elapsedTime.asSeconds();
            fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
            elapsedTime = sf::Time::Zero;
            frames = 0;
        }

        frames++;
        window.draw(fpsText);

        // Pixel Count
        int pixelCount = 0;
        for (int y = 0; y < WINDOW_HEIGHT; y++) {
            for (int x = 0; x < WINDOW_WIDTH; x++) {
                if (getGridTile(x, y) != TILE_NONE) {
                    pixelCount++;
                }
            }
        }

        pixelCountText.setString("Pixel count: " + std::to_string(pixelCount));
        window.draw(pixelCountText);

        // Brush Size
        brushSizeText.setString("Brush size: " + std::to_string(brushSize));
        window.draw(brushSizeText);
    }
};


int main(int argc, char* argv[])
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pixel Garden", sf::Style::Titlebar | sf::Style::Close);

    auto game = new Game(window);
    printf("Pixel Garden v0.0.1-DEBUG\n");
    printf("Context Version: %d.%d\n", window.getSettings().majorVersion, window.getSettings().minorVersion);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            game->poll(event);
        }

        game->update();

        window.clear();
        game->draw();
        window.display();
    }

    return 0;
}