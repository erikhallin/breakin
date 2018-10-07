#ifndef GAME_H
#define GAME_H

#include <iostream>
#include <sstream>
#include <windows.h>
#include <gl/gl.h>
#include <vector>
#include <math.h>
#include <ctime>
#include <string>

const float _deg2rad=0.0174532925;
const float _rad2deg=57.2957795;

using namespace std;

struct block
{
    block()
    {
        pos[0]=pos[1]=0;
    }
    block(float x,float y)
    {
        pos[0]=x;
        pos[1]=y;
    }
    float pos[2];

};

class game
{
    public:
        game();

        bool init(int world_width, int world_height, bool* keys,bool sound);
        bool update();
        bool draw();
        bool draw_number(char val);
        bool draw_gameover();
        bool draw_time();
        float move_ball_down(float x,float y);

        bool sound_enabled;

        float m_world_size[2];
        float m_world_min_y;
        bool* m_pkeys;
        bool  m_gameover;
        int   m_time_min,m_time_sec;
        int   m_time_atstart,m_time_now;

        float m_player_pos[2];
        float m_player_width;
        float m_player_height;
        float m_player_move_speed;
        float m_player_height_level;

        float m_ball_pos[2];
        float m_ball_direction;
        float m_ball_speed;
        float m_ball_size;

        float m_block_width;
        float m_block_height;
        float m_block_gap;

        vector<block> m_vec_blocks;

    private:
};

#endif // GAME_H
