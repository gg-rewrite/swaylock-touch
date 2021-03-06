#ifndef _SWAYLOCK_H
#define _SWAYLOCK_H
#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>
#include "background-image.h"
#include "cairo.h"
#include "pool-buffer.h"
#include "seat.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

enum auth_state {
	AUTH_STATE_INPUT,
	AUTH_STATE_VALIDATING,
	AUTH_STATE_INVALID,
};

enum render_state {
	RENDER_STATE_INITIAL,
	RENDER_STATE_PIN,
};

enum touch_event {
	TOUCH_EVENT_DOWN,
	TOUCH_EVENT_UP,
	TOUCH_EVENT_MOTION,
};

struct swaylock_colorset {
	uint32_t input;
	uint32_t verifying;
	uint32_t wrong;
};

struct swaylock_colors {
	uint32_t background;
	uint32_t separator;
	uint32_t input_text;
	uint32_t button_background;
	uint32_t button_background_pressed;
	uint32_t button_border;
	uint32_t button_text;
	struct swaylock_colorset line;
	struct swaylock_colorset text;
};

struct swaylock_args {
	struct swaylock_colors colors;
	enum background_mode mode;
	char *font;
	uint32_t font_size;
	uint32_t thickness;
	bool ignore_empty;
	bool show_failed_attempts;
	bool daemonize;
};

struct swaylock_password {
	size_t len;
	char buffer[1024];
};

struct swaylock_touch {
	bool pressed;
	uint32_t x;
	uint32_t y;
  int32_t current_pressed;
  uint32_t buttons_area_width, buttons_area_height;
  uint32_t button_spacing, button_width, button_height;
  uint32_t text_area_height;
  
};

struct swaylock_state {
	struct loop *eventloop;
	struct loop_timer *clear_password_timer; // clears the password buffer
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_subcompositor *subcompositor;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zwlr_input_inhibit_manager_v1 *input_inhibit_manager;
 	struct wl_shm *shm;
	struct wl_list surfaces;
	struct wl_list images;
	struct swaylock_args args;
	struct swaylock_password password;
	struct swaylock_xkb xkb;
	struct swaylock_touch touch;
	enum auth_state auth_state;
	enum render_state render_state;
	int failed_attempts;
	bool run_display;
	struct zxdg_output_manager_v1 *zxdg_output_manager;
};

struct swaylock_surface {
	cairo_surface_t *image;
	struct swaylock_state *state;
	struct wl_output *output;
	uint32_t output_global_name;
	struct zxdg_output_v1 *xdg_output;
	struct wl_surface *surface;
	struct wl_surface *child; // surface made into subsurface
	struct wl_subsurface *subsurface;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct pool_buffer buffers[2];
	struct pool_buffer indicator_buffers[2];
	struct pool_buffer *current_buffer;
	bool frame_pending, dirty;
	uint32_t width, height;
	uint32_t indicator_width, indicator_height;
	int32_t scale;
	enum wl_output_subpixel subpixel;
	char *output_name;
	struct wl_list link;
};

// There is exactly one swaylock_image for each -i argument
struct swaylock_image {
	char *path;
	char *output_name;
	cairo_surface_t *cairo_surface;
	struct wl_list link;
};

void swaylock_handle_key(struct swaylock_state *state, xkb_keysym_t keysym,
			 uint32_t codepoint);
void swaylock_handle_touch(struct swaylock_state *state, enum touch_event event,
			   int x, int y);
int32_t swaylock_touch_key_pressed(struct swaylock_touch *touch);
void swaylock_touch_recalculate_keys(struct swaylock_state *state, uint32_t new_width,
			    uint32_t new_height);
void render_frame_background(struct swaylock_surface *surface);
void render_frame(struct swaylock_surface *surface);
void render_frames(struct swaylock_state *state);
void damage_surface(struct swaylock_surface *surface);
void damage_state(struct swaylock_state *state);
void clear_password_buffer(struct swaylock_password *pw);

void initialize_pw_backend(int argc, char **argv);
void run_pw_backend_child(void);
void clear_buffer(char *buf, size_t size);

#endif
