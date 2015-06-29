#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define FINAL_RADIUS 32
#define MINUTE_RADIUS 132

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0;
static int minute_stroke = 5;
static int border_stroke = 4;
static bool s_animating = true;
static bool s_startup = false;

static TextLayer *s_hour_digit;
static TextLayer *s_day_date;
static char s_day_buffer[12];

static GPath *s_min_path;

static uint8_t face_colours[] = {
	GColorIslamicGreenARGB8,
		GColorInchwormARGB8,
	GColorOrangeARGB8,
		GColorYellowARGB8,
	GColorDarkCandyAppleRedARGB8,
		GColorRedARGB8,
	GColorIndigoARGB8,
		GColorMagentaARGB8,
	GColorDarkGreenARGB8,
		GColorJaegerGreenARGB8,
	GColorBlueMoonARGB8,
		GColorCyanARGB8,
	GColorImperialPurpleARGB8,
		GColorVeryLightBlueARGB8,
	GColorPurpleARGB8,
		GColorShockingPinkARGB8,
	GColorDarkGrayARGB8,
		GColorLightGrayARGB8,
	GColorWindsorTanARGB8,
		GColorChromeYellowARGB8,
	GColorArmyGreenARGB8,
		GColorLimerickARGB8,
	GColorCobaltBlueARGB8,
		GColorCadetBlueARGB8,
	GColorElectricUltramarineARGB8,
		GColorBabyBlueEyesARGB8,
	GColorJazzberryJamARGB8,
		GColorFashionMagentaARGB8,
	GColorOxfordBlueARGB8,
		GColorLibertyARGB8,
	GColorDukeBlueARGB8,
		GColorVividCeruleanARGB8,
	GColorSunsetOrangeARGB8,
		GColorMelonARGB8,
	GColorVividVioletARGB8,
		GColorRichBrilliantLavenderARGB8,
	GColorLavenderIndigoARGB8,
		GColorBrilliantRoseARGB8,
	GColorTiffanyBlueARGB8,
		GColorElectricBlueARGB8,
	GColorRoseValeARGB8,
		GColorFollyARGB8,
	GColorMayGreenARGB8,
		GColorMediumAquamarineARGB8,
	GColorMalachiteARGB8,
		GColorSpringBudARGB8,
	GColorKellyGreenARGB8,
		GColorMintGreenARGB8,
	GColorBulgarianRoseARGB8,
		GColorPurpureusARGB8
};


static int s_colour_a = -1;
static int s_colour_b;
static uint8_t main_colour;
static uint8_t minute_colour;

/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  
	if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

	if (s_last_time.minutes == 0 || s_animating) {
		
		if (s_animating && s_colour_a == -1) {
			// start color index randomly
			s_colour_a = rand() % 49;
		}
		else if (!s_animating) {
			s_colour_a++;
		}
		
		if (s_colour_a > 49) {
			s_colour_a = 0;
		}
		
		s_colour_b = s_colour_a + 1;
		
		if (s_colour_b > 49) {
			s_colour_b = 0;
		}
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d", s_colour_a, s_colour_b);
	}
	
  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}



static void update_proc(Layer *layer, GContext *ctx) {
	// Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;
	
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);

  // hour text
	if (!s_animating) {
		// Create a long-lived buffer
		static char buffer[] = "00";

		// Write the current hours and minutes into the buffer
		if(clock_is_24h_style() == true) {
			// Use 24 hour format
			strftime(buffer, sizeof("00"), "%H", tick_time);
		} else {
			// Use 12 hour format
			strftime(buffer, sizeof("00"), "%I", tick_time);
		}

		if (!clock_is_24h_style() && mode_time.hours > 0 && mode_time.hours < 10) {
			// remove the leading "0""
			snprintf(buffer, 2, "%d", mode_time.hours);
		}
		else if (!clock_is_24h_style() && mode_time.hours == 0) {
			// stupid time, of course we want "12"
			snprintf(buffer, 4, "%s", "12");
		}

		// Display this time on the TextLayer
		text_layer_set_text(s_hour_digit, buffer);
	}
	
  // Color background?
  if(COLORS) {
    //graphics_context_set_fill_color(ctx, GColorFromRGB(s_color_channels[0], s_color_channels[1], s_color_channels[2]));
	main_colour = face_colours[s_colour_a];
	graphics_context_set_fill_color(ctx, (GColor)main_colour);
    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  }

  graphics_context_set_stroke_color(ctx, GColorBlack);
  

  graphics_context_set_antialiased(ctx, ANTIALIASING);
	
	
	// Plot hands
	GPoint minute_hand = (GPoint) {
		.x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)MINUTE_RADIUS / TRIG_MAX_RATIO) + s_center.x,
		.y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)MINUTE_RADIUS / TRIG_MAX_RATIO) + s_center.y,
	};

	
	// Fill in minute color (aka s_colour_b)
	GPathInfo s_min_points =  {
		  .num_points = 3,
		  .points = (GPoint []) {{72, -1}, {72, 84}, {minute_hand.x, minute_hand.y}}
		};
	
	if (mode_time.minutes > 7 && mode_time.minutes < 23) {
		// add in the top-right corner
		s_min_points.num_points = 4;
		s_min_points.points = (GPoint []) {{72, -1}, {72, 84}, {minute_hand.x, minute_hand.y}, {145, -1}};
	}
	else if (mode_time.minutes > 22 && mode_time.minutes < 38) {
		// add in bottom-right and top-right
		s_min_points.num_points = 5,
		s_min_points.points = (GPoint []) {{72, -1}, {72, 84}, {minute_hand.x, minute_hand.y}, {145, 169}, {145, -1}};
	}
	else if (mode_time.minutes > 37 && mode_time.minutes < 52) {
		// bottom-left, bottom-right & top-right
		 s_min_points.num_points = 6,
		s_min_points.points = (GPoint []) {{72, -1}, {72, 84}, {minute_hand.x, minute_hand.y}, {-1, 169}, {145, 169}, {145, -1}};
	}
	else if (mode_time.minutes > 51 && mode_time.minutes <= 59) {
		//*/ going negative seems to help along the left edge - draw to all four corners
		s_min_points.num_points = 7,
		s_min_points.points = (GPoint []) {{72, -1}, {72, 84}, {minute_hand.x, minute_hand.y}, {-1, -1}, {-1, 169}, {145, 169}, {145, -1}};
	}
	
	// set up
	s_min_path = gpath_create(&s_min_points);
	
	// fill in minute area
	minute_colour = face_colours[s_colour_b];
	graphics_context_set_fill_color(ctx, (GColor)minute_colour);
  	gpath_draw_filled(ctx, s_min_path);
	
	// Draw white circle outline
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, minute_stroke + border_stroke);
  	graphics_draw_circle(ctx, s_center, s_radius);
	
	// draw minute line
	if (!s_animating) {
		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_stroke_width(ctx, minute_stroke + border_stroke);
		graphics_draw_line(ctx, s_center, minute_hand);
		
		graphics_context_set_stroke_color(ctx, GColorBlack);
		graphics_context_set_stroke_width(ctx, minute_stroke);
		graphics_draw_line(ctx, s_center, minute_hand);
	}
	
	
	
	
	// White clockface
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, s_center, s_radius);
	
  	
	// draw black circle outline
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, minute_stroke);
  	graphics_draw_circle(ctx, s_center, s_radius);

	
	// date box
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_stroke_width(ctx, 1);
	graphics_fill_rect(ctx, GRect(40, 131, 64, 24), 10, GCornersAll);
	graphics_draw_round_rect(ctx, GRect(40, 131, 64, 24), 10);
	strftime(s_day_buffer, sizeof("ddd dd"), "%a %d", tick_time);
	text_layer_set_text(s_day_date, s_day_buffer);
	
	s_startup = false;
	
}




static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
	
	// Create time TextLayer
	s_hour_digit = text_layer_create(GRect(41, 57, 62, 50));
	text_layer_set_background_color(s_hour_digit, GColorClear);
	text_layer_set_text_color(s_hour_digit, GColorBlack);
	
	// create day/date layer
	s_day_date = text_layer_create(GRect(40, 130, 64, 24));
	text_layer_set_background_color(s_day_date, GColorClear);
	text_layer_set_text_color(s_day_date, GColorBlack);
	
	// Improve the layout to be more like a watchface
	text_layer_set_font(s_hour_digit, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_alignment(s_hour_digit, GTextAlignmentCenter);
	
	text_layer_set_font(s_day_date, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD ));
	text_layer_set_text_alignment(s_day_date, GTextAlignmentCenter);

	// Add it as a child layer to the Window's root layer
	layer_add_child(s_canvas_layer, text_layer_get_layer(s_hour_digit));
	layer_add_child(s_canvas_layer, text_layer_get_layer(s_day_date));
	
}

static void window_unload(Window *window) {
  	text_layer_destroy(s_hour_digit);
	text_layer_destroy(s_day_date);
	layer_destroy(s_canvas_layer);
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  //s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Prepare animations
  AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
	
	
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
