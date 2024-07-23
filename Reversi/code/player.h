#include <string.h>
#include "../include/playerbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

int priority[20][20];
int directions[8][2] = 
    {
        {0, 1}, {0, -1}, {1, 0}, {-1, 0},
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
    };
void set_corners(int x, int y) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            priority[x + i][y + j] = 1;
        }
    }
}

void init(struct Player *player) {
    // This function will be executed at the begin of each game, only once.
    int row = player->row_cnt;
    int col = player->col_cnt;
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            priority[i][j] = 4;
        }
    }
    set_corners(0, 0), set_corners(0, col - 2);
    set_corners(row - 2, 0), set_corners(row - 2, col - 2);
    priority[0][0] = priority[0][col-1] = priority[row-1][0] = priority[row-1][col-1] = 5;
    for (int i = 2; i < col - 2; i++) {
        priority[0][i] = priority[row - 1][i] = 3;
        priority[1][i] = priority[row - 2][i] = 2;
    }
    for (int i = 2; i < row - 2; i++) {
        priority[i][0] = priority[i][col - 1] = 3;
        priority[i][1] = priority[i][col - 2] = 2;
    }
}

int SensPoint(int x, int y)
{
    return priority[x][y]==1;
}

bool is_within_bounds(struct Player *player, int x, int y) {
    return x >= 0 && x < player->row_cnt && y >= 0 && y < player->col_cnt;
}

bool check_direction(struct Player *player, int posx, int posy, int dx, int dy) {
    int x = posx + dx;
    int y = posy + dy;
    if (!is_within_bounds(player, x, y) || player->mat[x][y] != 'o') {
        return false;
    }
    while (is_within_bounds(player, x, y)) {
        x += dx;
        y += dy;
        if (!is_within_bounds(player, x, y) || (player->mat[x][y] >= '1' && player->mat[x][y] <= '9')) {
            break;
        }
        if (player->mat[x][y] == 'O') {
            return true;
        }
    }
    return false;
}

bool opponent_check_direction(struct Player *player, int posx, int posy, int dx, int dy) {
    int x = posx + dx;
    int y = posy + dy;
    if (!is_within_bounds(player, x, y) || player->mat[x][y] != 'O') {
        return false;
    }
    while (is_within_bounds(player, x, y)) {
        x += dx;
        y += dy;
        if (!is_within_bounds(player, x, y) || (player->mat[x][y] >= '1' && player->mat[x][y] <= '9')) {
            break;
        }
        if (player->mat[x][y] == 'o') {
            return true;
        }
    }
    return false;
}

bool is_valid(struct Player *player, int posx, int posy) {
    if (!is_within_bounds(player, posx, posy) || player->mat[posx][posy] == 'o' || player->mat[posx][posy] == 'O') {
        return false;
    }
    for (int i = 0; i < 8; i++) {
        if (check_direction(player, posx, posy, directions[i][0], directions[i][1])) {
            return true;
        }
    }
    return false;
}

int opponent_valid(struct Player *player, int posx, int posy)
{
    if (!is_within_bounds(player, posx, posy) || player->mat[posx][posy] == 'o' || player->mat[posx][posy] == 'O') {
        return false;
    }
    for (int i = 0; i < 8; i++) {
        if (opponent_check_direction(player, posx, posy, directions[i][0], directions[i][1])) {
            return true;
        }
    }
    return false;
}

void flip_line(struct Player *player, int x, int y, int dx, int dy) {
    int nx = x + dx;
    int ny = y + dy;
    while (is_within_bounds(player, nx, ny) && player->mat[nx][ny] == 'o') {
        player->mat[nx][ny] = 'O';
        nx += dx;
        ny += dy;
    }
}

void ChangeMat(struct Player *player, int i, int j) {
    player->mat[i][j] = 'O';
    for (int dir = 0; dir < 8; dir++) {
        int dx = directions[dir][0];
        int dy = directions[dir][1];
        if (check_direction(player, i, j, dx, dy)) {
            flip_line(player, i, j, dx, dy);
        }
    }
}

/*void copyMat(char** temp_mat, char** mat, int row_cnt, int col_cnt) {
    for (int ii = 0; ii < row_cnt; ii++) {
        for (int jj = 0; jj < col_cnt; jj++) {
            temp_mat[ii][jj] = mat[ii][jj];
        }
    }
}*/

// 检查一个棋子是否是稳定的
bool isStable(struct Player *player, int x, int y)
{
    int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
    for (int i = 0; i < 4; i++)
    {
        int dx1 = x + directions[i][0];
        int dy1 = y + directions[i][1];
        int dx2 = x - directions[i][0];
        int dy2 = x - directions[i][1];
        if (is_within_bounds(player, dx1, dy1) && is_within_bounds(player, dx2, dy2))
        {
            if (((player->mat[dx1][dy1] >= '0' && player->mat[dx1][dy1] <= '9') || player->mat[dx1][dy1] == 'o') && 
                ((player->mat[dx2][dy2] >= '0' && player->mat[dx2][dy2] <= '9') || player->mat[dx2][dy2] == 'o'))
                return false;
        }
    }
    return true;
}

// 计算稳定棋子总数
int getStablePieces(struct Player *player) {
    int stableCount = 0;
    int board_size = player->col_cnt;
    for (int row = 0; row < board_size; row++)
    {
        for (int col = 0; col < board_size; col++)
        {
            if (player->mat[row][col] == 'O' && isStable(player, row, col))
                stableCount++;
        }
    }
    return stableCount;
}

struct Point place(struct Player *player) {
    struct Point point = initPoint(-1, -1);
    int max_weight = -1;
    int min_num = 1000; // 最小行动力
    int max_stable_pieces = -1; // 最大稳定棋子数
    char temp_mat[15][15];

    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            if (is_valid(player, i, j) && priority[i][j] > max_weight) {
                max_weight = priority[i][j];
                point = initPoint(i, j);
            }

            if (is_valid(player, i, j) && priority[i][j] == max_weight) {
                for (int ii = 0; ii < player->row_cnt; ii++) {
                    for (int jj = 0; jj < player->col_cnt; jj++) {
                        temp_mat[ii][jj] = player->mat[ii][jj];
                    }
                }

                ChangeMat(player, i, j);
                int num = 0; // 统计行动力
                for (int pp = 0; pp < player->row_cnt; pp++) {
                    for (int qq = 0; qq < player->col_cnt; qq++) {
                        if (opponent_valid(player, pp, qq) && !SensPoint(pp, qq)) {
                            num++;
                        }
                    }
                }

                int stable_pieces = getStablePieces(player); // 获取稳定棋子数

                if (num < min_num || (num == min_num && stable_pieces > max_stable_pieces)) {
                    point = initPoint(i, j);
                    min_num = num;
                    max_stable_pieces = stable_pieces; // 更新最大稳定棋子数
                }

                for (int ii = 0; ii < player->row_cnt; ii++) {
                    for (int jj = 0; jj < player->col_cnt; jj++) {
                        player->mat[ii][jj] = temp_mat[ii][jj];
                    }
                }
            }
        }
    }
    return point;
}
