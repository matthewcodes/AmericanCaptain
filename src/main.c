#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static BitmapLayer *s_shield_top;
static GBitmap *s_shield_top_bitmap;
static BitmapLayer *s_shield_bottom;
static GBitmap *s_shield_bottom_bitmap;
static PropertyAnimation *top_property_animation;
static PropertyAnimation *bottom_property_animation;

static void bottom_animation_stopped(Animation *animation, bool finished, void *data) {
  
  if(finished) {
    property_animation_destroy(top_property_animation);
    property_animation_destroy(bottom_property_animation);
  }
}

static void open_shield() {

  Layer* shield_top = bitmap_layer_get_layer(s_shield_top);
  GRect top_from_frame = layer_get_frame(shield_top);
  GRect top_to_frame = GRect(0, -10, 144, 66);
  
  Layer* shield_bottom = bitmap_layer_get_layer(s_shield_bottom);
  GRect bottom_from_frame = layer_get_frame(shield_bottom);
  GRect bottom_to_frame = GRect(0, 112, 144, 66);

  top_property_animation = property_animation_create_layer_frame(shield_top, &top_from_frame, &top_to_frame);
  bottom_property_animation = property_animation_create_layer_frame(shield_bottom, &bottom_from_frame, &bottom_to_frame);
  
  animation_set_delay((Animation*) top_property_animation, 2000);
  animation_set_delay((Animation*) bottom_property_animation, 2000);
  
  animation_set_handlers((Animation*) bottom_property_animation, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) bottom_animation_stopped
  }, NULL);
  
  animation_schedule((Animation*) top_property_animation);
  animation_schedule((Animation*) bottom_property_animation);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void top_animation_stopped(Animation *animation, bool finished, void *data) {
  
  if(finished) {
    update_time(); 
  
    property_animation_destroy(top_property_animation);
    property_animation_destroy(bottom_property_animation);
  
    open_shield();
  }
}

static void close_shield() {

  Layer* shield_top = bitmap_layer_get_layer(s_shield_top);
  GRect top_from_frame = layer_get_frame(shield_top);
  GRect top_to_frame = GRect(0, 18, 144, 66);
  
  Layer* shield_bottom = bitmap_layer_get_layer(s_shield_bottom);
  GRect bottom_from_frame = layer_get_frame(shield_bottom);
  GRect bottom_to_frame = GRect(0, 84, 144, 66);

  top_property_animation = property_animation_create_layer_frame(shield_top, &top_from_frame, &top_to_frame);
  bottom_property_animation = property_animation_create_layer_frame(shield_bottom, &bottom_from_frame, &bottom_to_frame);
  
  animation_set_handlers((Animation*) top_property_animation, (AnimationHandlers) {
    .stopped = (AnimationStoppedHandler) top_animation_stopped
  }, NULL);
  
  animation_schedule((Animation*) top_property_animation);
  animation_schedule((Animation*) bottom_property_animation);
  
}
  
static void main_window_load(Window *window) {
  
  window_set_background_color(window, GColorBlack);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 60));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);
  //text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_AMERICAN_CAPTAIN_46));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create GBitmap, then set to created BitmapLayer
  s_shield_top_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHIELD_TOP);
  s_shield_top = bitmap_layer_create(GRect(0, -10, 144, 66));
  bitmap_layer_set_bitmap(s_shield_top, s_shield_top_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_shield_top));
  
  s_shield_bottom_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SHIELD_BOTTOM);
  s_shield_bottom = bitmap_layer_create(GRect(0, 112, 144, 66));
  bitmap_layer_set_bitmap(s_shield_bottom, s_shield_bottom_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_shield_bottom));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  gbitmap_destroy(s_shield_top_bitmap);
  bitmap_layer_destroy(s_shield_top);
  gbitmap_destroy(s_shield_bottom_bitmap);
  bitmap_layer_destroy(s_shield_bottom);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  close_shield();
  //update_time();
  //open_shield();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
}

static void deinit() {
  animation_unschedule_all();
  
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}