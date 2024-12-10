#ifndef HERO_H_INCLUDED
#define HERO_H_INCLUDED
//#ifndef ALLEGRO_BITMAP
#include <string>
#include <map>
#include "Object.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro.h>  

enum class HeroState
{
    LEFT,
    RIGHT,
    FRONT,
    BACK,
    HEROSTATE_MAX
};

class Hero : public Object
{

public:
    void init();
    void update();
    void draw();

private:
    HeroState state = HeroState::FRONT; // the state of character
    double speed = 5;                   // the move speed of hero
    int width, height;                  // the width and height of the hero image
    std::map<HeroState, std::string> gifPath;
private:
    ALLEGRO_BITMAP *coin_image = nullptr; // 儲存金幣圖片
    int coin_offset_x = 50; // 金幣相對於玩家的水平偏移
    int coin_offset_y = 50;  // 金幣相對於玩家的垂直偏移
};
#endif