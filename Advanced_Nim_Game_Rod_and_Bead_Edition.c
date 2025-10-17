#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_PILES 5
#define MAX_POLES 2
#define MAX_NAME_LEN 50
#define DATABASE_FILE "game_database.txt"

// ساختار برای ذخیره وضعیت بازی
typedef struct {
    int piles[MAX_PILES][MAX_POLES];
    int currentPlayer;
} GameState;

// ساختار برای تاریخچه بازی
typedef struct {
    char gameName[MAX_NAME_LEN];
    GameState states[100];
    int moveCount;
    int numPiles;
} GameHistory;

typedef void (*PlayerMoveFunc)(GameHistory *, int, int);

// چاپ قوانین بازی
void printRules() {
    printf("Rules of the Game:\n");
    printf("1. The game is played in 5 rounds. The player who wins the most rounds is the overall winner.\n");
    printf("2. The game consists of piles of tokens distributed on two poles.\n");
    printf("3. Each move, you can transfer up to K tokens from one pole to another.\n");
    printf("4. If the number of tokens in a pile is less than K, all tokens in that pile will be transferred.\n");
    printf("5. In single player mode, you play against the computer. You can choose the difficulty level (Easy, Medium, Hard).\n");
    printf("6. In two-player mode, players take turns making moves.\n");
    printf("7. The game ends when all tokens are transferred or removed from the piles.\n");
    printf("8. You can save and replay the game at any time during the game.\n");
    printf("Let's begin!\n\n");
}

// چاپ وضعیت فعلی توده‌ها
void printPiles(int piles[MAX_PILES][MAX_POLES], int size) {
    printf("Current game state:\n");
    for (int i = 0; i < size; i++) {
        printf("Pile %d: ", i + 1);
        for (int j = 0; j < MAX_POLES; j++) {
            printf("Pole %d: ", j + 1);
            for (int k = 0; k < piles[i][j]; k++) {
                printf("\u25CF "); // Unicode character for black circle (●)
            }
            printf("(%d) ", piles[i][j]);
        }
        printf("\n");
    }
}

// بررسی اینکه آیا بازی تمام شده است یا خیر
int isGameOver(int piles[MAX_PILES][MAX_POLES], int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < MAX_POLES; j++) {
            if (piles[i][j] > 0) {
                return 0;
            }
        }
    }
    return 1;
}

// ذخیره وضعیت بازی در فایل
void saveGameState(GameHistory *history) 
{
    FILE *file = fopen(DATABASE_FILE, "a");
    if (!file) {
        printf("Error saving game state!\n");
        return;
    }
    fprintf(file, "Game: %s\n", history->gameName);
    fprintf(file, "Moves: %d\n", history->moveCount);
    fprintf(file, "NumPiles: %d\n", history->numPiles);
    for (int moveIndex = 0; moveIndex < history->moveCount; moveIndex++) {
        GameState state = history->states[moveIndex];
        fprintf(file, "Player: %d\n", state.currentPlayer);
        for (int i = 0; i < history->numPiles; i++) {
            for (int j = 0; j < MAX_POLES; j++) {
                fprintf(file, "%d ", state.piles[i][j]);
            }
            fprintf(file, "\n");
        }
    }
    fprintf(file, "EndGame\n");
    fclose(file);
    printf("Game state saved with name %s\n", history->gameName);
}

// بارگذاری وضعیت بازی از فایل
int loadGameState(GameHistory *history, const char *gameName) {
    FILE *file = fopen(DATABASE_FILE, "r");
    if (!file) {
        printf("Error loading game state!\n");
        return 0;
    }
    char line[256];
    int found = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = 0;
        if (strncmp(line, "Game: ", 6) == 0 && strcmp(line + 6, gameName) == 0) {
            found = 1;
            strcpy(history->gameName, gameName);
            break;
        }
    }
    if (!found) {
        printf("Game with name %s not found!\n", gameName);
        fclose(file);
        return 0;
    }
    if (fscanf(file, "Moves: %d\n", &history->moveCount) != 1) {
        printf("Error reading move count!\n");
        fclose(file);
        return 0;
    }
    if (fscanf(file, "NumPiles: %d\n", &history->numPiles) != 1) {
        printf("Error reading number of piles!\n");
        fclose(file);
        return 0;
    }
    for (int moveIndex = 0; moveIndex < history->moveCount; moveIndex++) {
        GameState state;
        if (fscanf(file, "Player: %d\n", &state.currentPlayer) != 1) {
            printf("Error reading player information at move %d!\n", moveIndex + 1);
            fclose(file);
            return 0;
        }
        for (int i = 0; i < history->numPiles; i++) {
            for (int j = 0; j < MAX_POLES; j++) {
                if (fscanf(file, "%d", &state.piles[i][j]) != 1) {
                    printf("Error reading piles state at move %d, pile %d, pole %d!\n", moveIndex + 1, i + 1, j + 1);
                    fclose(file);
                    return 0;
                }
            }
        }
        history->states[moveIndex] = state;
    }
    fclose(file);
    return 1;
}

// بررسی اینکه آیا تمام توکن‌ها منتقل شده‌اند یا خیر
int isAllTokensTransferred(int (*piles)[MAX_POLES], int size) {
    for (int i = 0; i < size; i++) {
        if (piles[i][0] > 0) {
            return 0;
        }
    }
    return 1;
}

// حرکت بهینه (برای بازی کامپیوتری)
void optimalMove(GameHistory *history, int moveIndex, int K, int difficulty) {
    int (*piles)[MAX_POLES] = history->states[moveIndex].piles;
    for (int i = 0; i < history->numPiles; i++) {
        if (piles[i][0] > 0) {
            int tokensToTransfer = (difficulty == 3) ? (piles[i][0] > K ? K : piles[i][0]) : K;
            piles[i][0] -= tokensToTransfer;
            piles[i][1] += tokensToTransfer;
            return;
        }
    }
    for (int i = 0; i < history->numPiles; i++) {
        if (piles[i][1] > 0) {
            int tokensToRemove = (difficulty == 3) ? (piles[i][1] > K ? K : piles[i][1]) : K;
            piles[i][1] -= tokensToRemove;
            return;
        }
    }
}

// پوشش‌دهنده حرکت بهینه (برای فراخوانی راحت‌تر)
void optimalMoveWrapper(GameHistory *history, int moveIndex, int K) {
    optimalMove(history, moveIndex, K, 3);
}

// تابع برای گرفتن ورودی عدد صحیح و معتبر (عدد صحیح و مثبت)
int getValidatedIntInput(const char *prompt) {
    char input[100];
    int value;
    while (1) {
        printf("%s", prompt);
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &value) == 1 && value > 0) {
            return value;
        } else {
            printf("Invalid input. Please enter a positive integer.\n");
        }
    }
}

// تابع برای گرفتن ورودی K صحیح و اطمینان از اینکه عدد بین 1 و 15 است
int getValidatedKInput(const char *prompt) {
    int value;
    while (1) {
        value = getValidatedIntInput(prompt);
        if (value >= 1 && value <= 15) {
            return value;
        } else {
            printf("Invalid input. Please enter a number between 1 and 15.\n");
        }
    }
}

// حرکت پیش‌فرض بازیکن
void playerMoveDefault(GameHistory *history, int moveIndex, int K) {
    int pileIndex, poleIndex;
    int (*piles)[MAX_POLES] = history->states[moveIndex].piles;
    int player = history->states[moveIndex].currentPlayer;
    int numPiles = history->numPiles;
    while (1) {
        pileIndex = getValidatedIntInput("Choose a pile (1-5): ") - 1;
        poleIndex = getValidatedIntInput("Choose a pole: ") - 1;
        if (pileIndex < 0 || pileIndex >= numPiles || poleIndex < 0 || poleIndex >= MAX_POLES || piles[pileIndex][poleIndex] == 0) {
            printf("Invalid pile or no tokens, try again.\n");
            continue;
        }
        if ((poleIndex == 1) && !isAllTokensTransferred(piles, numPiles)) {
            printf("You cannot move tokens from the second pole before moving all from the first pole. Try again.\n");
            continue;
        }
        if (poleIndex == 0) {
            if (K > piles[pileIndex][poleIndex]) {
                printf("Not enough tokens, transferring all remaining tokens...\n");
                int tokensToTransfer = piles[pileIndex][poleIndex];
                piles[pileIndex][poleIndex] = 0;
                piles[pileIndex][poleIndex + 1] += tokensToTransfer;
            } else {
                piles[pileIndex][poleIndex] -= K;
                piles[pileIndex][poleIndex + 1] += K;
            }
        } else if (poleIndex == 1) {
            if (K >= piles[pileIndex][poleIndex]) {
                printf("Not enough tokens, removing all remaining tokens in the pole...\n");
                piles[pileIndex][poleIndex] = 0;
            } else {
                piles[pileIndex][poleIndex] -= K;
            }
        }
        break;
    }
}

// بارگذاری توابع حرکت بازیکن‌ها بر اساس میزان دشواری
void loadPlayerMoveFuncs(PlayerMoveFunc *func1, PlayerMoveFunc *func2, int difficulty) {
    *func1 = playerMoveDefault;
    *func2 = difficulty < 3 ? playerMoveDefault : optimalMoveWrapper;
}

// بازپخش بازی از تاریخچه بازی‌ها
void replayGame(GameHistory *history) {
    for (int i = 0; i < history->moveCount; i++) {
        printf("Move %d:\n", i + 1);
        printPiles(history->states[i].piles, history->numPiles);
        printf("Player %d's turn:\n", history->states[i].currentPlayer);
    }
}

// تابع اصلی بازی
int main() {
    srand(time(NULL));
    printRules(); // چاپ قوانین بازی
    int K, difficulty = 1, gameMode;
    K = getValidatedKInput("Enter the number of tokens to be removed in each move (K): "); // دریافت مقدار K معتبر
    while (1) {
        gameMode = getValidatedIntInput("Choose game mode (1 - Single Player, 2 - Two Players, 3 - Computer vs Computer): "); // انتخاب حالت بازی
        if (gameMode == 1 || gameMode == 2 || gameMode == 3) {
            break;
        } else {
            printf("Invalid game mode. Please enter 1 for Single Player, 2 for Two Players, or 3 for Computer vs Computer.\n");
        }
    }
    if (gameMode == 1 || gameMode == 3) {
        while (1) {
            difficulty = getValidatedIntInput("Enter the difficulty level (1 - Easy, 2 - Medium, 3 - Hard): "); // انتخاب سطح سختی
            if (difficulty >= 1 && difficulty <= 3) {
                break;
            } else {
                printf("Invalid difficulty level. Please enter  anthor value ths value is un available a value between 1 and 3.\n");
            }
        }
    }
    int winsPlayer1 = 0;
    int winsPlayer2 = 0;
    for (int round = 1; round <= 5; round++) 
    { // حلقه برای همه راند‌ها
        printf("Round %d begins!\n", round);
        GameHistory history = {.moveCount = 0};
        int (*piles)[MAX_POLES] = history.states[history.moveCount].piles;
        history.numPiles = 1 + rand() % MAX_PILES;
        for (int i = 0; i < history.numPiles; i++) {
            int randomTokens = 10 + rand() % 11;
            for (int j = 0; j < MAX_POLES; j++) {
                piles[i][j] = randomTokens;
            }
        }
        history.states[history.moveCount].currentPlayer = 1;
        PlayerMoveFunc player1Move, player2Move;
        if (gameMode == 1) {
            loadPlayerMoveFuncs(&player1Move, &player2Move, difficulty);
        } else if (gameMode == 2) {
            player1Move = playerMoveDefault;
            player2Move = playerMoveDefault;
        } else if (gameMode == 3) {
            player1Move = optimalMoveWrapper;
            player2Move = optimalMoveWrapper;
        }
        char command[MAX_NAME_LEN];
        while (!isGameOver(history.states[history.moveCount].piles, history.numPiles)) {
            printPiles(history.states[history.moveCount].piles, history.numPiles);
            printf("Player %d, it is your turn.\n", history.states[history.moveCount].currentPlayer);
            printf("Enter 'save' to save the game, 'replay' to replay the game, or any other key to continue: ");
            scanf("%s", command);
            if (strcmp(command, "save") == 0) {
                char gameName[MAX_NAME_LEN];
                printf("Enter the save game name: ");
                scanf("%s", gameName);
                strcpy(history.gameName, gameName);
                saveGameState(&history);
            } else if (strcmp(command, "replay") == 0) {
                char gameName[MAX_NAME_LEN];
                printf("Enter the replay game name: ");
                scanf("%s", gameName);
                if (loadGameState(&history, gameName)) {
                    replayGame(&history);
                } else {
                    printf("Error loading game state from %s\n", gameName);
                }
            } else {
                if (history.states[history.moveCount].currentPlayer == 1) {
                    player1Move(&history, history.moveCount, K);
                } else {
                    player2Move(&history, history.moveCount, K);
                }
            }
            history.moveCount++;
            history.states[history.moveCount] = history.states[history.moveCount - 1];
            history.states[history.moveCount].currentPlayer = 3 - history.states[history.moveCount - 1].currentPlayer;
        }
        printf("Game over!\n");
        if (history.states[history.moveCount].currentPlayer == 2) {
            printf("Player 1 wins this round!\n");
            winsPlayer1++;
        } else {
            printf("Player 2 wins this round!\n");
            winsPlayer2++;
        }
    }
    printf("Final Results:\n");
    printf("Player 1 wins: %d\n", winsPlayer1);
    printf("Player 2 wins: %d\n", winsPlayer2);
    if (winsPlayer1 > winsPlayer2) {
        printf("Player 1 is the overall winner!\n");
    } else if (winsPlayer2 > winsPlayer1) {
        printf("Player 2 is the overall winner!\n");
    } else {
        printf("It's a tie!\n");
    }
    return 0;
}
