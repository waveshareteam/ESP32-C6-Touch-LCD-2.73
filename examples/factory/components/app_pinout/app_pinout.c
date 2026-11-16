#include "app_pinout.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
#include "pinout_init/pinout_init.h"
lv_ui guider_ui;
bool app_pinout_open(lv_obj_t *parent)
{
    guider_ui.screen=parent;
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);
    pinout_ui_init(&guider_ui);
    return guider_ui.screen!=NULL;
}
void app_pinout_close(void)
{
    pinout_ui_deinit();
    if(guider_ui.screen) { lv_obj_delete(guider_ui.screen); guider_ui.screen=NULL; }
}
