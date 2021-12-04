//TFT LVGL Functions 
/********************************************************************************/
#define LV_LVGL_H_INCLUDE_SIMPLE
//#include "TestImage.c"

//TFT LVGL Objects
static lv_obj_t * scr;                // Main Screen object

//Screen objects
static lv_obj_t * scr_home;

static lv_obj_t *label;
static lv_obj_t * kb;
static lv_obj_t * tb_message;
static lv_obj_t * bg_nav_panel;
static lv_obj_t * bg_main_panel;
static lv_obj_t * btn_write_message;
static lv_obj_t * ta_write_message;

static lv_obj_t * my_image_name;

static lv_style_t nav_button_style;
static lv_style_t nav_button_text_style;
static lv_style_t time_date_text_style;

// const lv_img_dsc_t TestImage = {
//  {
//    LV_IMG_CF_TRUE_COLOR,  //Header CF
//    0,                           // header.alwayszero
//    2,                           // Unknown
//    285,                         // Width
//    190,                         // height 
//  },
//      86700 * LV_COLOR_SIZE / 8,   // data size
//     TestImage_map,        // data

// };


void guiTask(void *pvParameters) {
    
    InitTFTDisplay();

    uint16_t calData[5] = { 295, 3493, 320, 3602, 2 };
    tft.setTouch(calData);


    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX / 10);
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/
  
    lv_main();

  
    while (1) {
         lv_task_handler();
    }
}



/*
0, 334
1, 3439
2, 176
3, 3627
4, 2

0, 230
1, 3575
2, 179
3, 3707
4, 2

*/

void InitTFTDisplay()
{
  lv_init();
  tft.begin(); /* TFT init */
  tft.setRotation(3); /* Landscape orientation */


  uint16_t calData[5] = { 150, 3700, 150,3707 , 1 };
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

      /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/

    lv_main();  // Create LVGL objects

  
    while (1) {
         lv_task_handler();
    }
    
}

int WriteMessageFlag = 0;

// Create LVGL Objects
static void lv_main(){
    
    //LV_THEME_MATERIAL_FLAG_LIGHT
    //LV_THEME_MATERIAL_FLAG_DARK
    
  lv_theme_t * th = lv_theme_material_init(LV_THEME_DEFAULT_COLOR_PRIMARY, LV_THEME_DEFAULT_COLOR_SECONDARY, LV_THEME_MATERIAL_FLAG_LIGHT, LV_THEME_DEFAULT_FONT_SMALL , LV_THEME_DEFAULT_FONT_NORMAL, LV_THEME_DEFAULT_FONT_SUBTITLE, LV_THEME_DEFAULT_FONT_TITLE);     
  lv_theme_set_act(th);

  scr = lv_obj_create(NULL, NULL);
  lv_scr_load(scr);

  //

  InitStyles();
  
  InitHomeScreen();
  
  //InitPanels();

  //InitNavPanel();

  // Inititalise Image from array  
  // my_image_name = lv_img_create(bg_main_panel, NULL);
  // lv_img_set_src(my_image_name, &TestImage);

  /* Create simple label */
  // label = lv_label_create(scr_home, NULL);
  // lv_label_set_text(label, "Startup");
  // lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

  //InitWriteMessageObjects();

 }

void InitStyles()
{
  // Nav panel button style   
  lv_style_set_bg_color(&nav_button_style, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_style_set_radius(&nav_button_style, LV_STATE_DEFAULT, 10);
  lv_style_set_border_width(&nav_button_style, LV_STATE_DEFAULT, 1);
  lv_style_set_border_color(&nav_button_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_border_color(&nav_button_style, LV_STATE_FOCUSED, LV_COLOR_RED);
  lv_style_set_outline_color(&nav_button_style, LV_STATE_FOCUSED, LV_COLOR_WHITE);
  lv_style_set_outline_width(&nav_button_style, LV_STATE_FOCUSED, 0);
  lv_style_set_outline_width(&nav_button_style, LV_STATE_DEFAULT, 0);



  //Nav panel button text style
  lv_style_init(&nav_button_text_style);
  lv_style_set_text_font(&nav_button_text_style, LV_STATE_DEFAULT, &lv_font_montserrat_16);
  lv_style_set_text_color(&nav_button_text_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  // Time date text style
  lv_style_init(&time_date_text_style);
  lv_style_set_text_font(&time_date_text_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
  lv_style_set_text_color(&time_date_text_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  

} 

void InitHomeScreen()
{
  // Initialise the home screen

  scr_home = lv_obj_create(scr, NULL);
  lv_obj_clean_style_list(scr_home, LV_OBJ_PART_MAIN);
  lv_obj_set_style_local_bg_opa(scr_home, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(scr_home, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_MAROON);
  lv_obj_set_pos(scr_home, 0, 0);
  lv_obj_set_size(scr_home, LV_HOR_RES, LV_VER_RES);

  // Initialise view message button object
  lv_obj_t *btn_view_msg = lv_btn_create(scr_home,NULL);
  lv_obj_set_event_cb(btn_view_msg, BtnMessageWriteEventHandler);
  lv_obj_set_pos(btn_view_msg, 20, 20);
  lv_obj_set_size(btn_view_msg, 130, 90);
  lv_obj_add_style(btn_view_msg, LV_BTN_PART_MAIN, &nav_button_style);  /*Overwrite only a some colors to red*/

  // Initialise write message button object
  lv_obj_t *btn_write_msg = lv_btn_create(scr_home,NULL);
  lv_obj_set_event_cb(btn_write_msg, BtnMessageWriteEventHandler);
  lv_obj_set_pos(btn_write_msg, 170, 20);
  lv_obj_set_size(btn_write_msg, 130, 90);
  lv_obj_add_style(btn_write_msg, LV_BTN_PART_MAIN, &nav_button_style);  /*Overwrite only a some colors to red*/
  
  // Initialise reserve message button object
  lv_obj_t *btn_res = lv_btn_create(scr_home,NULL);
  lv_obj_set_event_cb(btn_res, BtnMessageWriteEventHandler);
  lv_obj_set_pos(btn_res, 20, 130);
  lv_obj_set_size(btn_res, 130, 90);
  lv_obj_add_style(btn_res, LV_BTN_PART_MAIN, &nav_button_style);  /*Overwrite only a some colors to red*/

  // Initialise settings message button object
  lv_obj_t *btn_settings = lv_btn_create(scr_home,NULL);
  lv_obj_set_event_cb(btn_settings, BtnMessageWriteEventHandler);
  lv_obj_set_pos(btn_settings, 170, 130);
  lv_obj_set_size(btn_settings, 130, 90);
  lv_obj_add_style(btn_settings, LV_BTN_PART_MAIN, &nav_button_style);  /*Overwrite only a some colors to red*/

  //Labels for buttons
  // Text in view messages button object
  lv_obj_t *label_view_messages_btn = lv_label_create(btn_view_msg, NULL);
  lv_label_set_text(label_view_messages_btn, "View\nMessages");
  lv_obj_align(label_view_messages_btn, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(label_view_messages_btn, LV_LABEL_PART_MAIN,&nav_button_text_style);

  // Text in write message button object
  lv_obj_t *label_write_message_btn = lv_label_create(btn_write_msg, NULL);
  lv_label_set_text(label_write_message_btn, "Write\nMessage");
  lv_obj_align(label_write_message_btn, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(label_write_message_btn, LV_LABEL_PART_MAIN,&nav_button_text_style);

    // Text in write message button object
  lv_obj_t *label_reserve_btn = lv_label_create(btn_res, NULL);
  lv_label_set_text(label_reserve_btn, "");
  lv_obj_align(label_reserve_btn, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(label_reserve_btn, LV_LABEL_PART_MAIN,&nav_button_text_style);

    // Text in write message button object
  lv_obj_t *label_settings_btn = lv_label_create(btn_settings, NULL);
  lv_label_set_text(label_settings_btn, "Settings");
  lv_obj_align(label_settings_btn, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(label_settings_btn, LV_LABEL_PART_MAIN,&nav_button_text_style);


  // Text in write message button object
  lv_obj_t *label_time_date = lv_label_create(scr_home, NULL);
  lv_label_set_text(label_time_date, "Time - Date");
  lv_obj_align(label_time_date, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, 2);
  lv_obj_add_style(label_time_date, LV_LABEL_PART_MAIN,&time_date_text_style);

}






void InitPanels()
{
  bg_main_panel = lv_obj_create(scr, NULL);
  lv_obj_clean_style_list(bg_main_panel, LV_OBJ_PART_MAIN);
  lv_obj_set_style_local_bg_opa(bg_main_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(bg_main_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_WHITE);
  lv_obj_set_pos(bg_main_panel, 0, 0);
  lv_obj_set_size(bg_main_panel, LV_HOR_RES, LV_VER_RES - 50);

  bg_nav_panel = lv_obj_create(scr, NULL);
  lv_obj_clean_style_list(bg_nav_panel, LV_OBJ_PART_MAIN);
  lv_obj_set_style_local_bg_opa(bg_nav_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_OPA_COVER);
  lv_obj_set_style_local_bg_color(bg_nav_panel, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT,LV_COLOR_PINK);
  lv_obj_set_pos(bg_nav_panel, 0, LV_VER_RES - 50);
  lv_obj_set_size(bg_nav_panel, LV_HOR_RES, 50);
}


void InitNavPanel()
{
  // Initialise write message button object
  btn_write_message = lv_btn_create(bg_nav_panel,NULL);
  lv_obj_set_event_cb(btn_write_message, BtnMessageWriteEventHandler);
  lv_obj_set_pos(btn_write_message, 2, 0);
  lv_obj_set_size(btn_write_message, 80, 49);
  lv_obj_add_style(btn_write_message, LV_BTN_PART_MAIN, &nav_button_style);  /*Overwrite only a some colors to red*/
  
  // Text in write button object
  lv_obj_t *label_write_message = lv_label_create(btn_write_message, NULL);
  lv_label_set_text(label_write_message, "Write\nMessage");
  lv_obj_align(label_write_message, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(label_write_message, LV_LABEL_PART_MAIN,&nav_button_text_style);
}

void InitWriteMessageObjects()
{

    /* Create the message box */
    ta_write_message = lv_textarea_create(bg_main_panel, NULL);
    lv_textarea_set_text(ta_write_message, "");
    //lv_textarea_set_pwd_mode(ta_write_message, true);
    //lv_textarea_set_one_line(ta_write_message, true);
    //lv_textarea_set_cursor_hidden(ta_write_message, true);
    lv_obj_set_size(ta_write_message, LV_HOR_RES - 50 - 20, (LV_VER_RES/2) -20 );
    lv_obj_set_pos(ta_write_message, 5, 10);
    lv_obj_set_event_cb(ta_write_message, ta_write_message_event);
    lv_obj_set_hidden(ta_write_message,true);

    // Create keyboard object  
    kb = lv_keyboard_create(scr, NULL);
    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);

    lv_keyboard_set_textarea(kb, ta_write_message);
    lv_obj_set_event_cb(kb, keyboard_event_cb);
    lv_obj_set_hidden(kb,true);
  
}

static void ta_write_message_event(lv_obj_t * ta, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        /* Focus on the clicked text area */
        if(kb != NULL)
        {
          lv_obj_set_hidden(kb,false);
        lv_obj_set_top(kb, true);
        lv_keyboard_set_textarea(kb, ta);
        }
        
        //lv_obj_move_foreground(kb); 
            
    }

    else if(event == LV_EVENT_INSERT) {
       // const char * str = lv_event_get_data();
       // if(str[0] == '\n') {
           // printf("Ready\n");
        //}
    }

}  

static void BtnMessageWriteEventHandler(lv_obj_t * btn_write_message, lv_event_t event)
{
      if(event == LV_EVENT_CLICKED) 
      {
        WriteMessageFlag = !WriteMessageFlag;
        if(WriteMessageFlag == true)
        {
          lv_obj_set_hidden(kb,false);
          lv_obj_set_top(kb, true);
          lv_obj_set_hidden(ta_write_message,false);
        }
        else
        {
          lv_obj_set_hidden(kb,true);
          lv_obj_set_hidden(ta_write_message,true);
        }
        printf("Clicked\n");
      }
      lv_label_set_text(label, "Button Press");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}

// // Initialise the keyboard object
// static void makeKeyboard(){
//   kb = lv_keyboard_create(lv_scr_act(), NULL);
//   lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);
//   lv_keyboard_set_cursor_manage(kb, true);
  
//   //lv_keyboard_set_textarea(kb, ta_password);
//   lv_obj_set_event_cb(kb, keyboard_event_cb);
//   lv_obj_move_background(kb);
// }

// Function to move the keyboard to the foreground when finished 
static void keyboard_event_cb(lv_obj_t * kb, lv_event_t event){
  lv_keyboard_def_event_cb(kb, event);
  
  if(event == LV_EVENT_APPLY){
    lv_obj_set_hidden(kb,true);
    Serial.println("Keyboard");
    
  }else if(event == LV_EVENT_CANCEL){
    lv_obj_set_hidden(kb,true);
    Serial.println("Keyboard1");
  }

}


/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    /* Create simple label */

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/* Reading input device (simulated encoder here) */
bool read_encoder(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static int32_t last_diff = 0;
    int32_t diff = 0; /* Dummy - no movement */
    int btn_state = LV_INDEV_STATE_REL; /* Dummy - no press */

    data->enc_diff = diff - last_diff;;
    data->state = btn_state;

    last_diff = diff;

    return false;
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if(!touched)
    {
      return false;
    }

    if(touchX > SCREEN_WIDTH || touchY > SCREEN_HEIGHT)
    {
      Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);
    }
    else
    {
      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
      data->point.x = touchX;
      data->point.y = touchY;
    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}



