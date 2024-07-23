#include <string.h>
#include "../include/playerbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_DEPTH 10

int step[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };  // 移动方向：右、左、下、上

void init(struct Player *player) {
    // 在每局游戏开始时执行一次
   // srand(time(0));  // 初始化随机种子
}

bool is_valid_move(struct Player *player, int x, int y) {
    return x >= 0 && x < player->row_cnt && y >= 0 && y < player->col_cnt && 
           (player->mat[x][y] == '.' || player->mat[x][y] == 'o' || player->mat[x][y] == 'O');
}

/*void shrink_mat(struct Player *player)
{
    
    }*/

bool willShrinkSoon(struct Player *player, int x, int y) {
    int rounds_to_shrink = player->round_to_shrink;
    int shrink_border = (rounds_to_shrink <= 2) ? 1 : 0;  // 提前2回合考虑
    return x < shrink_border || x >= player->row_cnt - shrink_border ||
           y < shrink_border || y >= player->col_cnt - shrink_border;
}

bool is_safe(struct Player *player, int x, int y, int depth) {
    if (depth == 0) {
        return true;
    }
    //int new_round_to_shrink=playr->round_to_shrink-1;
    char original = player->mat[x][y];
    player->mat[x][y] = '#';
    
    /*if(new_round_to_shrink==3)
    {
        
    }*/
    for (int i = 0; i < 4; i++) {
        int nx = x + step[i][0];
        int ny = y + step[i][1];
        //player->round_to_shrink=new_round_to_shrink;
        if (is_valid_move(player, nx, ny) && is_safe(player, nx, ny, depth - 1) && !willShrinkSoon(player, nx, ny)) 
        {
            player->mat[x][y] = original;
            //player->round_to_shrink=new_round_to_shrink+1;
            return true;
        }
    }

    player->mat[x][y] = original;
    //player->round_to_shrink=new_round_to_shrink+1;
    return false;
}

struct Point find_food(struct Player *player) {
    struct Point closest_food = initPoint(-1, -1);
    int min_distance = 10000;  // 初始化为一个较大的值

    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            if (player->mat[i][j] == 'o'||player->mat[i][j] == 'O') {
                bool visited[player->row_cnt][player->col_cnt];
                memset(visited, 0, sizeof(visited));

                // BFS 队列
                struct Point queue[player->row_cnt * player->col_cnt];
                int front = 0, rear = 0;

                // 将起始点添加到队列
                queue[rear++] = initPoint(player->your_posx, player->your_posy);
                visited[player->your_posx][player->your_posy] = true;

                int distance = 0;
                bool found = false;

                while (front < rear && !found) {
                    int size = rear - front;
                    for (int k = 0; k < size; k++) {
                        struct Point current = queue[front++];
                        if (current.X == i && current.Y == j) {
                            found = true;
                            break;
                        }

                        for (int d = 0; d < 4; d++) {
                            int nx = current.X + step[d][0];
                            int ny = current.Y + step[d][1];

                            if (is_valid_move(player, nx, ny) && !visited[nx][ny]) {
                                visited[nx][ny] = true;
                                queue[rear++] = initPoint(nx, ny);
                            }
                        }
                    }
                    distance++;
                }

                if (found && distance < min_distance && is_safe(player, i, j, MAX_DEPTH)) {
                    min_distance = distance;
                    closest_food = initPoint(i, j);
                }
            }
        }
    }
    return closest_food;
}

struct Point walk(struct Player *player) {
    struct Point food = find_food(player);
    int best_direction = -1;
    int best_distance = 10000;

    for (int i = 0; i < 4; i++) {
        int dx = player->your_posx + step[i][0];
        int dy = player->your_posy + step[i][1];

        if (is_valid_move(player, dx, dy) && is_safe(player, dx, dy, MAX_DEPTH)) {
            int distance = abs(dx - food.X) + abs(dy - food.Y);
            if (distance < best_distance) {
                best_distance = distance;
                best_direction = i;
            }
        }
    }

    if (best_direction != -1) {
        int new_x = player->your_posx + step[best_direction][0];
        int new_y = player->your_posy + step[best_direction][1];
        return initPoint(new_x, new_y);
    }

    // 如果没有更好的方向，则随机选择一个有效的方向
    for (int i = 0; i < 4; i++) {
        int dx = player->your_posx + step[i][0];
        int dy = player->your_posy + step[i][1];

        if (is_valid_move(player, dx, dy)) {
            return initPoint(dx, dy);
        }
    }

    // 如果没有有效的移动，则保持当前位置
    return initPoint(player->your_posx, player->your_posy);
}
