#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_LOCALE 2
#define KEY_CONDITIONS_ID 3
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_date_layer;
static TextLayer *s_conditions_layer;


static GFont s_time_font;
static GFont s_weather_font;
static GFont s_date_font;
static GFont s_conditions_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char timebuffer[10];
  static char datebuffer[30];
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 24h format
    strftime(timebuffer, sizeof(timebuffer), "%H:%M", tick_time);
  } else {
    //Use 12h format
    strftime(timebuffer, sizeof(timebuffer), "%I:%M", tick_time);
  }
  
  strftime(datebuffer, sizeof(datebuffer), "%A, %C. %B %Y", tick_time);

  // Display time on TimeTextLayer
  text_layer_set_text(s_time_layer, timebuffer);
  
  // Display date on DateTextLayer
  text_layer_set_text(s_date_layer, datebuffer);
}

static void main_window_load(Window *window) {
  
  
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 1, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  
  //Create GFont
  s_time_font = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD); 

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create date Layer
  s_date_layer = text_layer_create(GRect(0, 130, 144, 45));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_text(s_date_layer, "01.01.1900");
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  text_layer_set_font(s_date_layer, s_date_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Create temperature Layer
  s_weather_layer = text_layer_create(GRect(0, 148, 144, 45));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  s_weather_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  //Create Conditions Layer
  s_conditions_layer = text_layer_create(GRect(0, 50, 135, 50));
  text_layer_set_background_color(s_conditions_layer, GColorClear);
  text_layer_set_text_color(s_conditions_layer, GColorWhite);
  text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentRight);
  text_layer_set_text(s_conditions_layer, "G");
  s_conditions_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_METEO_50));
  text_layer_set_font(s_conditions_layer, s_conditions_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_conditions_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_conditions_font);
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy DateLayer
  text_layer_destroy(s_date_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  static char locale_buffer[8];
  static char conditions_id_buffer[3];

  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE: 
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "temp: %d", (int)t->value->int32);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_TEMPERATURE(0): %d", (int)t->key);
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", (int)t->value->int32);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "TEMPERATURE_BUFFER: %s", temperature_buffer);
      break;
    case KEY_CONDITIONS: 
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "cond: %s", t->value->cstring);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_CONDITIONS(1): %d", (int)t->key);
        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "CONDITIONS_BUFFER: %s", conditions_buffer);
      break;
    case KEY_LOCALE: 
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "locale: %s", t->value->cstring);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_LOCALE(2): %d", (int)t->key);
        locale_buffer = t->value->cstring;
        setlocale(LC_ALL, locale_buffer);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "LOCALE_BUFFER: %s", locale_buffer);
        break;
    case KEY_CONDITIONS_ID:
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "cond id: %s", t->value->cstring);
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "KEY_CONDITIONS_ID(3): %d", (int)t->key);
        snprintf(conditions_id_buffer, sizeof(conditions_id_buffer), "%s", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "CONDITIONS_ID_BUFFER: %s", conditions_id_buffer);
        break;
    
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }
    // Look for next item
    t = dict_read_next(iterator);
  
      
      
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

