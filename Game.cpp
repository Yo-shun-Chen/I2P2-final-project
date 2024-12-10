#include "Game.h"
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/OperationCenter.h"
#include "data/SoundCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "Player.h"
#include "Level.h"
/*-----I2P Revise start-----*/
#include "Hero.h"
/*-----I2P Revise end-----*/
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <vector>
#include <cstring>
#include <allegro5/allegro_image.h>


// fixed settings
constexpr char game_icon_img_path[] = "./assets/image/game_icon.png";
constexpr char game_start_sound_path[] = "./assets/sound/growl.wav";
constexpr char background_img_path[] = "./assets/image/StartBackground.jpg";
constexpr char background_sound_path[] = "./assets/sound/BackgroundMusic.ogg";

/**
 * @brief Game entry.
 * @details The function processes all allegro events and update the event state to a generic data storage (i.e. DataCenter).
 * For timer event, the game_update and game_draw function will be called if and only if the current is timer.
 */
void Game::execute()
{
	DataCenter *DC = DataCenter::get_instance();
	// main game loop
	bool run = true;
	while (run)
	{
		// process all events here
		al_wait_for_event(event_queue, &event);
		switch (event.type)
		{
		case ALLEGRO_EVENT_TIMER:
		{
			run &= game_update();
			game_draw();
			break;
		}
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
		{ // stop game
			run = false;
			break;
		}
		case ALLEGRO_EVENT_KEY_DOWN:
		{
			DC->key_state[event.keyboard.keycode] = true;
			break;
		}
		case ALLEGRO_EVENT_KEY_UP:
		{
			DC->key_state[event.keyboard.keycode] = false;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_AXES:
		{
			DC->mouse.x = event.mouse.x;
			DC->mouse.y = event.mouse.y;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		{
			DC->mouse_state[event.mouse.button] = true;
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		{
			DC->mouse_state[event.mouse.button] = false;
			break;
		}
		default:
			break;
		}
	}
}

/**
 * @brief Initialize all allegro addons and the game body.
 * @details Only one timer is created since a game and all its data should be processed synchronously.
 */
Game::Game()
{
	DataCenter *DC = DataCenter::get_instance();
	GAME_ASSERT(al_init(), "failed to initialize allegro.");

	// initialize allegro addons
	bool addon_init = true;
	addon_init &= al_init_primitives_addon();
	addon_init &= al_init_font_addon();
	addon_init &= al_init_ttf_addon();
	addon_init &= al_init_image_addon();
	addon_init &= al_init_acodec_addon();
	GAME_ASSERT(addon_init, "failed to initialize allegro addons.");

	// initialize events
	bool event_init = true;
	event_init &= al_install_keyboard();
	event_init &= al_install_mouse();
	event_init &= al_install_audio();
	GAME_ASSERT(event_init, "failed to initialize allegro events.");

	// initialize game body
	GAME_ASSERT(
		display = al_create_display(DC->window_width, DC->window_height),
		"failed to create display.");
	GAME_ASSERT(
		timer = al_create_timer(1.0 / DC->FPS),
		"failed to create timer.");
	GAME_ASSERT(
		event_queue = al_create_event_queue(),
		"failed to create event queue.");

	debug_log("Game initialized.\n");
	game_init();
}

/**
 * @brief Initialize all auxiliary resources.
 */
void Game::game_init()
{
	DataCenter *DC = DataCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	// set window icon
	game_icon = IC->get(game_icon_img_path);
	al_set_display_icon(display, game_icon);

	// register events to event_queue
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_mouse_event_source());
	al_register_event_source(event_queue, al_get_timer_event_source(timer));

	// init sound setting
	SC->init();

	// init font setting
	FC->init();

	ui = new UI();
	ui->init();

	DC->level->init();

	/*-----I2P Revise start-----*/
	DC->hero->init();
	/*-----I2P Revise end-----*/

	// game start
	background = IC->get(background_img_path);
	debug_log("Game state: change to START\n");
	state = STATE::START;
	al_start_timer(timer);
	// 新增障礙物圖片變數
	
}

/**
 * @brief The function processes all data update.
 * @details The behavior of the whole game body is determined by its state.
 * @return Whether the game should keep running (true) or reaches the termination criteria (false).
 * @see Game::STATE
 */
bool Game::game_update()
{
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	SoundCenter *SC = SoundCenter::get_instance();
	static ALLEGRO_SAMPLE_INSTANCE *background = nullptr;

	switch (state)
	{
	case STATE::START:
	{
		static bool is_played = false;
		static ALLEGRO_SAMPLE_INSTANCE *instance = nullptr;
		if (!is_played)
		{
			instance = SC->play(game_start_sound_path, ALLEGRO_PLAYMODE_ONCE);
			DC->level->load_level(1);
			is_played = true;
		}

		if (!SC->is_playing(instance))
		{
			debug_log("<Game> state: change to LEVEL\n");
			state = STATE::LEVEL;
		}
		break;
	}
	case STATE::LEVEL:
	{
		if (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P])
		{
			SC->toggle_playing(background);
			debug_log("<Game> state: change to PAUSE\n");
			state = STATE::PAUSE;
		}
		if (DC->level->remain_monsters() == 0 && DC->monsters.size() == 0)
		{
			debug_log("<Game> state: change to END\n");
			state = STATE::END;
		}
		if (DC->player->HP == 0)
		{
			debug_log("<Game> state: change to END\n");
			state = STATE::END;
		}
		static bool BGM_played = false;
		if (!BGM_played)
		{
			background = SC->play(background_sound_path, ALLEGRO_PLAYMODE_LOOP);
			BGM_played = true;
		}
		break;
	}
	case STATE::PAUSE:
	{
		if (DC->key_state[ALLEGRO_KEY_P] && !DC->prev_key_state[ALLEGRO_KEY_P])
		{
			SC->toggle_playing(background);
			debug_log("<Game> state: change to LEVEL\n");
			state = STATE::LEVEL;
		}
		break;
	}
	case STATE::END:
	{
		return false;
	}
	}
	// If the game is not paused, we should progress update.
	if (state != STATE::PAUSE)
	{
		DC->player->update();
		SC->update();
		ui->update();
		/*-----I2P Revise start-----*/
		DC->hero->update();
		/*-----I2P Revise end-----*/
		if (state != STATE::START)
		{
			DC->level->update();
			OC->update();
		}
	}
	// game_update is finished. The states of current frame will be previous states of the next frame.
	memcpy(DC->prev_key_state, DC->key_state, sizeof(DC->key_state));
	memcpy(DC->prev_mouse_state, DC->mouse_state, sizeof(DC->mouse_state));
	return true;
}

/**
 * @brief Draw the whole game and objects.
 */
void Game::game_draw()
{
	DataCenter *DC = DataCenter::get_instance();
	OperationCenter *OC = OperationCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();

	// Flush the screen first.
	al_clear_to_color(al_map_rgb(100, 100, 100));
	if (state != STATE::END)
	{
		// background
		al_draw_bitmap(background, 0, 0, 0);
		if (DC->game_field_length < DC->window_width)
			al_draw_filled_rectangle(
				DC->game_field_length, 0,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		if (DC->game_field_length < DC->window_height)
			al_draw_filled_rectangle(
				0, DC->game_field_length,
				DC->window_width, DC->window_height,
				al_map_rgb(100, 100, 100));
		// user interface
		if (state != STATE::START)
		{
			DC->level->draw();
			/*-----I2P Revise start-----*/
			DC->hero->draw();
			/*-----I2P Revise end-----*/
			ui->draw();
			OC->draw();
		}
		 // 新增槍的圖片繪製
        ALLEGRO_BITMAP *gun_image = al_load_bitmap("./assets/image/gun.png");
        if (gun_image) {
            int gun_width = al_get_bitmap_width(gun_image);
            int gun_height = al_get_bitmap_height(gun_image);
            al_draw_bitmap(
                gun_image,
                DC->window_width - gun_width, // 放在最右邊
                (DC->window_height - gun_height) / 2, // 垂直置中
                0
            );
			 // 設定旋轉中心 (圖片的中點)
            float center_x = gun_width / 2.0f;
            float center_y = gun_height / 2.0f;

            // 旋轉後繪製的位置 (將圖片移到螢幕最右側並垂直置中)
            float dest_x = DC->window_width - center_y; // 圖片旋轉後的寬度改變
            float dest_y = DC->window_height / 2.0f;

            // 向左旋轉 90 度 (角度使用弧度)
            float rotation_angle = -ALLEGRO_PI / 2;

            // 繪製旋轉後的圖片
            al_draw_rotated_bitmap(
                gun_image,
                center_x, center_y,   // 圖片的旋轉中心
                dest_x, dest_y,       // 繪製的目標中心點
                rotation_angle,       // 旋轉角度
                0                     // 無附加標誌
            );
            al_destroy_bitmap(gun_image); // 釋放記憶體
        }
		ALLEGRO_BITMAP *obstacle_image = al_load_bitmap("./assets/image/obstacle.jpg");

		// // 在 game_init 函式中載入障礙物圖片
		// obstacle_image = al_load_bitmap("./assets/image/obstacle.jpg");
		// if (!obstacle_image) {
		// 	fprintf(stderr, "Failed to load obstacle image!\n");
		// }
		if (obstacle_image) {
			int obstacle_width = al_get_bitmap_width(obstacle_image);
			int obstacle_height = al_get_bitmap_height(obstacle_image);

			// 設定障礙物的位置，例如在螢幕中央
			int obstacle_x = (DC->window_width - obstacle_width) / 2;
			int obstacle_y = (DC->window_height - obstacle_height) / 2-200;

			//al_draw_bitmap(obstacle_image, obstacle_x, obstacle_y, 0);
	}
	if (obstacle_image) {
        int obstacle_width = al_get_bitmap_width(obstacle_image);
        int obstacle_height = al_get_bitmap_height(obstacle_image);

        // 縮放比例 (例如縮小到 50%)
        float scale_factor = 0.1; 
        float scaled_width = obstacle_width * scale_factor;
        float scaled_height = obstacle_height * scale_factor;

        // 設定繪製位置 (例如畫面中央)
        float dest_x = (DC->window_width - scaled_width) / 2;
        float dest_y = (DC->window_height - scaled_height) / 2+100;

        al_draw_scaled_bitmap(
            obstacle_image,
            0, 0,                      // 原始圖片區域 (全圖)
            obstacle_width, obstacle_height, // 原始大小
            dest_x, dest_y,            // 繪製位置
            scaled_width, scaled_height, // 縮放後大小
            0                          // 無附加標誌
        );
    }


	}
	switch (state)
	{
	case STATE::START:
	{
	}
	case STATE::LEVEL:
	{
		break;
	}
	case STATE::PAUSE:
	{
		// game layout cover
		al_draw_filled_rectangle(0, 0, DC->window_width, DC->window_height, al_map_rgba(50, 50, 50, 64));
		al_draw_text(
			FC->caviar_dreams[FontSize::LARGE], al_map_rgb(255, 255, 255),
			DC->window_width / 2., DC->window_height / 2.,
			ALLEGRO_ALIGN_CENTRE, "GAME PAUSED");
		break;
	}
	case STATE::END:
	{
	}
	}
	
	al_flip_display();
}

Game::~Game()
{
	al_destroy_display(display);
	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}