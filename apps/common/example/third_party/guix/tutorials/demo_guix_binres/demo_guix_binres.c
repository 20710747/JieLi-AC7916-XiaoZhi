/* This is a small demo of the high-performance GUIX graphics framework. */

#include <stdio.h>
#include "gx_api.h"

#include "demo_guix_binres_resources.h"
#include "demo_guix_binres_specifications.h"

#include "app_config.h"

/* This demo can be built and run on evaluation boards such as the STM32F439, in which
   case the binary resource data is programmed to FLASH prior to running this demo.
*/
#define DEFAULT_CANVAS_PIXELS (MAIN_DISPLAY_X_RESOLUTION * MAIN_DISPLAY_Y_RESOLUTION)

GX_WINDOW_ROOT               *root;

GX_VALUE language_count;


GX_LANGUAGE_HEADER language_info[8];

/* reference the display table, theme table, and language table
   produced by GUIX Studio
*/
extern GX_STUDIO_DISPLAY_INFO demo_guix_binres_display_table[];
extern GX_CONST GX_THEME     *main_display_theme_table[];
extern GX_CONST GX_UBYTE    **main_display_language_table[];

/* pointers which will be initialized when the binary resources
   are installed
*/
GX_THEME   *theme = GX_NULL;
GX_STRING **language_table = GX_NULL;
GX_UBYTE   *binres_root_address = GX_NULL;
GX_CHAR     binres_pathname[] = CONFIG_ROOT_PATH"demo_guix_binres_resources.bin";

/* Define prototypes.   */
VOID demo_thread_entry(ULONG thread_input);

GX_COLOR default_canvas_memory[DEFAULT_CANVAS_PIXELS];

/* local application function prototypes */

VOID *load_binary_resource_data_to_ram();
UINT load_theme(GX_UBYTE *root_address, INT theme_id);
UINT load_language_table(GX_UBYTE *root_address);

UINT load_language_info(GX_UBYTE *root_address, GX_LANGUAGE_HEADER *put_info);
UINT load_language_count(GX_UBYTE *root_address, GX_VALUE *put_count);


/************************************************************************************/
/*  User defined memory allocate function.                                          */
/************************************************************************************/
VOID *memory_allocate(ULONG size)
{
    return malloc(size);
}

/************************************************************************************/
/*  User defined memory free function.                                              */
/************************************************************************************/
VOID memory_free(VOID *mem)
{
    free(mem);
}

/************************************************************************************/
/*  Program entry.                                                                  */
/************************************************************************************/
int main(int argc, char **argv)
{
    /* Enter the ThreadX kernel. */
    tx_kernel_enter();
    return (0);
}

/************************************************************************************/
/*  Define system initialization.                                                   */
/************************************************************************************/
VOID tx_application_define(void *first_unused_memory)
{
    thread_fork("Demo Thread", GX_SYSTEM_THREAD_PRIORITY + 1, 1024, 0, NULL, demo_thread_entry, 0);
}

VOID demo_thread_entry(ULONG thread_input)
{
    /* Initialize GUIX.  */
    gx_system_initialize();

    /* Set memory alloc/free functions. */
    gx_system_memory_allocator_set(memory_allocate, memory_free);

    /* Create a display driver based on the target platform */
    gx_studio_display_configure(MAIN_DISPLAY, jl_graphics_driver_setup_565rgb,
                                LANGUAGE_ENGLISH, MAIN_DISPLAY_THEME_1, &root);

    /* initialize the canvas memory pointer (not allocated by Studio) */

    root -> gx_window_root_canvas->gx_canvas_memory = default_canvas_memory;

    /* Load binary resource file into ram. */
    /* This suit the case that we generated a .bin format resource file. */
    binres_root_address = load_binary_resource_data_to_ram();


    /* Load language table. */
    load_language_table(binres_root_address);

    /* Load theme table. */
    load_theme(binres_root_address, 0);

    /* Create the "simple_window" screen. */
    gx_studio_named_widget_create("simple_window", (GX_WIDGET *)root, GX_NULL);

    gx_binres_language_count_get(binres_root_address, &language_count);
    gx_binres_language_info_load(binres_root_address, language_info);

    /* Show the root window to make "simple_window" screen visible.  */
    gx_widget_show(root);

    /* Start GUIX thread */
    gx_system_start();
}

/************************************************************************************/
/*  Override default event processing of "simple_window" to handle signals from     */
/*  child widgets.                                                                  */
/************************************************************************************/
UINT simple_window_event_handler(GX_WINDOW *widget, GX_EVENT *event_ptr)
{
    switch (event_ptr->gx_event_type) {
    case GX_SIGNAL(ID_THEME_1, GX_EVENT_RADIO_SELECT):
        /* Load first theme from generated binary resource data. */
        load_theme(binres_root_address, 0);
        break;

    case GX_SIGNAL(ID_THEME_2, GX_EVENT_RADIO_SELECT):
        /* Load second theme from generated binary resource data. */
        load_theme(binres_root_address, 1);
        break;

    case GX_SIGNAL(ID_ENGLISH, GX_EVENT_RADIO_SELECT):
        /* Set action language to English. */
        gx_system_active_language_set(LANGUAGE_ENGLISH);
        break;

    case GX_SIGNAL(ID_SPANISH, GX_EVENT_RADIO_SELECT):
        /* Set active language to Spanish. */
        gx_system_active_language_set(LANGUAGE_SPANISH);
        break;

    default:
        /* for all other events, allow the default event
           processor to run
        */
        return gx_window_event_process(widget, event_ptr);
    }

    return 0;
}

/************************************************************************************/
/*  Load binary resource data into ram.                                             */
/*  This is an example of how an application might load a binary resource data
    file to RAM (or FLASH) from a local filesystem. In this case we are using generic ANSI
    file I/O to read the resource data file.

    Note that for a typical embedded target, this step is not required. For most
    targets, the binary resource data has been directly programmed to FLASH rather
    than saved in a filesytem.
*/
/************************************************************************************/
VOID *load_binary_resource_data_to_ram()
{
    UCHAR *address = GX_NULL;

    /* If generated resource is stored in a FAT filesystem, it must be
       loaded into memory before it can be used. This memory could be
       RAM or FLASH, but for the purpose of this example will simply
       read the file to RAM. */
    FILE *p_file;
    size_t total_length;


#ifdef CONFIG_JLFAT_ENABLE
    char path[128];
    path[long_file_name_encode(binres_pathname, (UCHAR *)path, sizeof(path))] = '\0';
    p_file = fopen(path, "r");
#else
    p_file = fopen(binres_pathname, "r");
#endif
    if (p_file) {
        total_length = flen(p_file);
        address = memory_allocate(total_length);
        fread(address, 1, total_length, p_file);
        fclose(p_file);

        printf("load_binary_resource[%dKB]_data_to_ram\r\n", total_length / 1024);
    } else {
        printf("load_binary_resource_data_to_ram fail !!! \r\n");
    }


    return address;
}

/************************************************************************************/
/*  Load a theme.                                                                   */
/************************************************************************************/
UINT load_theme(GX_UBYTE *root_address, INT theme_id)
{
    UINT  status = GX_SUCCESS;

    if (theme) {
        memory_free(theme);
        theme = GX_NULL;
    }

    /* Load a theme from binary data memory. */
    status = gx_binres_theme_load(root_address,/* Address of binary resource data. */
                                  theme_id,    /* Theme identification, 0, 1, 2: 1th, 2nd, 3rd theme in the binary resource data. */
                                  &theme);     /* Loaded theme. */

    if (status == GX_SUCCESS) {
        /* Install themes to specified display. */
        gx_display_theme_install(demo_guix_binres_display_table[MAIN_DISPLAY].display, theme);
    }

    return status;
}

/************************************************************************************/
/*  Load language table.                                                            */
/************************************************************************************/
UINT load_language_table(GX_UBYTE *root_address)
{
    UINT  status = GX_SUCCESS;

    if (language_table) {
        memory_free(language_table);
        language_table = GX_NULL;
    }

    /* Load language table from binary data memory. */
    status = gx_binres_language_table_load_ext(root_address,     /* Address of binary resource data. */
             &language_table); /* Loaded language table that contains all languages in the specified binary resource data. */

    if (language_table) {
        /* Set language table. */
        gx_display_language_table_set_ext(root->gx_window_root_canvas->gx_canvas_display, (GX_CONST GX_STRING **)language_table, MAIN_DISPLAY_LANGUAGE_TABLE_SIZE, MAIN_DISPLAY_STRING_TABLE_SIZE);

        /* Set active language. */
        gx_system_active_language_set(LANGUAGE_ENGLISH);
    }
    return status;
}
