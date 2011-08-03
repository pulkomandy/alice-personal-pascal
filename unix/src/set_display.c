#include <stdio.h>
#include "/slib/boot.h"

#define NUM_COLOURS 4

#define COLOUR_MAP_BASE   0xf900
#define DOT_CLOCK_ADDRESS 0xfa8a

#define COLOUR_RGB  3
#define COLOUR_RGBI 2
#define MONOCHROME  1

#define DOT_CLK_ON  0x00
#define DOT_CLK_OFF 0x01

#define ONE_BPP   1
#define TWO_BPP   2

#define SCREEN_PORT     0xf983
#define LO_RES          0xfb
#define HI_RES          0x04

char *colour_names [16] =
{
"black", "dark red", "dark green", "dark yellow", "dark blue",
"dark violet", "dark turquoise", "grey", "dark grey", "red",
"green", "yellow", "blue", "violet", "turquoise", "white"
};

main (argc, argv)
int argc;
char *argv [];
{
unsigned monitor_type, resolution;
unsigned colour, value, colours [NUM_COLOURS], arg_count;

    if ((argc < 1) || (argc > 7))
    {
        usage();
        exit (-1);
    }
    else if (argc == 1)
    {
        /*
            if set_display is given no arguments, it will
            report the current values in the system table
        */
        read_sys_table ();
        exit (0);
    }

    /* setup default parameters colour values         */
    /* i.e a zero is don't change                     */
    for (colour = 0 ; colour < NUM_COLOURS ; colour++)
        colours [colour] = 0;

    /*  the first argument must be the bpp */
    resolution = atoi (argv [1]);

    if (argc >= 3)
        monitor_type = atoi (argv [2]);
    else
        monitor_type = 0;

    /* at this point we are ready to modifiy the sytem table */
    /* turn off the dot clock for colour and on for monochrome */
    if (monitor_type == COLOUR_RGBI)
        io_out (DOT_CLOCK_ADDRESS, DOT_CLK_OFF);
    else if (monitor_type == COLOUR_RGB)
        io_out (DOT_CLOCK_ADDRESS, DOT_CLK_ON);
    else if (monitor_type == MONOCHROME)
        io_out (DOT_CLOCK_ADDRESS, DOT_CLK_ON);
    else if (monitor_type)
    {
        monitor_type = 0;
        tfprintf (stdout, "Invalid monitor type specified\n");
    }

    /*
     *  Set the display into hi or low res mode depending on the number
     *  of bits/pixel requested. Return an error code if an unsupported
     *  number of bits/pixel are requested.
     */
    if (resolution == ONE_BPP) 
        io_out (SCREEN_PORT, (io_in (SCREEN_PORT) | HI_RES));
    else if (resolution == TWO_BPP) 
        io_out (SCREEN_PORT, (io_in (SCREEN_PORT) & LO_RES));
    else if (resolution)
    {
        resolution = 0;
        tfprintf (stdout, "Invalid resolution specified\n");
    }

    if (!resolution)
    {
        resolution = get_resolution ();
    }

    if (argc > 3)
    {
        /* set arg_count to the number of colours specified */
        arg_count = argc - 3;
        if (arg_count > 2)
        {
            /* modify the colour map from the parameters given */
            for (colour = 0 ; colour < arg_count ; colour++)
            {
                if ((value = atoi (argv [colour + 3])) > 16)
                {
                    tfprintf (stdout,
                    "Invalid colour specified for entry %d\n", colour + 1);
                    continue;
                }

                colours [colour] = value;
            }
        }
        else
        {
            if ((value = atoi (argv [3])) > 16)
            {
                tfprintf (stdout,
                    "Invalid colour specified for entry 1\n");
            }
            else
            {
                colours [0] = value;

                if (resolution == ONE_BPP)
                    colours [2] = colours [0];
            }

            if (arg_count == 2)
            {
                if ((value = atoi (argv [4])) > 16)
                {
                    tfprintf (stdout,
                        "Invalid colour specified for entry 2\n");
                }
                else
                {
                    colours [1] = value;

                    if (resolution == ONE_BPP)
                        colours [3] = colours [1];
                }
            }
        }
    }
 
    update_colour_map (colours [0], colours [1], colours [2], colours [3]);
    update_sys_table (monitor_type, resolution,
        colours [0], colours [1], colours [2], colours [3]);
    
    exit(0);
}

update_colour_map (colour0, colour1, colour2, colour3)
unsigned colour0, colour1, colour2, colour3;
{
    /* modify the colour map */
    if (colour0)
        io_out (COLOUR_MAP_BASE + 0, (colour0 - 1) & 0x0f);
    if (colour1)
        io_out (COLOUR_MAP_BASE + 4, (colour1 - 1) & 0x0f);
    if (colour2)
        io_out (COLOUR_MAP_BASE + 2, (colour2 - 1) & 0x0f);
    if (colour3)
        io_out (COLOUR_MAP_BASE + 6, (colour3 - 1) & 0x0f);
}

update_sys_table (monitor, resolution, colour0, colour1, colour2, colour3)
unsigned monitor, resolution, colour0, colour1, colour2, colour3;
{
struct  sys_table   *st_ptr;
unsigned *int60_ptr;
unsigned mask;
unsigned d1, d2;

    /*
    *   obtain system and boot table address (i.e. segment values)
    */

    set_extra_segment (0);
    int60_ptr = 0x60 * 4 + 2;
    set_extra_segment (@int60_ptr);

    /*
    *   set up screen parameters
    */

    st_ptr = 0;
    
    /* modify colour map contents in the system table */
    /* the definition of this word is assumed to be   */
    /*
        as follows:
             bits        colour
            0 -  3      background
            4 -  7      foreground
            8 - 11      background
           12 - 15      foreground
    */
    
    if (!(mask = @&st_ptr->colour_map))
        mask = readnv (11, &d1, &d2);

    if (colour0 && (--colour0 <= 15))
        mask = (mask & 0xfff0) | (colour0 & 0x000f);

    if (colour1 && (--colour1 <= 15))
        mask = (mask & 0xff0f) | ((colour1 & 0x000f) << 4);

    if (colour2 && (--colour2 <= 15))
        mask = (mask & 0xf0ff) | ((colour2 & 0x000f) << 8);

    if (colour3 && (--colour3 <= 15))
        mask = (mask & 0x0fff) | ((colour3 & 0x000f) << 12);

    @&st_ptr->colour_map = mask;

    if (!(mask = (@&st_ptr->workstation_info << 8) | @&st_ptr->monitor_info))
        mask = readnv (13, &d1, &d2);

    if (monitor == COLOUR_RGBI)
    {
        mask &= 0xfffe;  /* turn off the dot clock */
        mask &= 0xfff3;  /* clear monitor type */
        mask |= 0x0008;  /* set to RGBI monitor type */
    }
    else if (monitor == COLOUR_RGB)
    {
        mask |= 0x0001;  /* turn on the dot clock */
        mask &= 0xfff3;  /* clear monitor type */
        mask |= 0x0004;  /* set to RGB monitor type */
    }
    else if (monitor == MONOCHROME)
    {
        mask |= 0x0001;  /* turn on the dot clock */
        mask &= 0xfff3;  /* clear monitor type */
        mask |= 0x000c;  /* set to MONOCHROME monitor type */
    }

    if (resolution == ONE_BPP)      /* hi res */
    {
        mask &= 0xff3f;  /* clear bits per pixel resolution */
        mask |= 0x00c0;  /* set to 1 bits/pixel mode */
        @&st_ptr->screen_pitch = 80;
        @&st_ptr->screen_pxs = 640;
        @&st_ptr->screen_pys = 240;
        @&st_ptr->screen_bits_per_pixel = resolution;
    }
    else if (resolution == TWO_BPP) /* lo res */
    {
        mask &= 0xff3f;  /* clear bits per pixel resolution */
        mask |= 0x0080;  /* set to 2 bits/pixel mode */
        @&st_ptr->screen_pitch = 40;
        @&st_ptr->screen_pxs = 320;
        @&st_ptr->screen_pys = 240;
        @&st_ptr->screen_bits_per_pixel = resolution;
    }

    @&st_ptr->monitor_info = mask;
    /* This line below screws up the system table. */
    /*  @&st_ptr->workstation_info = mask >> 8 ;    */
}

get_resolution ()
{
struct sys_table *st_ptr;
unsigned *int60_ptr;

    /*
    *   obtain system and boot table address (i.e. segment values)
    */

    set_extra_segment (0);
    int60_ptr = 0x60 * 4 + 2;
    set_extra_segment (@int60_ptr);

    st_ptr = 0;

    return (@&st_ptr->screen_bits_per_pixel);
}

read_sys_table ()
{
    struct sys_table *st_ptr;
    char *ptr;
    unsigned *int60_ptr;
    unsigned mask, monitor_type, resolution;
    int i;

    /*
    *   obtain system and boot table address (i.e. segment values)
    */

    set_extra_segment (0);
    int60_ptr = 0x60 * 4 + 2;
    set_extra_segment (@int60_ptr);

    st_ptr = 0;

    monitor_type =  3 - ((@&st_ptr->monitor_info >> 2) & 0x03) + 1;
    resolution = 3 - ((@&st_ptr->monitor_info >> 6) & 0x03) + 1;

    if (resolution != @&st_ptr->screen_bits_per_pixel)
        resolution = 0;

    /* Report the current bpp setting of the screen */
    /* i.e. whether operating in lo or hi res mode  */
    tfprintf(stdout, "Current screen resolution:\n");
    switch (resolution)
    {
    case 0:
        ptr = "conflicting";
        break;

    case ONE_BPP:
        ptr = "hi";
        break;

    case TWO_BPP:
        ptr = "lo";
        break;

    default:
        ptr = "undefined";
        break;
    }  
    tfprintf(stdout, "    %s resolution mode\n", ptr);


    /* Report the type of monitor currently being used */
    tfprintf(stdout, "Monitor Type:\n");
    switch (monitor_type)
    {
    case COLOUR_RGB:
        ptr = "RGB";
        break;

    case COLOUR_RGBI:
        ptr = "RGBI";
        break;

    case MONOCHROME:
        ptr = "monochrome";
        break;

    default:
        ptr = "undefined";
        break;
    }  
    tfprintf(stdout, "    %s monitor\n", ptr);

    
    /* read colour map contents in the system table */
    mask = @&st_ptr->colour_map;

    /* report colours */
    /* a kludge until we can determine if the hardware */
    /*          matches the design specification       */
    tfprintf(stdout, "Current Colours:\n");
    for (i = 1 ; i <= NUM_COLOURS ; i++)
    {
        tfprintf (stdout, "    colour %2d is %-14s\n",
            i, colour_names [mask & 0x0f]);
        mask >>= 4;
    }
}

usage ()
{
    int i;

    tfprintf(stderr, "Usage: set_display [bpp [mon_type [clr1 [clr2 [clr3 [clr4]]]]]]\n");
    tfprintf(stderr, "    bits per pixel(bpp)\n");
    tfprintf(stderr, "             0 - no change\n");
    tfprintf(stderr, "             1 - hi resolution (640 x 240)\n");
    tfprintf(stderr, "             2 - lo resolution (320 x 240)\n");
    tfprintf(stderr, "    monitor type(mon_typ)\n");
    tfprintf(stderr, "             0 - no change\n");
    tfprintf(stderr, "             1 - monochrome\n");
    tfprintf(stderr, "             2 - RGBI colour\n");
    tfprintf(stderr, "             3 - RGB colour\n");
    tfprintf(stderr, "    clr1 - background colour(available in hi & lo res)\n");
    tfprintf(stderr, "    clr2 - foreground colour(available in hi & lo res)\n");
    tfprintf(stderr, "    clr3 - second background colour (available in lo res only)\n");
    tfprintf(stderr, "    clr4 - second foreground colour (available in lo res only)\n");
    tfprintf(stderr, "    Available Colour Choices:\n");
    tfprintf(stderr, "        %2d: %-14s\n", 0, "no change");
    for (i = 1 ; i <= 15 ; i += 3)
        tfprintf(stderr, "        %2d: %-14s   %2d: %-14s   %2d: %-14s\n",
        i, colour_names[i-1], i+1, colour_names[i], i+2, colour_names[i+1]);
    tfprintf(stderr, "        %2d: %-14s\n", i, colour_names[i-1]);
}
