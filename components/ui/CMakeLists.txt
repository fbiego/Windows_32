
# SET(SOURCES screens/ui_boot.c
#     screens/ui_desktop.c
#     ui.c
#     ui_helpers.c
#     images/ui_img_1214273956.c
#     images/ui_img_wallpaper_1_png.c
#     images/ui_img_1_png.c
#     images/ui_img_2_png.c
#     images/ui_img_190695247.c
#     images/ui_img_997471977.c)

# add_library(ui ${SOURCES})

set(requires lvgl)

idf_component_register(
    SRC_DIRS .
    INCLUDE_DIRS .
    REQUIRES ${requires}
)
