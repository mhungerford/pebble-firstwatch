#include <pebble.h>

Window *my_window = NULL;

char time_string[] = "00:00";
TextLayer *time_outline_layer = NULL;
TextLayer *time_text_layer = NULL;

time_t current_time;
char date_string[64];
TextLayer *date_text_layer = NULL;

BitmapLayer *bitmap_layer = NULL;
GBitmap *gbitmap_ptr = NULL;

int max_images = 0; //automatically detected
int image_index = 3; //Images start at resource = 3


void load_image_resource(uint32_t resource_id){
  if (gbitmap_ptr) {
    gbitmap_destroy(gbitmap_ptr);
    gbitmap_ptr = NULL;
  }
  gbitmap_ptr = gbitmap_create_with_resource(resource_id);
  image_index = resource_id;
  layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
}

void tap_handler(AccelAxisType axis, int32_t direction){
  image_index = ( image_index >= max_images + 2) ? 3 : (image_index + 1);
  load_image_resource(image_index);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  if (units_changed & HOUR_UNIT) {
    // Before 8am
    /*
    if (tick_time->tm_hour < 8 ) {
      load_image_resource(RESOURCE_ID_IMAGE_EARTH);
    } else if (tick_time->tm_hour < 20 ) {  //Before 8pm
      load_image_resource(RESOURCE_ID_IMAGE_GIANT);
    } else { //Before midnight
      load_image_resource(RESOURCE_ID_IMAGE_MONSTERS);
    }

    //Special case tuesday @ 8 -- YOGA?
    if (tick_time->tm_wday == 2 && tick_time->tm_hour == 8) {
      load_image_resource(RESOURCE_ID_IMAGE_MINIWOLF);
    }
    
    //Special case friday @ 5pm -- BEER!
    if (tick_time->tm_wday == 5 && tick_time->tm_hour == 17) {
      load_image_resource(RESOURCE_ID_IMAGE_MINIWOLF);
    }
    */
    
    layer_mark_dirty(bitmap_layer_get_layer(bitmap_layer));
  }
  
  if (units_changed & DAY_UNIT) {
    current_time = time(NULL);
    strftime(date_string, sizeof(date_string), "%a, %b %d", localtime(&current_time));
    //layer_mark_dirty(text_layer_get_layer(date_text_layer));
  }

  clock_copy_time_string(time_string,sizeof(time_string));
  layer_mark_dirty(text_layer_get_layer(time_outline_layer));
  layer_mark_dirty(text_layer_get_layer(time_text_layer));
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  bitmap_layer = bitmap_layer_create(bounds);
  load_image_resource(image_index);
  bitmap_layer_set_bitmap(bitmap_layer, gbitmap_ptr);

  //Add background first
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  
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
  
  //Setup hour and minute handlers
  tick_timer_service_subscribe((MINUTE_UNIT | HOUR_UNIT | DAY_UNIT), tick_handler);
  
  //Setup tap service
  accel_tap_service_subscribe(tap_handler);
}

static void window_unload(Window *window) {
}



void handle_init(void) {
  //Discover how many images from base index
  while (resource_get_handle(max_images + 3)) {
    max_images++;
  }
  
  my_window = window_create();
  window_set_fullscreen(my_window, true);
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
