#include <pebble.h>

Window* my_window = NULL;

//Time Display
char time_string[] = "00:00";  // Make this longer to show AM/PM
TextLayer* time_outline_layer = NULL;
TextLayer* time_text_layer = NULL;

//Date Display
time_t current_time;
char date_string[64];
TextLayer* date_text_layer = NULL;

//Image Display
BitmapLayer* bitmap_layer = NULL;
GBitmap* gbitmap_ptr = NULL;

//Transition animations
Layer* animation_layer = NULL;
PropertyAnimation* prop_animation_slide_left = NULL;
PropertyAnimation* prop_animation_slide_up = NULL;

//Image indexes and count
int max_images = 0; //automatically detected
int image_index = RESOURCE_ID_IMAGE_OFFSET + 1; //Images start after IMAGE_OFFSET

static void do_animation(void){
  static int animation_selection = 0; //just 0 and 1 for now
  const int animation_count = 2;

  animation_selection = (animation_selection + 1) % animation_count;

  if (animation_selection == 0) {
    animation_schedule((Animation*)prop_animation_slide_left);
  } else {
    animation_schedule((Animation*)prop_animation_slide_up);
  }
}

static void load_image_resource(uint32_t resource_id){
  if (gbitmap_ptr) {
    gbitmap_destroy(gbitmap_ptr);
    gbitmap_ptr = NULL;
  }
  gbitmap_ptr = gbitmap_create_with_resource(resource_id);
  image_index = resource_id;
  //layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
}

static void increment_image(void){
  image_index = ( image_index >= max_images + RESOURCE_ID_IMAGE_OFFSET) ? 
    (RESOURCE_ID_IMAGE_OFFSET + 1) : (image_index + 1);
  load_image_resource(image_index);
}

void tap_handler(AccelAxisType axis, int32_t direction){
  increment_image();
  do_animation();
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  if (units_changed & DAY_UNIT) {
    current_time = time(NULL);
    strftime(date_string, sizeof(date_string), "%a, %b %d", localtime(&current_time));
    //layer_mark_dirty(text_layer_get_layer(date_text_layer));
  }

  clock_copy_time_string(time_string,sizeof(time_string));
  layer_mark_dirty(text_layer_get_layer(time_outline_layer));
  layer_mark_dirty(text_layer_get_layer(time_text_layer));
}

static void init_animation(GRect bounds) {
  GRect right_image_bounds = (GRect){
    .origin = (GPoint){.x=144,.y=0},
    .size = bounds.size};

  GRect bottom_image_bounds = (GRect){
    .origin = (GPoint){.x=0,.y=168},
    .size = bounds.size};

  // Setup slide left animation
  prop_animation_slide_left = property_animation_create_layer_frame( 
    animation_layer, &right_image_bounds, NULL);

  animation_set_duration((Animation*)prop_animation_slide_left, 1500);
  animation_set_curve((Animation*)prop_animation_slide_left, 
    AnimationCurveEaseInOut);

  // Setup slide up animation
  prop_animation_slide_up = property_animation_create_layer_frame( 
    animation_layer, &bottom_image_bounds, NULL);

  animation_set_duration((Animation*)prop_animation_slide_up, 1500);
  animation_set_curve((Animation*)prop_animation_slide_up, 
    AnimationCurveEaseInOut);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Add layers from back to front (background first)

  //Initialise animations
  animation_layer = layer_create(bounds);
  layer_add_child(window_layer, animation_layer);
  init_animation(bounds);
  
  //Create bitmap layer for background image
  bitmap_layer = bitmap_layer_create(bounds);
  //Add bitmap_layer to animation layer
  layer_add_child(animation_layer, bitmap_layer_get_layer(bitmap_layer));

  //Load initial bitmap image
  load_image_resource(image_index);
  bitmap_layer_set_bitmap(bitmap_layer, gbitmap_ptr);
  
  //Setup the time outline display
  time_outline_layer = text_layer_create(GRect(0, 130, 144, 30));
  text_layer_set_text(time_outline_layer, time_string);
	text_layer_set_font(time_outline_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOXY_OUTLINE_30)));
  text_layer_set_text_alignment(time_outline_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_outline_layer, GColorClear);
  text_layer_set_text_color(time_outline_layer, GColorBlack);
  clock_copy_time_string(time_string,sizeof(time_string));
  
  //Add clock text second
  layer_add_child(window_layer, text_layer_get_layer(time_outline_layer));

  //Setup the time display
  time_text_layer = text_layer_create(GRect(0, 130, 144, 30));
  text_layer_set_text(time_text_layer, time_string);
	text_layer_set_font(time_text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BOXY_TEXT_30)));
  text_layer_set_text_alignment(time_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_text_layer, GColorClear);
  text_layer_set_text_color(time_text_layer, GColorWhite);
  clock_copy_time_string(time_string,sizeof(time_string));
  
  //Add clock text second
  layer_add_child(window_layer, text_layer_get_layer(time_text_layer));

  //Setup the date display
  current_time = time(NULL);
  strftime(date_string, sizeof(date_string), "%a, %b %d", localtime(&current_time));
  date_text_layer = text_layer_create(GRect(4, 148, 88, 18));
  text_layer_set_text(date_text_layer, date_string);
	text_layer_set_font(date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
 
  layer_add_child(window_layer, text_layer_get_layer(date_text_layer));
  layer_set_hidden(text_layer_get_layer(date_text_layer), true);
  
  //Setup tick time handler
  tick_timer_service_subscribe((MINUTE_UNIT), tick_handler);
  
  //Setup tap service
  accel_tap_service_subscribe(tap_handler);
}

static void window_unload(Window *window) {
}



void handle_init(void) {
  //Discover how many images from base index (offset+1)
  while (resource_get_handle(RESOURCE_ID_IMAGE_OFFSET + 1 + max_images)) {
    max_images++;
  }
  
  my_window = window_create();
  window_set_fullscreen(my_window, true);
  //Allows compositing new image animation over last image displayed
  window_set_background_color(my_window, GColorClear);  
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  window_stack_push(my_window, false/*animated*/);
}

void handle_deinit(void) {
    bitmap_layer_destroy(bitmap_layer);
	  text_layer_destroy(time_outline_layer);
	  text_layer_destroy(time_text_layer);
	  window_destroy(my_window);
}


int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
