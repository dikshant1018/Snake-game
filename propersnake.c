#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#define WIDTH 30
#define HEIGHT 20
#define MAX_TAIL 100
#define HIGH_SCORE_FILE "highscore.txt"

// Console color definitions
#define COLOR_DEFAULT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define COLOR_BRIGHT_RED (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_GREEN (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_BLUE (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define COLOR_BRIGHT_YELLOW (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Globals
int snakeHeadX, snakeHeadY;
int snakeTailX[MAX_TAIL], snakeTailY[MAX_TAIL];
int snakeTailLength;
int fruitX, fruitY;
int gameScore;
int gameSpeed;
bool isGameOver;
int difficultyLevel;
int highScore[3];
enum Direction dir;

HANDLE hConsole;
COORD topLeft = {0, 0};

// Function prototypes
void setColor(WORD color);
void clearScreen();
void initializeGame();
void drawGame();
void input();
void logic();
void generateFruit();
int menu();
void loadHighScores();
void saveHighScores();
void updateHighScore();
void showGameOver();
bool askReplay();
void showLeaderboard();
void showWelcomeScreen();
void drawBorderedText(const char *title);
void showWelcomeBanner();

void main()
{
    int i;
    srand((unsigned)time(NULL));
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    loadHighScores();
    showWelcomeScreen();

    bool playAgain;

    while (1)
    {
        system("cls");
        setColor(COLOR_BRIGHT_BLUE);
        drawBorderedText("MAIN MENU");

        setColor(COLOR_BRIGHT_YELLOW);
        printf("\n   1. Start Game\n");
        printf("   2. View Leaderboard\n");
        printf("   3. Exit\n\n");
        printf("   Enter your choice: ");

        char mainMenuChoice = _getch();

        if (mainMenuChoice == '2')
        {
            showLeaderboard();
            continue;
        }
        else if (mainMenuChoice == '3')
        {
            exit(0);
        }
        else if (mainMenuChoice != '1')
        {
            printf("\n   Invalid choice! Press any key to continue...");
            _getch();
            continue;
        }

        difficultyLevel = menu();

        if (difficultyLevel == 1)
            gameSpeed = 150;
        else if (difficultyLevel == 2)
            gameSpeed = 100;
        else if (difficultyLevel == 3)
            gameSpeed = 50;
        else
            continue;

        initializeGame();

        while (!isGameOver)
        {
            SetConsoleCursorPosition(hConsole, topLeft);
            drawGame();
            input();
            logic();
            Sleep(gameSpeed);
        }

        updateHighScore();
        saveHighScores();
        showGameOver();
        playAgain = askReplay();

        if (!playAgain)
            break;
    }

    
}

void setColor(WORD color)
{
    SetConsoleTextAttribute(hConsole, color);
}

void drawBorderedText(const char *title)
{
    printf("   =========================\n");
    printf("         %s\n", title);
    printf("   =========================\n");
}

void showWelcomeBanner()
{
    setColor(COLOR_BRIGHT_BLUE);

    // WELCOME TO banner
    printf("\n");
    printf("  __        __   _                            _          \n");
    printf("  \\ \\      / /__| | ___ ___  _ __ ___   ___  | |_ ___   \n");
    printf("   \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_  _ \\ / _ \\ | __/ _ \\ \n");
    printf("    \\ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) |  \n");
    printf("     \\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___|  \\__\\___/ \n");
    printf("\n");

    // SNAKE GAME banner
printf("   ____              _              ____                      \n");
printf("  / ___| _ __   __ _| | _____      / ___| __ _ _ __ ___   ___ \n");
printf("  \\___ \\| '_ \\ / _` | |/ / _ \\____| |  _ / _` | '_ ` _ \\ / _ \\\n");
printf("   ___) | | | | (_| |   <  __/____| |_| | (_| | | | | | |  __/\n");
printf("  |____/|_| |_|\\__,_|_|\\_\\___|     \\____|\\__,_|_| |_| |_|\\___|\n");
printf("\n");
setColor(COLOR_BRIGHT_RED);
printf("                  === Welcome to Snake Game ===              \n");

    setColor(COLOR_DEFAULT);
}

void showWelcomeScreen()
{
    system("cls");
    showWelcomeBanner();

    setColor(COLOR_BRIGHT_YELLOW);
    printf("   Controls: WASD to move, X to quit game\n");

    setColor(COLOR_DEFAULT);
    printf("\n   Press any key to start...\n");
    _getch();
}

void clearScreen()
{
    DWORD charsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD consoleSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &charsWritten);
    SetConsoleCursorPosition(hConsole, topLeft);
}

void initializeGame()
{
    snakeHeadX = WIDTH / 2;
    snakeHeadY = HEIGHT / 2;
    snakeTailLength = 0;
    dir = STOP;
    gameScore = 0;
    isGameOver = false;
    generateFruit();
}

void drawGame()
{
    int i, j, k;
    bool printed;

    setColor(COLOR_BRIGHT_BLUE);
    for (i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    for (j = 0; j < HEIGHT; j++)
    {
        for (i = 0; i <= WIDTH; i++)
        {
            if (i == 0 || i == WIDTH)
            {
                setColor(COLOR_BRIGHT_BLUE);
                printf("#");
            }
            else if (i == snakeHeadX && j == snakeHeadY)
            {
                setColor(COLOR_BRIGHT_GREEN);
                printf("O");
            }
            else if (i == fruitX && j == fruitY)
            {
                setColor(COLOR_BRIGHT_RED);
                printf("*");
            }
            else
            {
                printed = false;
                for (k = 0; k < snakeTailLength; k++)
                {
                    if (snakeTailX[k] == i && snakeTailY[k] == j)
                    {
                        setColor(FOREGROUND_GREEN);
                        printf("o");
                        printed = true;
                        break;
                    }
                }
                if (!printed)
                {
                    setColor(COLOR_DEFAULT);
                    printf(" ");
                }
            }
        }
        printf("\n");
    }

    setColor(COLOR_BRIGHT_BLUE);
    for (i = 0; i < WIDTH + 2; i++) printf("#");
    printf("\n");

    setColor(COLOR_DEFAULT);
    printf("Score: %d\n", gameScore);
    printf("High Score (%s): %d\n",
        (difficultyLevel == 1 ? "Easy" : (difficultyLevel == 2 ? "Medium" : "Hard")),
        highScore[difficultyLevel - 1]);
    printf("Controls: WASD to move. Press X to quit.\n");
}

void input()
{
    if (_kbhit())
    {
        char c = tolower(_getch());

        if (c == 'a' && dir != RIGHT) dir = LEFT;
        else if (c == 'd' && dir != LEFT) dir = RIGHT;
        else if (c == 'w' && dir != DOWN) dir = UP;
        else if (c == 's' && dir != UP) dir = DOWN;
        else if (c == 'x') isGameOver = true;
    }
}

void logic()
{
    int i;
    if (snakeTailLength > 0)
    {
        int prevX = snakeTailX[0], prevY = snakeTailY[0];
        snakeTailX[0] = snakeHeadX;
        snakeTailY[0] = snakeHeadY;

        for (i = 1; i < snakeTailLength; i++)
        {
            int prev2X = snakeTailX[i], prev2Y = snakeTailY[i];
            snakeTailX[i] = prevX;
            snakeTailY[i] = prevY;
            prevX = prev2X;
            prevY = prev2Y;
        }
        
    }

    switch (dir)
    {
    case LEFT: snakeHeadX--; break;
    case RIGHT: snakeHeadX++; break;
    case UP: snakeHeadY--; break;
    case DOWN: snakeHeadY++; break;
    default: break;
    }

    // Check wall collision
    if (snakeHeadX <= 0 || snakeHeadX >= WIDTH || snakeHeadY < 0 || snakeHeadY >= HEIGHT) {
        Beep(600, 150);
        Beep(600, 150);
        Beep(600, 150);
        isGameOver = true;
    }

    // Check tail collision
    for (i = 0; i < snakeTailLength; i++)
        if (snakeTailX[i] == snakeHeadX && snakeTailY[i] == snakeHeadY)
            isGameOver = true;

    // Eating fruit
    if (snakeHeadX == fruitX && snakeHeadY == fruitY)
    {
    	Beep(1200,60);
        gameScore += 10;
        snakeTailLength++;
        if (snakeTailLength > MAX_TAIL)
        {
            snakeTailLength = MAX_TAIL;
	    }
        generateFruit();
    }
}

void generateFruit()
{
    int i;
    bool validPos = false;
    while (!validPos)
    {
        fruitX = (rand() % (WIDTH - 1)) + 1;
        fruitY = rand() % HEIGHT;

        validPos = true;
        if (fruitX == snakeHeadX && fruitY == snakeHeadY)
            validPos = false;

        for (i = 0; i < snakeTailLength; i++)
            if (snakeTailX[i] == fruitX && snakeTailY[i] == fruitY)
            {
                validPos = false;
                break;
            }
    }
}

int menu()
{
    char ch;
    while (1)
    {
        system("cls");
        setColor(COLOR_BRIGHT_BLUE);
        drawBorderedText("SELECT DIFFICULTY");
        setColor(COLOR_BRIGHT_YELLOW);
        printf("\n   1. Easy\n");
        printf("   2. Medium\n");
        printf("   3. Hard\n\n");
        printf("   Enter choice: ");

        ch = _getch();

        if (ch == '1' || ch == '2' || ch == '3')
            return ch - '0';

        printf("\n   Invalid choice! Press any key to try again...");
        _getch();
    }
}

void loadHighScores()
{
    FILE *file = fopen(HIGH_SCORE_FILE, "r");
    int i;
    if (file == NULL)
    {
        for (i = 0; i < 3; i++) highScore[i] = 0;
        return;
    }
    for (i = 0; i < 3; i++)
    {
        fscanf(file, "%d", &highScore[i]);
    }
    fclose(file);
}

void saveHighScores()
{
    FILE *file = fopen(HIGH_SCORE_FILE, "w");
    int i;
    if (file == NULL) return;
    for (i = 0; i < 3; i++)
    {
        fprintf(file, "%d\n", highScore[i]);
    }
    fclose(file);
}

void updateHighScore()
{
    if (gameScore > highScore[difficultyLevel - 1])
        highScore[difficultyLevel - 1] = gameScore;
}

void showGameOver()
{
    system("cls");
    setColor(COLOR_BRIGHT_RED);
    printf("\n\n   GAME OVER!\n");
    printf("   Your Score: %d\n", gameScore);
    printf("   High Score (%s): %d\n",
        (difficultyLevel == 1 ? "Easy" : (difficultyLevel == 2 ? "Medium" : "Hard")),
        highScore[difficultyLevel - 1]);
    setColor(COLOR_DEFAULT);
}

bool askReplay()
{
    printf("\n   Play again? (Y/N): ");
    char c = tolower(_getch());
    return (c == 'y');
}

void showLeaderboard()
{
    system("cls");
    setColor(COLOR_BRIGHT_GREEN);

    // Big ASCII banner for "SNAKE GAME"
    printf("  _                    _           _                         _ \n");
    printf(" | |                  | |         | |                       | |\n");
    printf(" | |     ___  __ _  __| | ___ _ __| |__   ___   __ _ _ __ __| |\n");
    printf(" | |    / _ \\/ _` |/ _` |/ _ \\ '__| '_ \\ / _ \\ / _` | '__/ _` |\n");
    printf(" | |___|  __/ (_| | (_| |  __/ |  | |_) | (_) | (_| | | | (_| |\n");
    printf(" |______\\___|\\__,_|\\__,_|\\___|_|  |_.__/ \\___/ \\__,_|_|  \\__,_|\n");
    printf("\n");

    setColor(COLOR_BRIGHT_YELLOW);

    // Leaderboard table with borders
    printf("        +----------------+-----------+\n");
    printf("        |  Difficulty    | High Score|\n");
    printf("        +----------------+-----------+\n");
    printf("        |  Easy          | %-9d |\n", highScore[0]);
    printf("        |  Medium        | %-9d |\n", highScore[1]);
    printf("        |  Hard          | %-9d |\n", highScore[2]);
    printf("        +----------------+-----------+\n");

    setColor(COLOR_DEFAULT);
    printf("\n   Press any key to return to the menu...");
    _getch();
}
