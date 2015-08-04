#include <pebble.h>

#define COLORS       true
#define ANTIALIASING true

#define FINAL_RADIUS 32
#define MINUTE_RADIUS 132

#define ANIMATION_DURATION 500
#define ANIMATION_DELAY    600

// Storage Keys
#define KEY_OUTLINE_COLOUR 0
#define KEY_BATTERY_COLOUR 1
#define KEY_BATTERY_SHOW 2
#define KEY_TIME_COLOUR 3
#define KEY_LANG 4
#define KEY_DATE_SHOW 5
#define KEY_MINUTE_STROKE_COLOUR 6
#define KEY_HOUR_STROKE_COLOUR 7
#define KEY_HOUR_FILL_COLOUR 8
#define KEY_HOUR_TEXT_COLOUR 9
#define KEY_DATE_TEXT_COLOUR 10
#define KEY_DATE_FILL_COLOUR 11
#define KEY_DATE_STROKE_COLOUR 12



// Languages
#define LANG_DUTCH 0
#define LANG_ENGLISH 1
#define LANG_FRENCH 2
#define LANG_GERMAN 3
#define LANG_SPANISH 4
#define LANG_PORTUGUESE 5
#define LANG_SWEDISH 6
#define LANG_MAX 7

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

static uint8_t all_colours[] = {
	GColorArmyGreenARGB8,
	GColorBabyBlueEyesARGB8,
	GColorBlackARGB8,
	GColorBlueARGB8,
	GColorBlueMoonARGB8,
	GColorBrassARGB8,				//5
	GColorBrightGreenARGB8,
	GColorBrilliantRoseARGB8,
	GColorBulgarianRoseARGB8,
	GColorCadetBlueARGB8,
	GColorCelesteARGB8,				//10
	GColorChromeYellowARGB8,
	GColorCobaltBlueARGB8,
	GColorCyanARGB8,
	GColorDarkCandyAppleRedARGB8,
	GColorDarkGrayARGB8,			//15
	GColorDarkGreenARGB8,
	GColorDukeBlueARGB8,
	GColorElectricBlueARGB8,
	GColorElectricUltramarineARGB8,
	GColorFashionMagentaARGB8,		//20
	GColorFollyARGB8,
	GColorGreenARGB8,
	GColorIcterineARGB8,
	GColorImperialPurpleARGB8,
	GColorInchwormARGB8,			//25
	GColorIndigoARGB8,
	GColorIslamicGreenARGB8,
	GColorJaegerGreenARGB8,
	GColorJazzberryJamARGB8,
	GColorKellyGreenARGB8,			//30
	GColorLavenderIndigoARGB8,
	GColorLibertyARGB8,
	GColorLightGrayARGB8,
	GColorLimerickARGB8,
	GColorMagentaARGB8,				//35
	GColorMalachiteARGB8,
	GColorMayGreenARGB8,
	GColorMediumAquamarineARGB8,
	GColorMediumSpringGreenARGB8,
	GColorMelonARGB8,				//40
	GColorMidnightGreenARGB8,
	GColorMintGreenARGB8,
	GColorOrangeARGB8,
	GColorOxfordBlueARGB8,
	GColorPastelYellowARGB8,		//45
	GColorPictonBlueARGB8,
	GColorPurpleARGB8,
	GColorPurpureusARGB8,
	GColorRajahARGB8,
	GColorRedARGB8, 				//50
	GColorRichBrilliantLavenderARGB8,
	GColorRoseValeARGB8,
	GColorScreaminGreenARGB8,
	GColorShockingPinkARGB8,
	GColorSpringBudARGB8,			//55
	GColorSunsetOrangeARGB8,
	GColorTiffanyBlueARGB8,
	GColorVeryLightBlueARGB8,
	GColorVividCeruleanARGB8,
	GColorVividVioletARGB8,			//60
	GColorWhiteARGB8,
	GColorWindsorTanARGB8,
	GColorYellowARGB8
};

// first bg colour index
static int s_colour_a;

// second bg colour index
static int s_colour_b;

// store index for the colour name from the face_colours array
static uint8_t main_colour;
static uint8_t minute_colour;
static int b_angle = 45; // for battery arc
static int battery_level;
static bool battery_charging;
static BatteryChargeState charge_state;

// user choices - set defaults here
static int user_lang = 1; 							//default to english ??? does strftime do localization??
static bool user_show_battery = true;				//default to ON
static bool user_show_date = true;					//default to ON
static int user_minute_stroke_colour = 2; 			//default black (in "all-colours")
//static int user_hour_minute_outline_colour = 61; 	//default white
static int user_battery_colour = 50; 				//default red
static int user_hour_fill = 61; 					//default white
static int user_hour_stroke_colour = 2; 			//default black
static int user_hour_text_colour = 2; 				//default black
static int user_date_fill = 61; 					//default white
static int user_date_text_colour = 2; 				//default black
static int user_date_stroke_colour = 2;				//default black
static int user_outline_colour = 61;				//default white


static int user_time_colour[] = {44,59,24,35,15,33,43,63,16,6,3,13,0,34,19,1,62,11,50,40,14,54,17,9}; //**** indices for "all_colours"
/*
static int user_time_colour_00 = 44;					//colour for 12am
static int user_time_colour_01 = 59;					//colour for 1am
static int user_time_colour_02 = 24;					//colour for 2am
static int user_time_colour_03 = 35;					//colour for 3am
static int user_time_colour_04 = 15;					//colour for 4am
static int user_time_colour_05 = 33;					//colour for 5am
static int user_time_colour_06 = 43;					//colour for 6am
static int user_time_colour_07 = 63;					//colour for 7am
static int user_time_colour_08 = 16;					//colour for 8am
static int user_time_colour_09 = 6;						//colour for 9am
static int user_time_colour_10 = 3;						//colour for 10am
static int user_time_colour_11 = 13;					//colour for 11am
static int user_time_colour_12 = 0;						//colour for 12pm
static int user_time_colour_13 = 34;					//colour for 1pm
static int user_time_colour_14 = 19;					//colour for 2pm
static int user_time_colour_15 = 1;						//colour for 3pm
static int user_time_colour_16 = 62;					//colour for 4pm
static int user_time_colour_17 = 11;					//colour for 5pm
static int user_time_colour_18 = 50;					//colour for 6pm
static int user_time_colour_19 = 40;					//colour for 7pm
static int user_time_colour_20 = 14;					//colour for 8pm
static int user_time_colour_21 = 54;					//colour for 9pm
static int user_time_colour_22 = 17;					//colour for 10pm
static int user_time_colour_23 = 9;						//colour for 11pm
*/

// common coordinates
static GPoint screen_centre = { 72, 84 };
static GPoint screen_top_centre = { 72, -1 }; // find it works better to be slightly "outside" the screen for the corners
static GPoint screen_top_right = { 145, -1 };
static GPoint screen_bottom_right = { 145, 169 };
static GPoint screen_bottom_left = { -1, 169 };
static GPoint screen_top_left = { -1, -1 };



const char weekDay[LANG_MAX][7][6] = {
	{ "zon", "maa", "din", "woe", "don", "vri", "zat" },	// Dutch
	{ "sun", "mon", "tue", "wed", "thu", "fri", "sat" },	// English
	{ "dim", "lun", "mar", "mer", "jeu", "ven", "sam" },	// French
	{ "son", "mon", "die", "mit", "don", "fre", "sam" },	// German
	{ "dom", "lun", "mar", "mie", "jue", "vie", "sab" },	// Spanish
	{ "dom", "seg", "ter", "qua", "qui", "sex", "sab" },	// Portuguese
	{ "sön", "mån", "Tis", "ons", "tor", "fre", "lör" }		// Swedish
};






void batteryLineArc(GContext *ctx, float start_angle, float end_angle, GPoint centre, int radius, int thickness){
	
	float minus_bit = radius - thickness/2 + 0.5;// + (user_battery_colour <= 1 ? 0.5 : 0); //add 0.5 for the int casting, later !!! dark colours like 1.0, light better with 0.5
	float add_bit = (radius + thickness/2 + 0.5); 
	
	graphics_context_set_stroke_width(ctx, 1);
	
	if (start_angle == 0) {
		start_angle = 2.5;	// for some reason, 0 seems to start too far to the left....at least for my purposes
	}
	
	for (float i = start_angle; i <= end_angle; i+=0.5) {
		
		GPoint inside_point = (GPoint) {
			.x = (int16_t)(sin_lookup(i * TRIG_MAX_ANGLE / 360) * minus_bit / TRIG_MAX_RATIO) + centre.x,
			.y = (int16_t)(-cos_lookup(i * TRIG_MAX_ANGLE / 360) * minus_bit / TRIG_MAX_RATIO) + centre.y,
		};

		GPoint outside_point = (GPoint) {
			.x = (int16_t)(sin_lookup(i * TRIG_MAX_ANGLE / 360) * add_bit / TRIG_MAX_RATIO) + centre.x,
			.y = (int16_t)(-cos_lookup(i * TRIG_MAX_ANGLE / 360) * add_bit / TRIG_MAX_RATIO) + centre.y,
		};
		
		graphics_draw_line(ctx, inside_point, outside_point);
	}
}


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
		
		//**** new, time-based method
		s_colour_a = tick_time->tm_hour;
		
		s_colour_b = s_colour_a + 1;
		
		if (s_colour_b > 23) {
			s_colour_b = 0;
		}
		
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d", s_colour_a, s_colour_b);
	}
	
	if (user_show_battery) {
		charge_state = battery_state_service_peek();
		battery_level = charge_state.charge_percent;
		battery_charging = charge_state.is_charging;
	}
	
  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}



static void update_proc(Layer *layer, GContext *ctx) {
	// Don't use current time while animating
  	Time mode_time = (s_animating) ? s_anim_time : s_last_time;
	s_radius = s_animating ? s_radius : FINAL_RADIUS;
	
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);
	
	graphics_context_set_antialiased(ctx, ANTIALIASING);

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
			// remove the leading "0"
			snprintf(buffer, 2, "%d", mode_time.hours);
		}
		else if (!clock_is_24h_style() && mode_time.hours == 0) {
			// stupid time, of course we want "12"
			snprintf(buffer, 4, "%s", "12");
		}

		// Display this time on the TextLayer
		text_layer_set_text_color(s_hour_digit, (GColor)all_colours[user_hour_text_colour]);
		text_layer_set_text(s_hour_digit, buffer);
	}
	
	// Color background for first colour
	main_colour = all_colours[user_time_colour[s_colour_a]]; //all_colours[s_colour_a];
	graphics_context_set_fill_color(ctx, (GColor)main_colour);
	graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);
  
	
  	
	
	
	// Plot minute line
	GPoint minute_hand = (GPoint) {
		.x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)MINUTE_RADIUS / TRIG_MAX_RATIO) + s_center.x,
		.y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)MINUTE_RADIUS / TRIG_MAX_RATIO) + s_center.y,
	};

	
	// Fill in minute color (aka s_colour_b) -- basic is a triangle in the first eighth of the clock (< 7 minutes)
	GPathInfo s_min_points =  {
		.num_points = 3,
		.points = (GPoint []) {
			screen_top_centre, 
			screen_centre, 
			{minute_hand.x, minute_hand.y}
		}
	};
	
	if (mode_time.minutes > 7 && mode_time.minutes < 23) {
		// add in the top-right corner
		s_min_points.num_points = 4;
		s_min_points.points = (GPoint []) {
			screen_top_centre, 
			screen_centre, 
			{minute_hand.x, minute_hand.y}, 
			screen_top_right
		};
	}
	else if (mode_time.minutes > 22 && mode_time.minutes < 38) {
		// add in bottom-right and top-right
		s_min_points.num_points = 5,
		s_min_points.points = (GPoint []) {
			screen_top_centre, 
			screen_centre, 
			{minute_hand.x, minute_hand.y},  
			screen_bottom_right, 
			screen_top_right
		};
	}
	else if (mode_time.minutes > 37 && mode_time.minutes < 52) {
		// bottom-left, bottom-right & top-right
		s_min_points.num_points = 6,
		s_min_points.points = (GPoint []) {
			screen_top_centre, 
			screen_centre, 
			{minute_hand.x, minute_hand.y}, 
			screen_bottom_left, 
			screen_bottom_right, 
			screen_top_right
		};
	}
	else if (mode_time.minutes > 51 && mode_time.minutes <= 59) {
		//*/ going negative seems to help along the left edge - draw to all four corners
		s_min_points.num_points = 7,
		s_min_points.points = (GPoint []) {
			screen_top_centre, 
			screen_centre, 
			{minute_hand.x, minute_hand.y}, 
			screen_top_left, 
			screen_bottom_left, 
			screen_bottom_right, 
			screen_top_right
		};
	}
	
	// set up
	s_min_path = gpath_create(&s_min_points);
	
	// fill in minute area
	minute_colour = all_colours[user_time_colour[s_colour_b]]; //all_colours[s_colour_b];
	graphics_context_set_fill_color(ctx, (GColor)minute_colour);
  	gpath_draw_filled(ctx, s_min_path);
	
	// Draw hour circle outline
	graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_outline_colour]);//white
	graphics_context_set_stroke_width(ctx, minute_stroke + border_stroke);
  	graphics_draw_circle(ctx, s_center, s_radius);
	
	// draw minute line
	if (!s_animating) {
		//outline first, wider
		graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_outline_colour]); //white
		graphics_context_set_stroke_width(ctx, minute_stroke + border_stroke);
		graphics_draw_line(ctx, s_center, minute_hand);
		
		graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_minute_stroke_colour]);
		graphics_context_set_stroke_width(ctx, minute_stroke);
		graphics_draw_line(ctx, s_center, minute_hand);
	}
	
	
	
	
	// fill hour 
	graphics_context_set_fill_color(ctx, (GColor)all_colours[user_hour_fill]);
	graphics_fill_circle(ctx, s_center, s_radius);
	
  	
	// draw hour stroke 
	graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_hour_stroke_colour]);
	graphics_context_set_stroke_width(ctx, minute_stroke);
  	graphics_draw_circle(ctx, s_center, s_radius);

	if (user_show_date) {
		// date box fill
		graphics_context_set_fill_color(ctx, (GColor)all_colours[user_date_fill]);
		graphics_fill_rect(ctx, GRect(40, 131, 64, 24), 10, GCornersAll);

		// date box stroke
		graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_date_stroke_colour]);
		graphics_context_set_stroke_width(ctx, 1);
		graphics_draw_round_rect(ctx, GRect(40, 131, 64, 24), 10);

		// date box content -- weekDay[user_lang][tick_time->tm_wday]
		strftime(s_day_buffer, sizeof("ddd dd"), "%a %d", tick_time);
		text_layer_set_text_color(s_day_date, (GColor)all_colours[user_date_text_colour]);
		text_layer_set_text(s_day_date, s_day_buffer);
	}
	else {
		// text will stick around otherwise
		text_layer_set_text(s_day_date, "");
	}
	
	// if showing battery level, draw arc
	if (user_show_battery) {// && !battery_charging) {
		graphics_context_set_stroke_color(ctx, (GColor)all_colours[user_battery_colour]);
		
		b_angle = (int)((360 * ((100 - battery_level)) / 100) + 0.5); // +0.5 is a float to int trick
		batteryLineArc(ctx, 0, b_angle, s_center, s_radius, minute_stroke);
	}
	
	s_startup = false;
	
} // end update_proc




static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect window_bounds = layer_get_bounds(window_layer);

	s_center = grect_center_point(&window_bounds);

	s_canvas_layer = layer_create(window_bounds);
	layer_set_update_proc(s_canvas_layer, update_proc);
	layer_add_child(window_layer, s_canvas_layer);
	
	//battery_state_service_subscribe(battery_handler);
	
	// Create time TextLayer
	s_hour_digit = text_layer_create(GRect(41, 58, 62, 50));
	text_layer_set_background_color(s_hour_digit, GColorClear);
	
	
	// create day/date layer
	s_day_date = text_layer_create(GRect(40, 130, 64, 24));
	text_layer_set_background_color(s_day_date, GColorClear);
	
	
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
	battery_state_service_unsubscribe();
	tick_timer_service_unsubscribe();
	
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


char *
strtok(s, delim)
	register char *s;
	register const char *delim;
{
	register char *spanp;
	register int c, sc;
	char *tok;
	static char *last;


	if (s == NULL && (s = last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


int my_getnbr(char *str) {
	int           result;
	int           puiss;

	result = 0;
	puiss = 1;
	while (('-' == (*str)) || ((*str) == '+'))
	{
		if (*str == '-')
			puiss = puiss * -1;
		str++;
	}
	while ((*str >= '0') && (*str <= '9'))
	{
		result = (result * 10) + ((*str) - '0');
		str++;
	}
	return (result * puiss);
}


static void in_recv_handler(DictionaryIterator *iterator, void *context) {
	//Get Tuples
	Tuple *t = dict_read_first(iterator);
	int i = 0;
	char *p;
	
	while (t != NULL) {
		switch(t->key) {
			case KEY_BATTERY_SHOW:
				if (strcmp(t->value->cstring, "1") == 0) {
					user_show_battery = true;
				}
				else {
					user_show_battery = false;
				}
			break;
			
			case KEY_BATTERY_COLOUR:
				user_battery_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "batt color: %d", user_battery_colour);
			break;
			
			case KEY_DATE_SHOW:
				if (strcmp(t->value->cstring, "1") == 0) {
					user_show_date = true;
				}
				else {
					user_show_date = false;
				}
			break;
			
			case KEY_DATE_FILL_COLOUR:
				user_date_fill = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "date fill: %d", user_date_fill);
			break;
			
			case KEY_DATE_TEXT_COLOUR:
				user_date_text_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "date text: %d", user_date_text_colour);
			break;
			
			case KEY_DATE_STROKE_COLOUR:
				user_date_stroke_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "date stroke: %d", user_date_stroke_colour);
			break;
			
			case KEY_HOUR_FILL_COLOUR:
				user_hour_fill = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "hour fill: %d", user_hour_fill);
			break;
			
			case KEY_HOUR_TEXT_COLOUR:
				user_hour_text_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "hour text: %d", user_hour_text_colour);
			break;
			
			case KEY_HOUR_STROKE_COLOUR:
				user_hour_stroke_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "hour stroke: %d", user_hour_stroke_colour);
			break;
			
			case KEY_MINUTE_STROKE_COLOUR:
				user_minute_stroke_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "min stroke: %d", user_minute_stroke_colour);
			break;
			
			case KEY_OUTLINE_COLOUR:
				user_outline_colour = (int)*t->value->cstring;
				APP_LOG(APP_LOG_LEVEL_DEBUG, "outline color: %d", user_outline_colour);
			break;
			
			case KEY_LANG:
				user_lang = (int)*t->value->cstring;
			break;
			
			case KEY_TIME_COLOUR:
				//char *array[25];
				i = 0;
				char* new_str = malloc(strlen(t->value->cstring));
  				strcpy(new_str, t->value->cstring);
				//APP_LOG(APP_LOG_LEVEL_DEBUG, "time color cstr: %s", new_str);
				
				p = strtok(new_str, ",");
				
				while (p != NULL) {
					user_time_colour[i++] = my_getnbr(p);
					p = strtok(NULL, ",");
					APP_LOG(APP_LOG_LEVEL_DEBUG, "time color %d: %d", (i-1), user_time_colour[i-1]);
				}
				
				//free(new_str);
			break;
		} // end switch

		t = dict_read_next(iterator);
	} // end while
	
	layer_mark_dirty(s_canvas_layer);
} // end function



static void init() {
  //srand(time(NULL));

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
	
	//**** read user values
	user_lang = persist_exists(KEY_LANG) ? persist_read_int(KEY_LANG) : user_lang; 							
	user_show_battery = persist_exists(KEY_BATTERY_SHOW) ? persist_read_bool(KEY_BATTERY_SHOW) : user_show_battery;
	user_show_date = persist_exists(KEY_DATE_SHOW) ? persist_read_bool(KEY_DATE_SHOW) : user_show_date;
	user_minute_stroke_colour = persist_exists(KEY_MINUTE_STROKE_COLOUR) ? persist_read_int(KEY_MINUTE_STROKE_COLOUR) : user_minute_stroke_colour; 
	user_hour_stroke_colour = persist_exists(KEY_HOUR_STROKE_COLOUR) ? persist_read_int(KEY_HOUR_STROKE_COLOUR) : user_hour_stroke_colour;
	user_battery_colour = persist_exists(KEY_BATTERY_COLOUR) ? persist_read_int(KEY_BATTERY_COLOUR) : user_battery_colour;
	user_hour_fill = persist_exists(KEY_HOUR_FILL_COLOUR) ? persist_read_int(KEY_HOUR_FILL_COLOUR) : user_hour_fill;
	user_hour_text_colour = persist_exists(KEY_HOUR_TEXT_COLOUR) ? persist_read_int(KEY_HOUR_TEXT_COLOUR) : user_hour_text_colour;
	user_date_fill = persist_exists(KEY_DATE_FILL_COLOUR) ? persist_read_int(KEY_DATE_FILL_COLOUR) : user_date_fill;
	user_date_text_colour = persist_exists(KEY_DATE_TEXT_COLOUR) ? persist_read_int(KEY_DATE_TEXT_COLOUR) : user_date_text_colour;
	user_date_stroke_colour = persist_exists(KEY_DATE_STROKE_COLOUR) ? persist_read_int(KEY_DATE_STROKE_COLOUR) : user_date_stroke_colour;
	user_outline_colour = persist_exists(KEY_OUTLINE_COLOUR) ? persist_read_int(KEY_OUTLINE_COLOUR) : user_outline_colour;
	persist_read_data(KEY_TIME_COLOUR, &user_time_colour, sizeof(user_time_colour));
	/*char time_colours[24 * 2 + 23];
	for (int i = 0; i < 24; i++) {
		
	}*/
	
	app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
} // end init



static void deinit() {
	window_destroy(s_main_window);
	
	//**** save user values 
	persist_write_int(KEY_LANG, user_lang);
	persist_write_bool(KEY_BATTERY_SHOW, user_show_battery);
	persist_write_bool(KEY_DATE_SHOW, user_show_date);
	persist_write_int(KEY_MINUTE_STROKE_COLOUR, user_minute_stroke_colour);
	persist_write_int(KEY_HOUR_STROKE_COLOUR, user_hour_stroke_colour);
	persist_write_int(KEY_BATTERY_COLOUR, user_battery_colour);
	persist_write_int(KEY_HOUR_FILL_COLOUR, user_hour_fill);
	persist_write_int(KEY_HOUR_TEXT_COLOUR, user_hour_text_colour);
	persist_write_int(KEY_DATE_FILL_COLOUR, user_date_fill);
	persist_write_int(KEY_DATE_TEXT_COLOUR, user_date_text_colour);
	persist_write_int(KEY_DATE_STROKE_COLOUR, user_date_stroke_colour);
	persist_write_int(KEY_OUTLINE_COLOUR, user_outline_colour);
	persist_write_data(KEY_TIME_COLOUR, &user_time_colour, sizeof(user_time_colour));
	
} // end deinit



int main() {
  init();
  app_event_loop();
  deinit();
}
