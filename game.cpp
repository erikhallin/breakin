#include "game.h"

game::game()
{
    //ctor
}

bool game::init(int world_width, int world_height, bool* keys, bool sound)
{
    m_vec_blocks.clear();
    m_time_atstart=clock();
    m_gameover=false;
    m_world_size[0]=world_width;
    m_world_size[1]=world_height;
    m_world_min_y=30;
    m_pkeys=keys;
    if(sound) sound_enabled=true;
    else sound_enabled=false;

    //create player
    m_player_pos[0]=100;
    m_player_height_level=550;
    m_player_pos[1]=m_player_height_level;
    m_player_width=40;
    m_player_height=6;
    m_player_move_speed=4;

    //create ball
    m_ball_pos[0]=295;
    m_ball_pos[1]=300;
    m_ball_direction=230;
    m_ball_speed=4.0;
    m_ball_size=5;

    //set block properties
    m_block_width=15;
    m_block_height=5;
    m_block_gap=3;
    //m_vec_blocks.push_back(block(60,50));


    return true;
}

bool game::update()
{
    if(m_gameover)
    {
        //check for reset
        if(m_pkeys[13] || m_pkeys[27] || m_pkeys[32])
        {
            init(m_world_size[0],m_world_size[1],m_pkeys,sound_enabled);
        }

        return false;
    }

    //update time
    m_time_now=clock();
    m_time_min=int((m_time_now-m_time_atstart)/1000/60);
    m_time_sec=int((m_time_now-m_time_atstart)/1000)-m_time_min*60;

    //increase ball speed
    m_ball_speed=m_time_min+4;

    //update player pos
    if(m_pkeys[37])
    {
        //move player left
        m_player_pos[0]-=m_player_move_speed;

        //cap
        if(m_player_pos[0]-m_player_width<0) m_player_pos[0]=m_player_width;
    }
    if(m_pkeys[39])
    {
        //move player left
        m_player_pos[0]+=m_player_move_speed;

        //cap
        if(m_player_pos[0]+m_player_width>m_world_size[0]) m_player_pos[0]=m_world_size[0]-m_player_width;
    }

    //update ball pos
    float old_pos_x=m_ball_pos[0];
    float old_pos_y=m_ball_pos[1];
    m_ball_pos[0]=m_ball_pos[0]+cosf(m_ball_direction*_deg2rad)*m_ball_speed;
    m_ball_pos[1]=m_ball_pos[1]+sinf(m_ball_direction*_deg2rad)*m_ball_speed;
    //check collision with side walls
    bool collision_done=false;
    if(m_ball_pos[0]<m_ball_size)
    {
        collision_done=true;
        //left side collision
        if(m_ball_direction>90 && m_ball_direction<270)
        {
            m_ball_direction=180-m_ball_direction;
            while(m_ball_direction>=360) m_ball_direction-=360;
            while(m_ball_direction<0) m_ball_direction+=360;
        }
        //cap
        if(m_ball_pos[0]-m_ball_size<0) m_ball_pos[0]=m_ball_size;
    }
    if(m_ball_pos[0]+m_ball_size>m_world_size[0])
    {
        collision_done=true;
        //right side collision
        if(m_ball_direction<90 || m_ball_direction>270)
        {
            m_ball_direction=180-m_ball_direction;
            while(m_ball_direction>=360) m_ball_direction-=360;
            while(m_ball_direction<0) m_ball_direction+=360;
        }
        //cap
        if(m_ball_pos[0]+m_ball_size>m_world_size[0]) m_ball_pos[0]=m_world_size[0]-m_ball_size;
    }
    if(m_ball_pos[1]-m_ball_size<m_world_min_y)
    {
        collision_done=true;
        //top side collision
        if(m_ball_direction>180)
        {
            m_ball_direction=m_ball_direction-(2*m_ball_direction-360);
            while(m_ball_direction>=360) m_ball_direction-=360;
            while(m_ball_direction<0) m_ball_direction+=360;

            //spawn block
            float pos_y=m_block_height+m_block_gap+m_world_min_y;
            float pos_x=-1;
            for(float x=m_block_width+m_block_gap;x<m_world_size[0];x+=m_block_width*2+m_block_gap)
            {
                if(x>m_ball_pos[0]-m_block_width)
                {
                    pos_x=x;
                    break;
                }
            }
            bool pos_blocked=false;
            for(int j=0;j<(int)m_vec_blocks.size();j++)
            {
                //check if block on top of old one
                if(pos_x==m_vec_blocks[j].pos[0] &&
                   m_vec_blocks[j].pos[1]==pos_y)
                {
                    //do not spawn
                    pos_blocked=true;
                    break;
                }
            }
            if(!pos_blocked)
            {
                m_vec_blocks.push_back(block(pos_x,pos_y));

                //move ball
                m_ball_pos[1]=pos_y+m_ball_size+m_block_height+m_block_gap*2;
                float secure_hight=move_ball_down(m_ball_pos[0],m_ball_pos[1]);
                if(secure_hight==-1)
                {
                    m_gameover=true;
                    if(sound_enabled)
                    {
                        Beep(400,100);
                        Beep(250,150);
                    }
                }
                else m_ball_pos[1]=secure_hight;
            }
        }
    }
    if(m_ball_pos[1]>m_world_size[1] && !m_gameover)
    {
        //cout<<"Game Over\n\n";
        m_gameover=true;

        if(sound_enabled)
        {
            Beep(400,100);
            Beep(250,150);
        }
    }

    //test collision with player
    if (m_ball_pos[0] < m_player_pos[0] + m_player_width+m_ball_size &&
        m_ball_pos[0] + m_ball_size+m_player_width > m_player_pos[0] &&
        m_ball_pos[1] < m_player_pos[1] + m_player_height+m_ball_size &&
        m_ball_pos[1] + m_ball_size+m_player_height > m_player_pos[1])
    {
        collision_done=true;
        //collision from topside of player
        if(m_ball_direction<180)
        {
            float real_pos=(m_ball_pos[0]-m_player_pos[0]+m_player_width+m_ball_size)/(m_player_width*2+m_ball_size*2);
            m_ball_direction=180+180*real_pos;
            if(m_ball_direction<200) m_ball_direction=200;
            if(m_ball_direction>340) m_ball_direction=340;
            while(m_ball_direction>=360) m_ball_direction-=360;
            while(m_ball_direction<0) m_ball_direction+=360;
        }
    }

    //test collision with blocks
    for(int i=0;i<(int)m_vec_blocks.size();i++)
    {
        if (m_ball_pos[0] < m_vec_blocks[i].pos[0] + m_block_width+m_ball_size &&
            m_ball_pos[0] + m_ball_size+m_block_width > m_vec_blocks[i].pos[0] &&
            m_ball_pos[1] < m_vec_blocks[i].pos[1] + m_block_height+m_ball_size &&
            m_ball_pos[1] + m_ball_size+m_block_height > m_vec_blocks[i].pos[1])
        {
            collision_done=true;
            //find closest edge
            //int side_colided=-1;//0 right, 1 down, 2 left, 3 up
            float dist_sides[4];
            //float rel_x=m_ball_pos[0]-m_vec_blocks[i].pos[0];
            //float rel_y=m_ball_pos[1]-m_vec_blocks[i].pos[1];
            dist_sides[0]=sqrt((m_ball_pos[0]-m_vec_blocks[i].pos[0]-m_block_width)*(m_ball_pos[0]-m_vec_blocks[i].pos[0]-m_block_width));
            dist_sides[1]=sqrt((m_ball_pos[1]-m_vec_blocks[i].pos[1]-m_block_height)*(m_ball_pos[1]-m_vec_blocks[i].pos[1]-m_block_height));
            dist_sides[2]=sqrt((m_ball_pos[0]-m_vec_blocks[i].pos[0]+m_block_width)*(m_ball_pos[0]-m_vec_blocks[i].pos[0]+m_block_width));
            dist_sides[3]=sqrt((m_ball_pos[1]-m_vec_blocks[i].pos[1]+m_block_height)*(m_ball_pos[1]-m_vec_blocks[i].pos[1]+m_block_height));
            //cout<<dist_sides[0]<<"\t"<<dist_sides[1]<<"\t"<<dist_sides[2]<<"\t"<<dist_sides[3]<<endl;
            if(dist_sides[0]<=dist_sides[1] &&
               dist_sides[0]<=dist_sides[2] &&
               dist_sides[0]<=dist_sides[3])
            {
                //cout<<"1"<<endl;
                //right
                if(m_ball_direction>90 && m_ball_direction<270)
                {
                    m_ball_direction=180-m_ball_direction;

                    //spawn block
                    bool pos_blocked=false;
                    for(int j=0;j<(int)m_vec_blocks.size();j++)
                    {
                        //check if block on top of old one
                        if(m_vec_blocks[j].pos[0]==m_vec_blocks[i].pos[0]+m_block_width*2+m_block_gap &&
                           m_vec_blocks[j].pos[1]==m_vec_blocks[i].pos[1])
                        {
                            //do not spawn
                            pos_blocked=true;
                            break;
                        }
                    }
                    if(!pos_blocked)
                    {
                        m_vec_blocks.push_back(block(m_vec_blocks[i].pos[0]+m_block_width*2+m_block_gap,m_vec_blocks[i].pos[1]));

                        //move ball
                        m_ball_pos[0]=m_vec_blocks[i].pos[0]+m_ball_size+m_block_width*3+m_block_gap*2;
                        float secure_hight=move_ball_down(m_ball_pos[0],m_ball_pos[1]);
                        if(secure_hight==-1)
                        {
                            m_gameover=true;
                            if(sound_enabled)
                            {
                                Beep(400,100);
                                Beep(250,150);
                            }
                        }
                        else m_ball_pos[1]=secure_hight;
                    }
                }
            }
            else if(dist_sides[1]<=dist_sides[0] &&
                    dist_sides[1]<=dist_sides[2] &&
                    dist_sides[1]<=dist_sides[3])
            {
                //cout<<"2"<<endl;
                //down
                if(m_ball_direction>180)
                {
                    m_ball_direction=m_ball_direction-(2*m_ball_direction-360);

                    //spawn block
                    bool pos_blocked=false;
                    for(int j=0;j<(int)m_vec_blocks.size();j++)
                    {
                        //check if block on top of old one
                        if(m_vec_blocks[i].pos[0]==m_vec_blocks[j].pos[0] &&
                           m_vec_blocks[j].pos[1]==m_vec_blocks[i].pos[1]+m_block_height*2+m_block_gap)
                        {
                            //do not spawn
                            pos_blocked=true;
                            break;
                        }
                    }
                    if(!pos_blocked)
                    {
                        m_vec_blocks.push_back(block(m_vec_blocks[i].pos[0],m_vec_blocks[i].pos[1]+m_block_height*2+m_block_gap));

                        //move ball
                        m_ball_pos[1]=m_vec_blocks[i].pos[1]+m_ball_size+m_block_height*3+m_block_gap*2;
                        float secure_hight=move_ball_down(m_ball_pos[0],m_ball_pos[1]);
                        if(secure_hight==-1)
                        {
                            m_gameover=true;
                            if(sound_enabled)
                            {
                                Beep(400,100);
                                Beep(250,150);
                            }
                        }
                        else m_ball_pos[1]=secure_hight;
                    }

                }



            }
            else if(dist_sides[2]<=dist_sides[1] &&
                    dist_sides[2]<=dist_sides[0] &&
                    dist_sides[2]<=dist_sides[3])
            {
                //cout<<"3"<<endl;
                //left
                if(m_ball_direction<90 || m_ball_direction>270)
                {
                    m_ball_direction=180-m_ball_direction;

                    //spawn block
                    bool pos_blocked=false;
                    for(int j=0;j<(int)m_vec_blocks.size();j++)
                    {
                        //check if block on top of old one
                        if(m_vec_blocks[j].pos[0]==m_vec_blocks[i].pos[0]-m_block_width*2-m_block_gap &&
                           m_vec_blocks[j].pos[1]==m_vec_blocks[i].pos[1])
                        {
                            //do not spawn
                            pos_blocked=true;
                            break;
                        }
                    }
                    if(!pos_blocked)
                    {
                        m_vec_blocks.push_back(block(m_vec_blocks[i].pos[0]-m_block_width*2-m_block_gap,m_vec_blocks[i].pos[1]));

                        //move ball
                        m_ball_pos[0]=m_vec_blocks[i].pos[0]-m_ball_size-m_block_width*3-m_block_gap*2;
                        float secure_hight=move_ball_down(m_ball_pos[0],m_ball_pos[1]);
                        if(secure_hight==-1)
                        {
                            m_gameover=true;
                            if(sound_enabled)
                            {
                                Beep(400,100);
                                Beep(250,150);
                            }
                        }
                        else m_ball_pos[1]=secure_hight;
                    }
                }


            }
            else if(dist_sides[3]<=dist_sides[1] &&
                    dist_sides[3]<=dist_sides[2] &&
                    dist_sides[3]<=dist_sides[0])
            {
                //cout<<"4"<<endl;
                //up
                if(m_ball_direction<180)
                {
                    m_ball_direction=m_ball_direction-(2*m_ball_direction-360);

                    //no spawn from above
                }

            }
            while(m_ball_direction>=360) m_ball_direction-=360;
            while(m_ball_direction<0) m_ball_direction+=360;
        }
    }

    //check row of blocks
    for(float y=m_block_height+m_block_gap+m_world_min_y; y<m_world_size[1]; y+=m_block_height*2+m_block_gap)
    {
        int counter=0;
        int counter_target=18;
        for(int i=0;i<(int)m_vec_blocks.size();i++)
        {
            if(m_vec_blocks[i].pos[1]==y)
            {
                counter++;
            }
        }
        if(counter>=counter_target)
        {
            //remove row
            for(int i=0;i<(int)m_vec_blocks.size();i++)
            {
                if(m_vec_blocks[i].pos[1]==y)
                {
                    m_vec_blocks.erase(m_vec_blocks.begin()+i);
                    i--;
                }
            }
        }
    }

    //check if blocks at player level
    for(int i=0;i<(int)m_vec_blocks.size();i++)
    {
        if(m_vec_blocks[i].pos[1]+m_block_height>=m_player_height_level)
        {
            m_gameover=true;
        }
    }

    if(collision_done)
    {
        if(sound_enabled)
        {
            Beep(750, 3);
        }

        //secure move of ball
        float secure_hight=move_ball_down(m_ball_pos[0],m_ball_pos[1]);
        if(secure_hight==-1)
        {
            m_gameover=true;
            if(sound_enabled)
            {
                Beep(400,100);
                Beep(250,150);
            }
        }
        else m_ball_pos[1]=secure_hight;
    }

    return true;
}

bool game::draw()
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    //gamover screen
    if(m_gameover)
    {
        draw_gameover();
        draw_time();

        return false;
    }

    glPushMatrix();

    //draw blocks
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    for(int i=0;i<(int)m_vec_blocks.size();i++)
    {
        glVertex2f(m_vec_blocks[i].pos[0]-m_block_width,m_vec_blocks[i].pos[1]-m_block_height);
        glVertex2f(m_vec_blocks[i].pos[0]-m_block_width,m_vec_blocks[i].pos[1]+m_block_height);
        glVertex2f(m_vec_blocks[i].pos[0]+m_block_width,m_vec_blocks[i].pos[1]+m_block_height);
        glVertex2f(m_vec_blocks[i].pos[0]+m_block_width,m_vec_blocks[i].pos[1]-m_block_height);
    }
    glEnd();

    //draw player
    glPushMatrix();
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glVertex2f(m_player_pos[0]-m_player_width,m_player_pos[1]-m_player_height);
    glVertex2f(m_player_pos[0]-m_player_width,m_player_pos[1]+m_player_height);
    glVertex2f(m_player_pos[0]+m_player_width,m_player_pos[1]+m_player_height);
    glVertex2f(m_player_pos[0]+m_player_width,m_player_pos[1]-m_player_height);
    glEnd();
    glPopMatrix();

    //draw ball
    glPushMatrix();
    glColor3f(1,1,1);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(m_ball_pos[0],m_ball_pos[1]);
    for(int i=0;i<360;i+=36)
    {
        float degInRad = (float)i*_deg2rad;
        glVertex2f(m_ball_pos[0]+cosf(degInRad)*m_ball_size,m_ball_pos[1]+sinf(degInRad)*m_ball_size);
    }
    glVertex2f(m_ball_pos[0]+cosf(360*_deg2rad)*m_ball_size,m_ball_pos[1]+sinf(360*_deg2rad)*m_ball_size);
    glEnd();
    glPopMatrix();

    //draw score timer
    draw_time();

    glPopMatrix();

    return true;
}

bool game::draw_number(char val)
{
    switch(val)
    {
        case '0':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0);
            glVertex2f(0,0.5);

            glVertex2f(0,0.5);
            glVertex2f(0,1);

            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '1':
        {
            glBegin(GL_LINES);
            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);
            glEnd();
        }break;

        case '2':
        {
            glBegin(GL_LINES);
            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(1,0);
            glVertex2f(0,1);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '3':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0.5);
            glVertex2f(1,0.5);

            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '4':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0);
            glVertex2f(0,0.5);

            glVertex2f(0,0.5);
            glVertex2f(1,0.5);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);
            glEnd();
        }break;

        case '5':
        {
            glBegin(GL_LINES);
            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(0,0);
            glVertex2f(1,1);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '6':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0);
            glVertex2f(0,0.5);

            glVertex2f(0,0.5);
            glVertex2f(0,1);

            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(0,0.5);
            glVertex2f(1,0.5);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '7':
        {
            glBegin(GL_LINES);
            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);

            glVertex2f(1,0);
            glVertex2f(0,0);
            glEnd();
        }break;

        case '8':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0);
            glVertex2f(0,0.5);

            glVertex2f(0,0.5);
            glVertex2f(0,1);

            glVertex2f(0,1);
            glVertex2f(1,1);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);

            glVertex2f(1,0);
            glVertex2f(0,0);

            glVertex2f(0,0.5);
            glVertex2f(1,0.5);
            glEnd();
        }break;

        case '9':
        {
            glBegin(GL_LINES);
            glVertex2f(0,0);
            glVertex2f(0,0.5);

            glVertex2f(1,1);
            glVertex2f(1,0.5);

            glVertex2f(1,0.5);
            glVertex2f(1,0);

            glVertex2f(1,0);
            glVertex2f(0,0);

            glVertex2f(0,0.5);
            glVertex2f(1,0.5);
            glEnd();
        }break;

        case ':':
        {
            glBegin(GL_POINTS);
            glVertex2f(0.5,0.75);
            glVertex2f(0.5,0.25);
            glEnd();
        }break;
    }

    return true;
}

bool game::draw_gameover()
{
    glPushMatrix();
    glTranslatef(-80,0,0);
    glColor3f(1,1,1);
    glLineWidth(8);
    glBegin(GL_LINE_STRIP);
    glVertex2f(231,108);
    glVertex2f(135,113);
    glVertex2f(132,206);
    glVertex2f(228,216);
    glVertex2f(242,178);
    glVertex2f(201,173);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(292,204);
    glVertex2f(312,160);
    glVertex2f(354,167);
    glVertex2f(312,160);
    glVertex2f(327,115);
    glVertex2f(381,205);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(412,203);
    glVertex2f(415,125);
    glVertex2f(454,185);
    glVertex2f(496,136);
    glVertex2f(529,198);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(633,227);
    glVertex2f(572,208);
    glVertex2f(564,171);
    glVertex2f(616,167);
    glVertex2f(564,171);
    glVertex2f(555,130);
    glVertex2f(618,109);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(147,294);
    glVertex2f(156,395);
    glVertex2f(263,387);
    glVertex2f(231,281);
    glVertex2f(147,294);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(292,287);
    glVertex2f(336,369);
    glVertex2f(372,302);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(476,383);
    glVertex2f(431,361);
    glVertex2f(419,317);
    glVertex2f(447,321);
    glVertex2f(419,317);
    glVertex2f(409,276);
    glVertex2f(473,284);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2f(543,382);
    glVertex2f(520,281);
    glVertex2f(604,296);
    glVertex2f(534,331);
    glVertex2f(615,385);
    glEnd();
    glLineWidth(1);
    glPopMatrix();

    return true;
}

bool game::draw_time()
{
    stringstream ss1;
    ss1<<m_time_min;
    string string_number1(ss1.str());
    string to_print1;
    if(m_time_min<10) to_print1="0";
    to_print1.append(string_number1);
    stringstream ss2;
    ss2<<m_time_sec;
    string string_number2(ss2.str());
    string to_print2;
    if(m_time_sec<10) to_print2="0";
    to_print2.append(string_number2);

    glPushMatrix();
    glTranslatef(360,m_world_size[1]-30,0);
    glLineWidth(2);
    glColor3f(0.8,0.8,0.8);
    glScalef(15,15,20);
    for(int i=(int)to_print2.size();i>=0;i--)
    {
        draw_number(to_print2[i]);
        glTranslatef(-1.3,0,0);
    }
    glPointSize(3);
    draw_number(':');
    glPointSize(1);
    //glTranslatef(-0.3,0,0);
    for(int i=(int)to_print1.size();i>=0;i--)
    {
        draw_number(to_print1[i]);
        glTranslatef(-1.3,0,0);
    }
    glLineWidth(1);
    glPopMatrix();

    return true;
}

float game::move_ball_down(float x,float y)
{
    //move down
    bool new_pos_found=false;
    float good_y=y;
    for(float new_y=y;new_y<m_world_size[1];new_y+=m_ball_size)
    {
        bool pos_ok=true;
        for(int i=0;i<(int)m_vec_blocks.size();i++)
        {
            if (x < m_vec_blocks[i].pos[0] + m_block_width+m_ball_size &&
                x + m_ball_size+m_block_width > m_vec_blocks[i].pos[0] &&
                new_y < m_vec_blocks[i].pos[1] + m_block_height+m_ball_size &&
                new_y + m_ball_size+m_block_height > m_vec_blocks[i].pos[1])
            {
                //collision
                pos_ok=false;
                break;
            }
        }
        if(pos_ok)
        {
            new_pos_found=true;
            good_y=new_y;
            break;
        }
        else continue;
    }
    if(new_pos_found)
    {
        return good_y;
    }

    return -1;//pos not ok, game over
}


