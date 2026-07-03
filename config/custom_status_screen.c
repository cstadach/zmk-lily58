/*
 * Custom status screen for Lily58 + nice!view displays (160x68 px, 1-bit).
 *
 * Left half  (central):   BT/battery bar | BT profile [1] 2 3 | layer name
 * Right half (peripheral): BT/battery bar | Link sprite (48x48)
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/display/widgets/output_status.h>

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/display/widgets/layer_status.h>
#include <zmk/ble.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

/* ── Shared ─────────────────────────────────────────────────────────────── */

static struct zmk_widget_battery_status battery_widget;
static struct zmk_widget_output_status output_widget;

/* File-scope: lv_line_set_points stores a pointer, must outlive the call. */
static lv_point_t divider_pts[] = {{0, 17}, {159, 17}};

/* ── Left half (central) ─────────────────────────────────────────────────── */

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

static struct zmk_widget_layer_status layer_widget;
static lv_obj_t *profile_label;

static void update_profile_label(void) {
    int idx = zmk_ble_active_profile_index();
    char buf[20];
    snprintf(buf, sizeof(buf), " %s1%s   %s2%s   %s3%s ",
        idx == 0 ? "[" : " ", idx == 0 ? "]" : " ",
        idx == 1 ? "[" : " ", idx == 1 ? "]" : " ",
        idx == 2 ? "[" : " ", idx == 2 ? "]" : " ");
    lv_label_set_text(profile_label, buf);
}

static void profile_work_cb(struct k_work *work) {
    update_profile_label();
}
K_WORK_DEFINE(profile_work, profile_work_cb);

static int on_ble_profile_changed(const zmk_event_t *eh) {
    k_work_submit_to_queue(zmk_display_work_q(), &profile_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(profile_display, on_ble_profile_changed);
ZMK_SUBSCRIPTION(profile_display, zmk_ble_active_profile_changed);

/* ── Right half (peripheral) ─────────────────────────────────────────────── */

#else

/*
 * 48x48 monochrome Link sprite (48 rows x 6 bytes = 288 bytes).
 *
 * To generate:
 *   1. Download your avatar:  curl -sL "https://avatars.githubusercontent.com/u/80837?v=4&size=64" -o avatar.png
 *   2. Run:                   python3 convert_image.py avatar.png
 *   3. Replace the array body below with the output.
 *   4. If Link looks inverted re-run with: python3 convert_image.py avatar.png --invert
 *
 * The explicit size [6 * 48] ensures sizeof() is correct even with a
 * partial initializer, so data_size in the descriptor stays accurate.
 */
static const uint8_t link_sprite_map[6 * 48] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  1 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  2 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  3 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  4 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  5 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  6 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  7 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  8 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row  9 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 11 */
    0x00, 0x01, 0xFF, 0xFF, 0x80, 0x00,  /* row 12 */
    0x00, 0x01, 0xFF, 0xFF, 0x80, 0x00,  /* row 13 */
    0x00, 0x01, 0xFF, 0xFF, 0x80, 0x00,  /* row 14 */
    0x00, 0x00, 0x00, 0x03, 0x80, 0x00,  /* row 15 */
    0x00, 0x00, 0x00, 0x03, 0x80, 0x00,  /* row 16 */
    0x00, 0x00, 0x00, 0x03, 0x80, 0x00,  /* row 17 */
    0x00, 0x00, 0x38, 0x03, 0x80, 0x00,  /* row 18 */
    0x00, 0x00, 0x38, 0x03, 0x80, 0x00,  /* row 19 */
    0x00, 0x00, 0x38, 0x03, 0x80, 0x00,  /* row 20 */
    0x00, 0x00, 0x07, 0xE0, 0x0E, 0x00,  /* row 21 */
    0x00, 0x00, 0x07, 0xE0, 0x0E, 0x00,  /* row 22 */
    0x00, 0x00, 0x07, 0xE0, 0x0E, 0x00,  /* row 23 */
    0x03, 0xFF, 0xC0, 0x00, 0x7E, 0x00,  /* row 24 */
    0x03, 0xFF, 0xC0, 0x00, 0x7E, 0x00,  /* row 25 */
    0x03, 0xFF, 0xC0, 0x00, 0x7E, 0x00,  /* row 26 */
    0x1F, 0x8F, 0xF8, 0x00, 0x0E, 0x00,  /* row 27 */
    0x1F, 0x8F, 0xF8, 0x00, 0x0E, 0x00,  /* row 28 */
    0x1F, 0x8F, 0xF8, 0x00, 0x0E, 0x00,  /* row 29 */
    0x1F, 0x8F, 0xC0, 0xFF, 0x80, 0x00,  /* row 30 */
    0x1F, 0x8F, 0xC0, 0xFF, 0x80, 0x00,  /* row 31 */
    0x1F, 0x8F, 0xC0, 0xFF, 0x80, 0x00,  /* row 32 */
    0x1F, 0x8F, 0xC7, 0xE0, 0x00, 0x00,  /* row 33 */
    0x1F, 0x8F, 0xC7, 0xE0, 0x00, 0x00,  /* row 34 */
    0x1F, 0x8F, 0xC7, 0xE0, 0x00, 0x00,  /* row 35 */
    0x1F, 0xFF, 0xC0, 0x00, 0x00, 0x00,  /* row 36 */
    0x1F, 0xFF, 0xC0, 0x00, 0x00, 0x00,  /* row 37 */
    0x1F, 0xFF, 0xC0, 0x00, 0x00, 0x00,  /* row 38 */
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00,  /* row 39 */
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00,  /* row 40 */
    0x00, 0x00, 0x38, 0x00, 0x00, 0x00,  /* row 41 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 42 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 43 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 44 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 45 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 46 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* row 47 */
};

static const lv_img_dsc_t link_sprite = {
    .header = {
        .cf = LV_IMG_CF_ALPHA_1BIT,  /* 1=draw black, 0=transparent */
        .w  = 48,
        .h  = 48,
    },
    .data_size = sizeof(link_sprite_map),
    .data      = link_sprite_map,
};

#endif /* IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) */

/* ── Screen builder ──────────────────────────────────────────────────────── */

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(screen);

    /* Status bar: BT/USB output (left) + battery % (right) */
    zmk_widget_output_status_init(&output_widget, screen);
    lv_obj_align(zmk_widget_output_status_obj(&output_widget),
                 LV_ALIGN_TOP_LEFT, 2, 2);

    zmk_widget_battery_status_init(&battery_widget, screen);
    lv_obj_align(zmk_widget_battery_status_obj(&battery_widget),
                 LV_ALIGN_TOP_RIGHT, -2, 2);

    lv_obj_t *divider = lv_line_create(screen);
    lv_line_set_points(divider, divider_pts, 2);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

    /* Profile selector: highlights active profile in brackets */
    profile_label = lv_label_create(screen);
    lv_obj_align(profile_label, LV_ALIGN_TOP_MID, 0, 22);
    update_profile_label();  /* safe: called from display work queue context */

    /* Current layer name */
    zmk_widget_layer_status_init(&layer_widget, screen);
    lv_obj_align(zmk_widget_layer_status_obj(&layer_widget),
                 LV_ALIGN_BOTTOM_MID, 0, -4);

#else

    /* Link sprite, bottom-centered in the 50px content area below the bar */
    lv_obj_t *img = lv_img_create(screen);
    lv_img_set_src(img, &link_sprite);
    /* Alpha-mask images need an explicit foreground color to be visible. */
    lv_obj_set_style_img_recolor(img, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
    lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 0, 0);

#endif

    return screen;
}
