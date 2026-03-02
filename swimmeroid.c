#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#define BUFFER_SIZE 1024
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NOISE_SENSITIVE 0.1f
/* Do i need to commit the font? I guess u can download one yourself */
#define TTF_FILE_NAME "TerminessNerdFont-Bold.ttf"
#define TTF_SIZE 24
#define SPRITE_SETUP_PADDING 50

#define WHITE_COLOR (SDL_Color){.r = 255, .g = 255, .b = 255, .a = SDL_ALPHA_OPAQUE }

static float buffer[BUFFER_SIZE];
/* static bool noisy_enough = false; */

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_Texture* user_avatar = NULL;
static TTF_Font *font;
/* static TTF_Text *welcome_text; */
static SDL_Texture* text;
static SDL_FRect sprite_face_size;
static SDL_FRect sprite_normal_mode_silent;
static bool is_silent_mode_defined;
static bool noisy_enough;
static SDL_FRect sprite_normal_mode_sound;
static SDL_FRect window_size_rect = {
	.x = 0,
	.y = 0,
	.w = WINDOW_WIDTH,
	.h = WINDOW_HEIGHT
};

typedef enum {
	STARTING,
	SELECTING,
	SETUP,
	SHOW,
	DONE
} sw_state;

static sw_state state;

static void init_window(void){
	window = SDL_CreateWindow("Swimmeroid", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_BORDERLESS);
	renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
	SDL_RenderClear(renderer);
}

static SDL_Texture* load_texture(const char* file_path) {
	SDL_Surface* tmp_surface = SDL_LoadPNG(file_path);
	SDL_Texture* tmp_texture = SDL_CreateTextureFromSurface(renderer, tmp_surface);
	if(!tmp_surface){
		SDL_Log("[ERROR] surface not created in load_texture method %s", SDL_GetError());
	}
	SDL_DestroySurface(tmp_surface);
	return tmp_texture;
}

static void init_avatar_asset(const char* file_path){
	user_avatar = load_texture(file_path);
	if(!user_avatar){
		SDL_Log("[ERROR] no ce podi cargar seu asset, rasao: %s", SDL_GetError());
	} else {
		state = SETUP;
		SDL_Log("[INFO] avatar asset loaded");
	}
}

static SDL_Texture * create_texture_for_string(const char* message, SDL_Color color);

static void clear_opaque(void){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

static void clear_transparent(void){
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_TRANSPARENT);
	SDL_RenderClear(renderer);
}

static void display_welcome_message(void){
	clear_opaque();
	SDL_FRect dst;
	SDL_GetTextureSize(text, &dst.w, &dst.h);
	SDL_RenderTexture(renderer, text, NULL, &dst);
	SDL_Texture* instruction_texture = create_texture_for_string("drop a png file to use as avatar", WHITE_COLOR);
	dst.y = 30;
	SDL_GetTextureSize(instruction_texture, &dst.w, &dst.h);
	SDL_RenderTexture(renderer, instruction_texture, NULL, &dst);
	SDL_DestroyTexture(instruction_texture);
}

static void display_sprite_picker(void){
	SDL_Log("picker page implementation");
	SDL_Delay(10*1000);
}

static void draw_image_on_setup(void){
	SDL_FRect image_texture_size;
	SDL_GetTextureSize(user_avatar, &image_texture_size.w, &image_texture_size.h);
	image_texture_size.x = SPRITE_SETUP_PADDING;
	image_texture_size.y = SPRITE_SETUP_PADDING;
	SDL_RenderTexture(renderer, user_avatar, NULL, &image_texture_size);
}

static void draw_image_sprite_selector(void){
	SDL_GetTextureSize(user_avatar, &sprite_face_size.w, &sprite_face_size.h);
	sprite_face_size.w = sprite_face_size.w / 2;
	if(!is_silent_mode_defined){
		sprite_face_size.x = SPRITE_SETUP_PADDING;
	} else{
		sprite_face_size.x = SPRITE_SETUP_PADDING + (sprite_face_size.w);
	}

	sprite_face_size.y = SPRITE_SETUP_PADDING;
	SDL_SetRenderDrawColor(renderer, 255, 100, 100, SDL_ALPHA_OPAQUE);
	SDL_RenderRect(renderer, &sprite_face_size);
}

static void display_sprite_setup(void){
	clear_opaque();
	char * message;
	if(is_silent_mode_defined){
		message = "pick sprite normal mode (silent)";
	} else {
		message = "pick sprite normal mode (sound)";
	}
	SDL_Texture* instruction_texture = create_texture_for_string(message, WHITE_COLOR);
	SDL_FRect dst;
	SDL_GetTextureSize(instruction_texture, &dst.w, &dst.h);
	SDL_RenderTexture(renderer, instruction_texture, NULL, &dst);

	draw_image_on_setup();
	draw_image_sprite_selector();
	SDL_DestroyTexture(instruction_texture);
}

static void check_audio_freq(){
	noisy_enough = false;
	for(size_t i = 0; i < BUFFER_SIZE; i++){
		if(buffer[i] > NOISE_SENSITIVE){
			noisy_enough = true;
		}
	}
}

static void display_user_avatar(){
	clear_transparent();
	if(noisy_enough){
		SDL_RenderTexture(renderer, user_avatar, &sprite_normal_mode_sound, &window_size_rect);
	} else {
		SDL_RenderTexture(renderer, user_avatar, &sprite_normal_mode_silent, &window_size_rect);
	}
}

static void display_swimmeroid(){
	check_audio_freq();
	display_user_avatar();
}

static void init_font(void){
	if(!TTF_Init()){
		SDL_Log("couldnt init truetype fonts %s", SDL_GetError());
	}
	font = TTF_OpenFont(TTF_FILE_NAME, TTF_SIZE);
	SDL_Color color = {.r = 255, .g = 255, .b = 255, .a = SDL_ALPHA_OPAQUE};
	text = create_texture_for_string("Welcome to Swimmeroid!", color);
	if(!text){
		SDL_Log("ye we really succ %s", SDL_GetError());
	}
}

/**
 * remember always destroy your textures,
 * it can be serious problem if u keep them
 */
static SDL_Texture * create_texture_for_string(const char* message, SDL_Color color){

	SDL_Surface* surface = TTF_RenderText_Solid(font, message, SDL_strlen(message), color);
	if(!surface){
		SDL_Log("[ERROR] could not create surface text we succ %s", SDL_GetError());
	}
	SDL_Texture* le_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);
	if(!le_texture){
		SDL_Log("[ERROR] could not create texture we succ %s", SDL_GetError());
	}
	return le_texture;
}

static void load_file_dropped(const char* file_path){
	char* file_path_includes_png = SDL_strstr(file_path, ".png");
	if(!file_path_includes_png){
		SDL_Log("file not a png file");
	} else {
		init_avatar_asset(file_path);
	}
	SDL_Log(file_path);
}

static void handle_key_press(unsigned long key){
	switch(key){
		case SDLK_RETURN:
			SDL_Log("return key pressed");
			if(state == SETUP){
				if(!is_silent_mode_defined) {
					sprite_face_size.x -= SPRITE_SETUP_PADDING;
					sprite_face_size.y -= SPRITE_SETUP_PADDING;
					sprite_normal_mode_silent = sprite_face_size;
					is_silent_mode_defined = true;
				} else {
					sprite_face_size.x -= SPRITE_SETUP_PADDING;
					sprite_face_size.y -= SPRITE_SETUP_PADDING;
					sprite_normal_mode_sound = sprite_face_size;
					state = SHOW;
				}
			}
			break;
	}
}

static void update_based_on_state(sw_state state){
	switch(state){
		case STARTING:
			display_welcome_message();
			break;
		case SELECTING:
			display_sprite_picker();
			break;
		case SETUP:
			display_sprite_setup();
			break;
		case SHOW:
			display_swimmeroid();
			break;
		case DONE:
			break;
	}
}

static void file_dialog_callback(void *userdata, const char * const *filelist, int filter){
	(void)(userdata);
	(void)(filter);
	if(filelist[0]){
		load_file_dropped(filelist[0]);
	}
}

int main(int argc, char* argv[]){
	(void)(argc);
	(void)(argv);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	init_window();
	init_font();

	SDL_AudioSpec want;
	SDL_zero(want);
	want.format = SDL_AUDIO_F32;
	want.channels = 2;
	want.freq = 48000;

	SDL_AudioDeviceID recording = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &want);

	int freq;
	if(!SDL_GetAudioDeviceFormat(recording, &want, &freq)){
		SDL_Log("[ERROR] cant get audio device format: %s", SDL_GetError());
	}

	SDL_AudioStream *r_stream = SDL_CreateAudioStream(&want, &want);

	if(!SDL_BindAudioStream(recording, r_stream)){
		SDL_Log("[ERROR] cant bind device to stream, %s", SDL_GetError());
	}

	SDL_Event event;

	int iteration_count = 0;
	int iteration_rate = 8;
	while(state != DONE){

		SDL_Delay(8);
		int len = SDL_GetAudioStreamData(r_stream, buffer, sizeof(buffer));
		SDL_FlushAudioStream(r_stream);
		(void)(len);
		if(iteration_count >= iteration_rate){
			update_based_on_state(state);
		}
		iteration_count++;

		if(SDL_PollEvent(&event)){
			if(event.type == SDL_EVENT_QUIT){
				state = DONE;
				break;
			}
			if(SDL_EVENT_KEY_DOWN == event.type){
				handle_key_press(event.key.key);
			}
			if(SDL_EVENT_DROP_FILE == event.type){
				load_file_dropped(event.drop.data);
				SDL_Log("new file droped!");
			}
			if(SDL_EVENT_MOUSE_BUTTON_DOWN == event.type){
				if(state == STARTING){
					SDL_DialogFileFilter png_filter[1];
					png_filter[0] = (SDL_DialogFileFilter){
						.name = "only-png",
						.pattern = ".png"
					};
					SDL_ShowOpenFileDialog(&file_dialog_callback, NULL, window, 
							png_filter, 1, NULL, false);
				}
			}
		}
		SDL_RenderPresent(renderer);
	}
	SDL_FlushAudioStream(r_stream);
	SDL_DestroyTexture(text);
	TTF_CloseFont(font);
	TTF_Quit();
}
