#include "common.h"
#include "pathfinding.h"

bfs_result bfs_compute(bfs_info &info)
{
    static i32 x_mod[4] = {0, 1, 0, -1};
    static i32 y_mod[4] = {-1, 0, 1, 0};

    bfs_result result = {0};
    for (u32 y = 0; y < MAX_BFS_DIM; y++)
    {
        for (u32 x = 0; x < MAX_BFS_DIM; x++)
        {
            result.costs[y][x] = -1; // -1 means not yet visited
        }
    }
    // flag the starting position as visited with a zero cost to move:
    result.costs[info.start_y][info.start_x] = 0;

    bfs_queue q = {0};
    // put starting position in queue:
    q.front++;
    q.data[q.front] = (info.start_y * MAX_BFS_DIM) + info.start_x;

    while (q.front != q.rear)
    {
        // dequeue into p:
        q.rear++;
        i32 p = q.data[q.rear];

        // extract 2d coordinate from p:
        i32 x_coord = p % MAX_BFS_DIM;
        i32 y_coord = p / MAX_BFS_DIM;

        // visit all neighbors of p, enqueing them if not visited:
        for (u32 d = 0; d < 4; d++)
        {
            i32 neighbor_x = x_coord + x_mod[d];
            i32 neighbor_y = y_coord + y_mod[d];
            if (neighbor_x >= 0 && neighbor_x < MAX_BFS_DIM &&
                neighbor_y >= 0 && neighbor_y < MAX_BFS_DIM)
            {
                if (result.costs[neighbor_y][neighbor_x] < 0 &&
                    info.terrain[neighbor_y][neighbor_x] != TERRAIN_OCCUPIED)
                {
                    // this neighbor is traversable and not visited, so enqueue it:
                    q.front++;
                    q.data[q.front] = (neighbor_y * 9) + neighbor_x;

                    // store the cost of moving to the neighbor in the visited matrix:
                    result.costs[neighbor_y][neighbor_x] =
                        result.costs[y_coord][x_coord] + 1 + info.terrain[neighbor_y][neighbor_x];
                }
            }
        }
    }
    return result;
}

void bfs_print(bfs_result &result)
{
    for (i32 y = 0; y < MAX_BFS_DIM; y++)
    {
        for (i32 x = 0; x < MAX_BFS_DIM; x++)
        {
            printf("%2d ", result.costs[y][x]);
        }
        printf("%s", "\n");
    }
}

void bfs_tests()
{
    bfs_result result = {0};
    bfs_info info = {0};
    info.terrain[4][4] = TERRAIN_OCCUPIED;
    info.terrain[0][1] = TERRAIN_DIFFICULT;
    info.terrain[1][0] = TERRAIN_DIFFICULT;
    for (i32 y = 0; y < MAX_BFS_DIM; y++)
    {
        for (i32 x = 0; x < MAX_BFS_DIM; x++)
        {
            info.start_x = x;
            info.start_y = y;
            bfs_result result = bfs_compute(info);
            bfs_print(result);
            printf("%s", "\n");
        }
    }
}