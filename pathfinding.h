#ifndef __PATHFINDING_H__
#define __PATHFINDING_H__

#define MAX_BFS_DIM 9

enum terrain_e : i8
{
    TERRAIN_OCCUPIED = -1,
    TERRAIN_EMPTY,
    TERRAIN_DIFFICULT
};

struct bfs_queue
{
    i32 front;
    i32 rear;
    i32 data[MAX_BFS_DIM * MAX_BFS_DIM];
};

struct bfs_info
{
    i32 start_x;
    i32 start_y;
    i32 width;
    i32 height;
    i32 move_range;
    terrain_e terrain[MAX_BFS_DIM][MAX_BFS_DIM];
};

struct bfs_result
{
    i32 costs[MAX_BFS_DIM][MAX_BFS_DIM];
};

#endif