#include <string.h>
#include "../include/playerbase.h"
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define WALL '#'
#define ROAD '.'
#define APPLE 'o'
#define SHIELD 'O'
#define SNAKE1 '1'
#define SNAKE2 '2'
#define OVERLAP '3'
#define MAX_QUEUE_SIZE 100

// 方向数组
int steps[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

// 判断是否是有效的移动位置
bool isValid(struct Player *player, int x, int y) {
    return x >= 0 && x < player->row_cnt && y >= 0 && y < player->col_cnt &&
           (player->mat[x][y] == ROAD || player->mat[x][y] == APPLE || player->mat[x][y] == SHIELD);
}

// 判断当前位置是否接近边缘并将要缩小
bool willShrinkSoon(struct Player *player, int x, int y) {
    int rounds_to_shrink = player->round_to_shrink;
    int shrink_border = (rounds_to_shrink <= 2) ? 1 : 0;  // 提前2回合考虑
    return x < shrink_border || x >= player->row_cnt - shrink_border ||
           y < shrink_border || y >= player->col_cnt - shrink_border;
}

// 使用DFS判断是否是死胡同
int dfs(struct Player *player, int x, int y, bool visited[30][30], int depth) {
    if (depth > 10) { // 设定一个深度阈值，避免无限递归
        return 0;
    }

    visited[x][y] = true;
    int count = 1;

    for (int i = 0; i < 4; i++) {
        int newX = x + steps[i][0];
        int newY = y + steps[i][1];

        if (isValid(player, newX, newY) && !visited[newX][newY] && !willShrinkSoon(player, newX, newY)) {
            count += dfs(player, newX, newY, visited, depth + 1);
        }
    }

    return count;
}

bool isDeadEnd(struct Player *player, int x, int y) {
    bool visited[30][30];
    memset(visited, 0, sizeof(visited));
    int reachable = dfs(player, x, y, visited, 0);
    return reachable < 6; // 设定一个阈值，小于此值则认为是死胡同
}

// 判断当前位置是否是大死路并计算其中的苹果数量
int dfs_apples(struct Player *player, int x, int y, bool visited[30][30], int *apple_count) {
    if (!isValid(player, x, y) || visited[x][y]) return 0;
    visited[x][y] = true;
    int count = 1;
    if (player->mat[x][y] == APPLE) (*apple_count)++;
    for (int i = 0; i < 4; i++) {
        int newX = x + steps[i][0];
        int newY = y + steps[i][1];
        count += dfs_apples(player, newX, newY, visited, apple_count);
    }
    return count;
}

bool isLargeDeadEnd(struct Player *player, int x, int y, int *apple_count) {
    bool visited[30][30];
    memset(visited, 0, sizeof(visited));
    *apple_count = 0;
    int reachable = dfs_apples(player, x, y, visited, apple_count);
    return reachable < player->your_score * 2; // 设定一个阈值，大于此值则认为是大死路
}

// 尽量在死路中拖延时间吃掉所有苹果
struct Point exploreDeadEnd(struct Player *player) {
    for (int i = 0; i < 4; i++) {
        int dx = player->your_posx + steps[i][0];
        int dy = player->your_posy + steps[i][1];
        if (isValid(player, dx, dy)) {
            return initPoint(dx, dy);
        }
    }
    return initPoint(player->your_posx, player->your_posy);
}

// 初始化函数
void init(struct Player *player) {
    // 初始化相关数据
}

// 广度优先搜索最短路径距离
void find_dis(struct Player *player, struct Point start, struct Point end, int *distance, struct Point came_from[][20]) {
    bool visited[20][20] = {false};
    struct Point queue[MAX_QUEUE_SIZE];
    int front = 0, rear = 0;
    queue[rear++] = start;
    visited[start.X][start.Y] = true;

    while (front < rear) {
        struct Point current = queue[front++];
        if (current.X == end.X && current.Y == end.Y) {
            *distance = 0;
            while (!(current.X == start.X && current.Y == start.Y)) {
                current = came_from[current.X][current.Y];
                (*distance)++;
            }
            return;
        }
        for (int i = 0; i < 4; i++) {
            int newX = current.X + steps[i][0];
            int newY = current.Y + steps[i][1];
            if (isValid(player, newX, newY) && !visited[newX][newY] && !isDeadEnd(player, newX, newY) && !willShrinkSoon(player, newX, newY)) {
                visited[newX][newY] = true;
                came_from[newX][newY] = current;
                queue[rear++] = initPoint(newX, newY);
            }
        }
    }
    *distance = -1;
}

// 每回合执行的函数
struct Point walk(struct Player *player) {
    struct Point ret = initPoint(player->your_posx, player->your_posy);
    struct Point next = ret;
    struct Point came_from[20][20];
    int min_dis = 1000;

    // 对方蛇头位置
    struct Point opponent_head = initPoint(player->opponent_posx, player->opponent_posy);

    // 判断是否应该进攻对方蛇头
    bool should_attack = false;
    int distance_to_opponent = abs(player->your_posx - player->opponent_posx) + abs(player->your_posy - player->opponent_posy);
    if (distance_to_opponent >= 1 && distance_to_opponent <= 2 &&
        ((player->your_status == 0 && player->opponent_status == 0 && player->your_score > player->opponent_score) || 
        (player->your_status > 0 && player->opponent_status == 0))) {
        should_attack = true;
    }

    if (should_attack) {
        // 尝试进攻对方蛇头
        for (int i = 0; i < 4; i++) {
            int newX = player->your_posx + steps[i][0];
            int newY = player->your_posy + steps[i][1];
            if (isValid(player, newX, newY)) {
                int opp_newX = player->opponent_posx + steps[i][0];
                int opp_newY = player->opponent_posy + steps[i][1];
                if (isValid(player, opp_newX, opp_newY) && 
                    (abs(newX - player->opponent_posx) + abs(newY - player->opponent_posy) < distance_to_opponent)) {
                    return initPoint(newX, newY);
                }
            }
        }
    } else {
        // 寻找所有苹果和护盾的位置并计算最短路径
        for (int i = 0; i < player->row_cnt; i++) {
            for (int j = 0; j < player->col_cnt; j++) {
                if (player->mat[i][j] == APPLE || player->mat[i][j] == SHIELD) {
                    struct Point target = initPoint(i, j);
                    int distance;
                    find_dis(player, ret, target, &distance, came_from);
                    if (distance != -1 && distance < min_dis) {
                        min_dis = distance;
                        next = target;
                    }
                }
            }
        }

        // 根据计算结果，确定下一步移动方向
        if (min_dis != 1000) {
            struct Point current = next;
            while (!(current.X == player->your_posx && current.Y == player->your_posy)) {
                struct Point prev = came_from[current.X][current.Y];
                if (prev.X == player->your_posx && prev.Y == player->your_posy) {
                    if (!willShrinkSoon(player, current.X, current.Y)) {
                        return current;
                    }
                }
                current = prev;
            }
        }
    }

    // 如果没有找到苹果或护盾，选择一个有效的移动位置
    for (int i = 0; i < 4; i++) {
        int dx = player->your_posx + steps[i][0];
        int dy = player->your_posy + steps[i][1];
        if (isValid(player, dx, dy) && !willShrinkSoon(player, dx, dy)) {
            return initPoint(dx, dy);
        }
    }

    // 检查是否在大死路中，如果是则尽量拖延时间吃掉所有苹果
    int apple_count = 0;
    if (isLargeDeadEnd(player, player->your_posx, player->your_posy, &apple_count) && apple_count > 0) {
        return exploreDeadEnd(player);
    }

    // 判断两个方向是否是死路，并选择有苹果或护盾的一边
    struct Point best_dead_end_move = initPoint(player->your_posx, player->your_posy);
    int max_apples = -1;
    for (int i = 0; i < 4; i++) {
        int dx = player->your_posx + steps[i][0];
        int dy = player->your_posy + steps[i][1];
        if (isValid(player, dx, dy)) {
            int apple_count = 0;
            if (isLargeDeadEnd(player, dx, dy, &apple_count)) {
                if (apple_count > max_apples) {
                    max_apples = apple_count;
                    best_dead_end_move = initPoint(dx, dy);
                }
            }
        }
    }

    if (max_apples > 0) {
        return best_dead_end_move;
    }

    return initPoint(player->your_posx, player->your_posy);
}
