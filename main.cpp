#include <raylib.h>
#include <iostream>
#include <vector>
#include <ctime>
using namespace std;

// Colors
Color Green = Color{38, 185, 154, 255};
Color Dark_Green = Color{20, 160, 133, 255};
Color Light_Green = Color{129, 204, 184, 255};
Color Yellow = Color{243, 213, 91, 255};
Color LightPink = Color{255, 182, 193, 255};
Color blue = Color{0, 0, 255, 255};
// Global scores
int player_score = 0;
int cpu_score = 0;
bool issingle;
bool game_ended = false;
string winner_text = "";
const int MAX_SCORE = 5;

// Ball class
class Ball
{
private:
    float x, y;
    int speed_x, speed_y;
    int radius;

public:
    Ball() : x(640), y(400), speed_x(7), speed_y(7), radius(20) {}

    Ball(float a, float b, int s1, int s2, int r)
        : x(a), y(b), speed_x(s1), speed_y(s2), radius(r) {}

    void Draw()
    {
        DrawCircle(x, y, radius, WHITE);
    }

    void Update()
    {
        x += speed_x;
        y += speed_y;

        // Bounce off top and bottom walls
        if (y + radius >= GetScreenHeight() || y - radius <= 0)
        {
            speed_y *= -1;
        }

        // Score updates and ball reset
        if (x + radius >= GetScreenWidth())
        {
            if (issingle)
            {
                cpu_score++;
                if (cpu_score >= MAX_SCORE)
                {
                    game_ended = true;
                    winner_text = "CPU WINS!";
                }
            }
            else
            {
                cpu_score++;
                if (cpu_score >= MAX_SCORE)
                {
                    game_ended = true;
                    winner_text = "PLAYER 2 WINS!";
                }
            }
            ResetBall();
        }

        if (x - radius <= 0)
        {
            player_score++;
            if (player_score >= MAX_SCORE)
            {
                game_ended = true;
                winner_text = issingle ? "PLAYER 1 WINS!" : "PLAYER 1 WINS!";
            }
            ResetBall();
        }
    }

    void ResetBall()
    {
        x = GetScreenWidth() / 2;
        y = GetScreenHeight() / 2;

        int speed_choices[2] = {-1, 1};
        speed_x = 7 * speed_choices[GetRandomValue(0, 1)];
        speed_y = 7 * speed_choices[GetRandomValue(0, 1)];
    }

    // Getter functions
    float getX() const { return x; }
    float getY() const { return y; }
    int getSpeedX() const { return speed_x; }
    int getSpeedY() const { return speed_y; }
    int getRadius() const { return radius; }

    // Setter functions
    void setSpeedX(int speed) { speed_x = speed; }
    void setSpeedY(int speed) { speed_y = speed; }
};

// Paddle class
class Paddle
{
protected:
    float x, y;
    float width, height;
    int speed;

    void LimitMovement()
    {
        if (y <= 0)
        {
            y = 0;
        }
        if (y + height >= GetScreenHeight())
        {
            y = GetScreenHeight() - height;
        }
    }

public:
    Paddle(float width = 25, float height = 120, int speed = 6, float x_pos = 0, float y_pos = 0)
        : x(x_pos), y(y_pos), width(width), height(height), speed(speed) {}

    void Draw()
    {
        DrawRectangleRounded(Rectangle{x, y, width, height}, 0.8, 0, WHITE);
    }

    void Update()
    {
        if (IsKeyDown(KEY_UP))
        {
            y -= speed;
        }
        if (IsKeyDown(KEY_DOWN))
        {
            y += speed;
        }
        LimitMovement();
    }

    // Getter functions
    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    // Setter function
    void setHeight(float h) { height = h; }
};

// CPU Paddle
class CpuPaddle : public Paddle
{
public:
    CpuPaddle(float width = 25, float height = 120, int speed = 6, float x_pos = 10, float y_pos = 0)
        : Paddle(width, height, speed, x_pos, y_pos) {}

    void Update(int ball_y)
    {
        if (y + height / 2 > ball_y)
        {
            y -= speed;
        }
        if (y + height / 2 < ball_y)
        {
            y += speed;
        }
        LimitMovement();
    }
};

// Player Paddle
class PlayerPaddle : public Paddle
{
public:
    PlayerPaddle(float width = 25, float height = 120, int speed = 6, float x_pos = 10, float y_pos = 0)
        : Paddle(width, height, speed, x_pos, y_pos) {}

    void Update()
    {
        if (IsKeyDown(KEY_W))
        {
            y -= speed;
        }
        if (IsKeyDown(KEY_S))
        {
            y += speed;
        }
        LimitMovement();
    }
};

// PowerUp class
class PowerUp
{
private:
    float x, y;
    float size;
    int type;
    bool active;        // 0: Increase paddle size, 1: Increase ball speed, 2: Spawn extra ball
    float duration;     // Duration of the power-up effect
    float active_timer; // Tracks time since activation

public:
    PowerUp(float x_pos, float y_pos, float size, int type, float duration = 5.0f)
        : x(x_pos), y(y_pos), size(size), active(true), type(type), duration(duration), active_timer(0.0f) {}

    void Draw()
    {
        if (active)
        {
            DrawRectangle(x, y, size, size, (type == 0) ? blue : (type == 1) ? Light_Green
                                                                             : Yellow);
        }
        else if (active_timer > 0.0f)
        {
            DrawText(TextFormat("Effect %.1f", duration - active_timer), 10, 50 + 20 * type, 20, WHITE);
        }
    }
    void RemoveEffect(Paddle &player, CpuPaddle &cpu, Ball &ball, vector<Ball> &extraBalls)
    {
        if (type == 0)
        {
            // Revert paddle size increase
            player.setHeight(player.getHeight() / 1.5);
        }
        else if (type == 1)
        {
            // Revert ball speed increase
            ball.setSpeedX(ball.getSpeedX() / 1.2);
            ball.setSpeedY(ball.getSpeedY() / 1.2);
        }
        else if (type == 2)
        {
            // Remove extra balls (optional, as they might be removed via game rules)
            extraBalls.clear();
        }
    }
    void Update(float deltaTime, Paddle &player, CpuPaddle &cpu, Ball &ball, vector<Ball> &extraBalls)
    {
        if (!active && active_timer > 0.0f)
        {
            active_timer += deltaTime;
            if (active_timer >= duration)
            {
                active_timer = 0.0f;
                RemoveEffect(player, cpu, ball, extraBalls);
            }
        }
    }

    void CheckCollision(Ball &ball, Paddle &player, CpuPaddle &cpu, vector<Ball> &extraBalls)
    {
        if (active && CheckCollisionCircleRec({ball.getX(), ball.getY()}, ball.getRadius(), {x, y, size, size}))
        {
            active = false;
            active_timer = 0.01f; // Start the effect timer
            ApplyEffect(player, cpu, ball, extraBalls);
        }
    }

    void ApplyEffect(Paddle &player, CpuPaddle &cpu, Ball &ball, vector<Ball> &extraBalls)
    {
        if (type == 0)
        {
            player.setHeight(player.getHeight() * 1.5);
        }
        else if (type == 1)
        {
            ball.setSpeedX(ball.getSpeedX() * 1.2);
            ball.setSpeedY(ball.getSpeedY() * 1.2);
        }
        else if (type == 2)
        {
            extraBalls.emplace_back(ball.getX(), ball.getY(), 7, 7, 20);
        }
    }

    bool isActive() const { return active; }
};

void draw(int playerLives, int opponentLives, bool isSinglePlayer)
{
    int ballRadius = 10;                  // Radius of each ball
    int screenWidth = GetScreenWidth();   // Get screen width dynamically
    int screenHeight = GetScreenHeight(); // Get screen height dynamically

    // Define positions for the balls on the screen
    int leftSideX = 50;                // Player 1's side (leftmost corner)
    int rightSideX = screenWidth - 50; // Opponent's side (rightmost corner)
    int centerY = 10;                  // Vertically center the balls

    if (isSinglePlayer)
    {
        // Draw Player 1's balls (lives) on the rightmost side of the screen
        for (int i = 0; i < playerLives; ++i)
        {
            // Calculate horizontal offset for each ball (spacing them out horizontally)
            int ballX = rightSideX - (i * (ballRadius * 2)); // Position balls horizontally
            // Draw the ball for Player 1 (Light Pink balls for Player 1)
            DrawCircle(ballX, centerY, ballRadius, LightPink);
        }
    }
    else
    {
        // Draw Player 1's balls (lives) on the leftmost side of the screen
        for (int i = 0; i < opponentLives; ++i)
        {
            // Calculate horizontal offset for each ball (spacing them out horizontally)
            int ballX = leftSideX + (i * (ballRadius * 2)); // Position balls horizontally
            // Draw the ball for Player 1 (Light Pink balls for Player 1)
            DrawCircle(ballX, centerY, ballRadius, LightPink);
        }

        // Draw Player 2's balls (lives) on the rightmost side of the screen
        for (int i = 0; i < playerLives; ++i)
        {
            // Calculate horizontal offset for each ball (spacing them out horizontally)
            int ballX = rightSideX - (i * (ballRadius * 2)); // Position balls horizontally
            // Draw the ball for Player 2 (Green balls for Player 2)
            DrawCircle(ballX, centerY, ballRadius, Green);
        }
    }
}

// Global Variables
Ball ball;
Ball backgroundBall(640, 400, 4, 4, 15);
Paddle player;
CpuPaddle cpu;
PlayerPaddle player1;

vector<PowerUp> powerups;
vector<Ball> extraBalls;
int last_powerup_score = 0;
const float SPAWN_INTERVAL = 5.0f; // Time between spawns in seconds
float spawnTimer = 0.0f;

// Main
int main()
{
    const int screen_width = 1280;
    const int screen_height = 800;
    InitWindow(screen_width, screen_height, "Pong Game with PowerUps!");
    SetTargetFPS(60);

    player = Paddle(25, 120, 6, screen_width - 25 - 10, screen_height / 2 - 120 / 2);
    cpu = CpuPaddle(25, 120, 6, 10, screen_height / 2 - 120 / 2);
    player1 = PlayerPaddle(25, 120, 6, 10, screen_height / 2 - 120 / 2);

    bool game_started = false;

    // Game Loop
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        BeginDrawing();
        ClearBackground(BLACK);

        if (!game_started)
        {
            backgroundBall.Update();
            backgroundBall.Draw();
            DrawText("PONG GAME", screen_width / 2 - 220, screen_height / 3, 80, WHITE);
            DrawText("Press S for Singleplayer", screen_width / 2 - 240, screen_height / 2, 40, WHITE);
            DrawText("Press M for Multiplayer", screen_width / 2 - 240, screen_height / 2 + 50, 40, WHITE);
            player_score = 0;
            cpu_score = 0;
            if (IsKeyPressed(KEY_S))
            {
                issingle = true;
                game_started = true;
            }
            else if (IsKeyPressed(KEY_M))
            {
                issingle = false;
                game_started = true;
            }
        }
        else if (game_ended)
        {
            DrawText(winner_text.c_str(), screen_width / 2 - MeasureText(winner_text.c_str(), 40) / 2, screen_height / 2, 40, WHITE);
            DrawText("Press R to Restart or ESC to Exit", screen_width / 2 - MeasureText("Press R to Restart or ESC to Exit", 30) / 2, screen_height / 2 + 50, 30, WHITE);

            if (IsKeyPressed(KEY_R))
            {
                game_ended = false;
                game_started = false;
                player_score = 0;
                cpu_score = 0;
                winner_text = "";
                ball.ResetBall();
                powerups.clear(); // Clear all power-ups when restarting the game
            }
        }
        else
        {
            DrawCircle(screen_width / 2, screen_height / 2, 150, LightPink);
            DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, WHITE);
            ball.Update();

            if (issingle)
            {
                player.Update();
                cpu.Update(ball.getY());
            }
            else
            {
                player.Update();
                player1.Update();
            }

            if (CheckCollisionCircleRec({ball.getX(), ball.getY()}, ball.getRadius(), {player.getX(), player.getY(), player.getWidth(), player.getHeight()}))
            {
                ball.setSpeedX(-ball.getSpeedX());
            }

            if (!issingle && CheckCollisionCircleRec({ball.getX(), ball.getY()}, ball.getRadius(), {player1.getX(), player1.getY(), player1.getWidth(), player1.getHeight()}))
            {
                ball.setSpeedX(-ball.getSpeedX());
            }

            if (issingle && CheckCollisionCircleRec({ball.getX(), ball.getY()}, ball.getRadius(), {cpu.getX(), cpu.getY(), cpu.getWidth(), cpu.getHeight()}))
            {
                ball.setSpeedX(-ball.getSpeedX());
            }

            if (issingle)
                cpu.Draw();
            else
                player1.Draw();
            draw(MAX_SCORE - cpu_score, issingle ? 0 : MAX_SCORE - player_score, issingle);
            player.Draw();
            ball.Draw();
            spawnTimer += deltaTime;

            // Check if it's time to spawn a new power-up
            if (spawnTimer >= SPAWN_INTERVAL)
            {
                // Reset spawn timer
                spawnTimer = 0.0f;

                // Generate random position and type for the new power-up
                float x = GetRandomValue(200, screen_width - 200);
                float y = GetRandomValue(100, screen_height - 100);
                int type = GetRandomValue(0, 2); // Power-up types: 0 (increase paddle size), 1 (increase ball speed), 2 (extra ball)

                // Create and add the new power-up to the vector
                powerups.emplace_back(x, y, 50, type);
            }

            for (auto &powerup : powerups)
            {
                powerup.Update(deltaTime, player, cpu, ball, extraBalls);
                powerup.Draw();
                powerup.CheckCollision(ball, player, cpu, extraBalls);
            }

            for (auto it = extraBalls.begin(); it != extraBalls.end();)
            {
                it->Update();
                it->Draw();
                if (it->getX() - it->getRadius() <= 0 || it->getX() + it->getRadius() >= screen_width)
                {
                    it = extraBalls.erase(it);
                }
                else
                {
                    ++it;
                }
            }

            DrawText(TextFormat("%i", cpu_score), screen_width / 4 - 20, 20, 80, WHITE);
            DrawText(TextFormat("%i", player_score), 3 * screen_width / 4 - 20, 20, 80, WHITE);
            if(issingle){
                if (player_score >= MAX_SCORE || cpu_score >= MAX_SCORE)
                {
                    game_ended = true;
                    winner_text = (player_score >= MAX_SCORE) ? "PLAYER 1 WINS!" : "CPU WINS!";
                }
            }
            else{
                if (player_score >= MAX_SCORE || cpu_score >= MAX_SCORE)
                {
                    game_ended = true;
                    winner_text = (player_score >= MAX_SCORE) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}