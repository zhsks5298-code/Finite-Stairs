#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstdio>
#include <algorithm>

using namespace std;

struct Point { int x, y; };

HANDLE hBuffer[2];
int nBufferIdx = 0;

const WORD C_HAIR = 0;
const WORD C_SKIN = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
const WORD C_WHITE = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
const WORD C_BROWN = BACKGROUND_RED | BACKGROUND_GREEN;
const WORD C_DEF = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

void initBuffer();
void printToBuffer(int x, int y, const char* str, WORD color = C_DEF);
void flipBuffer();
void drawBorder();
void showMenu();
int getValidNextX(int currentX);

// [캐릭터 그리기] - 사용자 디자인 유지
void drawCharacter(int x, int y, bool isJumping) {
    DWORD dw;

    // ❌ 기존 머리 윗줄 삭제 (y-3 제거)

    // 얼굴 (기존 y-2 → y-3 역할)
    printToBuffer(x + 1, y - 2, " ", C_SKIN);
    printToBuffer(x + 2, y - 2, " ", C_SKIN);

    COORD eyeL = { (SHORT)(x + 1), (SHORT)(y - 2) };
    COORD eyeR = { (SHORT)(x + 2), (SHORT)(y - 2) };
    FillConsoleOutputCharacterA(hBuffer[nBufferIdx], '.', 1, eyeL, &dw);
    FillConsoleOutputCharacterA(hBuffer[nBufferIdx], '.', 1, eyeR, &dw);

    // 몸통
    printToBuffer(x + 1, y - 1, " ", C_WHITE);
    printToBuffer(x + 2, y - 1, " ", C_WHITE);
    printToBuffer(x, y - 1, " ", C_SKIN);
    printToBuffer(x + 3, y - 1, " ", C_SKIN);

    // 다리
    printToBuffer(x + 1, y, " ", C_BROWN);
    printToBuffer(x + 2, y, " ", C_BROWN);

    if (!isJumping) {
        COORD mid = { (SHORT)(x + 1), (SHORT)(y) };
        FillConsoleOutputCharacterA(hBuffer[nBufferIdx], '|', 0, mid, &dw);
    }
}

// [게임 그리기] - 머리 위 계단 유지 + 몸 부분 계단 제거 로직
void drawGame(int score, int stage, const vector<Point>& stairs, int charX, int charY, bool isJumping) {
    drawBorder();
    for (int i = 1; i < 49; i++) printToBuffer(i, 4, "─");
    char scoreBuf[64];
    sprintf(scoreBuf, "Stage: %d | Score: %d / 100", stage, score);
    printToBuffer(3, 2, scoreBuf);

    // 캐릭터 실제 위치 (원본 코드의 cx 가산치 유지)
    int cx = charX + 1;
    int cy = charY;

    for (const auto& s : stairs) {
        if (s.y > 4 && s.y < 23 && s.x > 0 && s.x < 42) {
            for (int i = 0; i < 5; i++) {
                int sx = s.x + i;
                int sy = s.y;

                // 🔥 캐릭터의 몸통(y-1)과 다리(y)가 위치한 칸만 계단을 그리지 않음
                // 머리(y-2, y-3)는 조건에서 제외하여 머리 주변 계단은 사라지지 않게 함
                bool isOverlap = false;
                if (sy == cy && (sx >= cx + 1 && sx <= cx + 2)) isOverlap = true;
                else if (sy == cy - 1 && (sx >= cx && sx <= cx + 3)) isOverlap = true;

                if (isOverlap) continue;

                if (i == 0) printToBuffer(sx, sy, "[");
                else if (i == 4) printToBuffer(sx, sy, "]");
                else printToBuffer(sx, sy, "━");
            }
        }
    }

    // 캐릭터를 마지막에 그려서 계단 위로 덮어씌움
    drawCharacter(cx, cy, isJumping);
    printToBuffer(3, 23, "A: Left | S: Right | Q: Quit");
}

void initBuffer() {
    for (int i = 0; i < 2; i++) {
        hBuffer[i] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };
        SetConsoleCursorInfo(hBuffer[i], &cursorInfo);
    }
}
void printToBuffer(int x, int y, const char* str, WORD color) {
    if (x < 0 || x >= 50 || y < 0 || y >= 25) return;
    DWORD dw; COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hBuffer[nBufferIdx], pos);
    SetConsoleTextAttribute(hBuffer[nBufferIdx], color);
    WriteConsoleA(hBuffer[nBufferIdx], str, (DWORD)strlen(str), &dw, NULL);
}
void flipBuffer() {
    SetConsoleActiveScreenBuffer(hBuffer[nBufferIdx]);
    nBufferIdx = !nBufferIdx;
    COORD pos = { 0, 0 }; DWORD dw;
    FillConsoleOutputCharacterA(hBuffer[nBufferIdx], ' ', 50 * 25, pos, &dw);
    FillConsoleOutputAttribute(hBuffer[nBufferIdx], C_DEF, 50 * 25, pos, &dw);
}
void drawBorder() {
    for (int i = 0; i < 50; i += 2) { printToBuffer(i, 0, "■"); printToBuffer(i, 24, "■"); }
    for (int i = 1; i < 24; i++) { printToBuffer(0, i, "■"); printToBuffer(48, i, "■"); }
}
int getValidNextX(int currentX) {
    int nextX = currentX + (rand() % 2 == 1 ? 5 : -5);
    if (nextX < 2) return currentX + 5;
    if (nextX > 40) return currentX - 5;
    return nextX;
}
void showMenu() {
    int menuCharX = 22, menuCharY = 17;
    while (true) {
        drawBorder();
        printToBuffer(12, 5, "      INFINITY STAIRS    ", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printToBuffer(14, 21, "Press 'Enter' to Start");
        for (int i = 0; i < 6; i++) printToBuffer(20 + (i % 2 == 0 ? 0 : 6), 18 - (i * 2), "[━━━━━]");
        if (menuCharY > 9) { menuCharY -= 2; menuCharX = (menuCharX == 22 ? 28 : 22); }
        else { menuCharX = 22; menuCharY = 17; }
        drawCharacter(menuCharX, menuCharY, false);
        flipBuffer();
        if (_kbhit() && _getch() == 13) break;
        Sleep(400);
    }
}

int main() {
    srand((unsigned int)time(NULL));
    system("mode con cols=50 lines=25");
    system("chcp 949");
    initBuffer();
    while (true) {
        showMenu();
        bool gameLoop = true;
        while (gameLoop) {
            int score = 0, stage = 1, charX = 20, charY = 19;
            vector<Point> stairs; stairs.push_back({ charX + 1, charY + 1 });
            for (int i = 0; i < 15; i++) stairs.push_back({ getValidNextX(stairs.back().x), stairs.back().y - 2 });
            bool alive = true;
            while (alive) {
                drawGame(score, stage, stairs, charX, charY, false);
                flipBuffer();
                if (score >= 100) {
                    printToBuffer(18, 10, "★ SUCCESS ★", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    flipBuffer(); _getch(); alive = false; gameLoop = false; break;
                }
                if (_kbhit()) {
                    char key = _getch();
                    if (key == 'q' || key == 'Q') return 0;
                    int inputDir = (key == 'a' || key == 'A') ? 0 : (key == 's' || key == 'S' ? 1 : -1);
                    if (inputDir != -1) {
                        if (inputDir == (stairs[1].x > stairs[0].x ? 1 : 0)) {
                            score++; int diffX = (inputDir == 1 ? 5 : -5);
                            drawGame(score, stage, stairs, charX + (diffX / 2), charY - 1, true);
                            flipBuffer(); Sleep(60);
                            stairs.erase(stairs.begin());
                            for (auto& s : stairs) { s.y += 2; s.x -= diffX; }
                            stairs.push_back({ getValidNextX(stairs.back().x), stairs.back().y - 2 });
                        }
                        else alive = false;
                    }
                }
                Sleep(10);
            }
            if (!alive && score < 100) {
                while (true) {
                    drawBorder();
                    printToBuffer(15, 8, " G A M E   O V E R ", FOREGROUND_RED | FOREGROUND_INTENSITY);
                    char fScore[32]; sprintf(fScore, "Final Score: %d", score);
                    printToBuffer(17, 10, fScore);
                    printToBuffer(13, 14, "[ R ] : REPLAY", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    printToBuffer(13, 16, "[ Q ] : QUIT", FOREGROUND_RED | FOREGROUND_INTENSITY);
                    flipBuffer();
                    if (_kbhit()) {
                        char choice = _getch();
                        if (choice == 'r' || choice == 'R') break;
                        if (choice == 'q' || choice == 'Q') return 0;
                    }
                    Sleep(100);
                }
            }
        }
    }
    return 0;
}