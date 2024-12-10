#include "Hero.h"
#include "data/DataCenter.h"
#include "data/GIFCenter.h"
#include "algif5/algif.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include <stdio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro.h>
namespace HeroSetting
{
    static constexpr char gif_root_path[50] = "./assets/gif/Hero";
    static constexpr char gif_postfix[][10] = {
        "left",
        "right",
        "front",
        "back",
    };
}
namespace {
    constexpr double HP_DEDUCTION_INTERVAL = 2.0; // 每 2 秒扣一次血
}

void Hero::init()
{
    for (size_t type = 0; type < static_cast<size_t>(HeroState::HEROSTATE_MAX); ++type)
    {
        char buffer[50];
        sprintf(
            buffer, "%s/dragonite_%s.gif",
            HeroSetting::gif_root_path,
            HeroSetting::gif_postfix[static_cast<int>(type)]);
        gifPath[static_cast<HeroState>(type)] = std::string{buffer};
    }
    DataCenter *DC = DataCenter::get_instance();
    GIFCenter *GIFC = GIFCenter::get_instance();
    ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);
    width = gif->width;
    height = gif->height;
    shape.reset(new Rectangle{DC->window_width / 2,
                              DC->window_height / 2,
                              DC->window_width / 2 + width,
                              DC->window_height / 2 + height});
     // 載入金幣圖片
    coin_image = al_load_bitmap("./assets/image/coin.png");
    if (!coin_image) {
        fprintf(stderr, "Failed to load coin image!\n");
    }
}

void Hero::update()
{
    DataCenter *DC = DataCenter::get_instance();
    if (DC->key_state[ALLEGRO_KEY_W])
    {
        shape->update_center_y(shape->center_y() - speed);
        state = HeroState::BACK;
    }
    else if (DC->key_state[ALLEGRO_KEY_A])
    {
        shape->update_center_x(shape->center_x() - speed);
        state = HeroState::LEFT;
    }
    else if (DC->key_state[ALLEGRO_KEY_S])
    {
        shape->update_center_y(shape->center_y() + speed);
        state = HeroState::FRONT;
    }
    else if (DC->key_state[ALLEGRO_KEY_D])
    {
        shape->update_center_x(shape->center_x() + speed);
        state = HeroState::RIGHT;
    }
    //12/2，玩家超出右邊界則扣血
     bool is_out_of_screen = shape->center_x() + width / 2 >= DC->window_width ||  // 超出右邊界
                            shape->center_x() - width / 2 < 0;                   // 超出左邊界

    static double elapsed_time = 0.0; // 靜態變數累積時間

    if (is_out_of_screen) {
        // 12/2 如果角色超出銀幕範圍，累積時間
        elapsed_time += 1.0 / DC->FPS;

        if (elapsed_time >= HP_DEDUCTION_INTERVAL) {
            // 時間到，扣除玩家 HP
            if (DC->player->HP > 0) {
                DC->player->HP--;
            }
            elapsed_time = 0.0; // 重置計時器
        }
    } else {
        // 如果角色在銀幕範圍內，重置計時器
        elapsed_time = 0.0;
    }
    
}
void Hero::draw()
{
    GIFCenter *GIFC = GIFCenter::get_instance();
    ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);
    algif_draw_gif(gif,
                   shape->center_x() - width / 2,
                   shape->center_y() - height / 2,
                   0);
    //  GIFCenter *GIFC = GIFCenter::get_instance();
    // ALGIF_ANIMATION *gif = GIFC->get(gifPath[state]);

    // 繪製角色
    // algif_draw_gif(gif,
    //                shape->center_x() - width / 2,
    //                shape->center_y() - height / 2,
    //                0);

    // 繪製金幣
    if (coin_image) {
        al_draw_bitmap(
            coin_image,
            shape->center_x() + coin_offset_x, // 金幣的 x 座標
            shape->center_y() + coin_offset_y, // 金幣的 y 座標
            0
        );
    }

}

