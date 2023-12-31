SET(SOURCES components/ui_comp_normalbtn.c
    components/ui_comp.c
    screens/ui_mainScreen.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES lvgl)
