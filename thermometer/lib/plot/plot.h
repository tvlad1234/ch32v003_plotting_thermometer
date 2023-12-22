#ifndef _PLOT_H
#define _PLOT_H

#include <stdint.h>

struct plot_t
{
	uint16_t pos_x, pos_y, width, height;
	float val_min, val_max;
	uint16_t color_frame, color_bg, color_plot;
	uint16_t point_num;
	float *points;
	uint16_t point_dist;

	uint8_t autorange, circle_points, circle_radius;
    uint16_t circle_color;
    
	uint16_t effective_area;
};

typedef struct plot_t plot_t;

void plot_render( plot_t *plot, gfx_inst *gfx );
void plot_add_point( plot_t *plot, float val );

#endif