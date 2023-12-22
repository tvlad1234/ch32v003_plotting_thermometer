
#include <stdint.h>
#include <stdio.h>
#include "gfx.h"
#include "plot.h"

uint16_t map_float_to_u16( float x, float in_min, float in_max, uint16_t out_min, uint16_t out_max )
{
	return ( x - in_min ) * ( out_max - out_min ) / ( in_max - in_min ) + out_min;
}

void plot_render( plot_t *plot, gfx_inst *gfx )
{
	char s[11];

	float min = plot->val_min;
	float max = plot->val_max;

	int intMin = min;
	int decMin = ( (int)( min * 10 ) % 10 );

	int intMax = max;
	int decMax = ( (int)( max * 10 ) % 10 );

	gfx_set_cursor( gfx, plot->pos_x, plot->pos_y );

	if ( decMax )
		snprintf( s, 10, "%d.%d", intMax, decMax );
	else
		snprintf( s, 10, "%d", intMax );

	int labelMaxLen = strlen( s );
	gfx_print_string( gfx, s );


	gfx_set_cursor( gfx, plot->pos_x, plot->pos_y + plot->height - 8 );

	if ( decMin )
		snprintf( s, 10, "%d.%d", intMin, decMin );
	else
		snprintf( s, 10, "%d", intMin );

	int labelMinLen = strlen( s );
	gfx_print_string( gfx, s );

	int labelLen = ( ( labelMaxLen > labelMinLen ) ? labelMaxLen : labelMinLen ) * 6;

	plot->effective_area = plot->width - labelLen - 1;

	gfx_draw_fast_v_line( gfx, plot->pos_x + labelLen, plot->pos_y, plot->height - 4, plot->color_frame );

	uint16_t currentX = plot->pos_x + labelLen + 1;
	uint16_t prevX;

	uint16_t currentY, prevY;

	gfx_draw_fast_h_line(
		gfx, plot->pos_x + labelLen, plot->pos_y + plot->height - 4, plot->width - labelLen, plot->color_frame );

	for ( int i = 0; i < plot->point_num && currentX <= plot->pos_x + plot->width; i++ )
	{
		currentY = plot->height - 4 -
		           map_float_to_u16( plot->points[i], plot->val_min, plot->val_max, 0, plot->height - 4 ) - 1 +
		           plot->pos_y;

		if ( i ) gfx_draw_line( gfx, prevX, prevY, currentX, currentY, plot->color_plot );

		if ( i && plot->circle_points ) gfx_draw_circle( gfx, currentX, currentY, plot->circle_radius, plot->circle_color );

		prevX = currentX;
		currentX += plot->point_dist;
		prevY = currentY;
	}
}

void plot_add_point( plot_t *plot, float val )
{
	if ( plot->autorange )
	{
		if ( val > plot->val_max ) plot->val_max = val;
		if ( val < plot->val_min ) plot->val_min = val;
	}

	uint16_t visiblePoints = plot->effective_area / plot->point_dist;
	if ( plot->point_num < visiblePoints )
		plot->point_num++;
	else
		for ( int i = 1; i < plot->point_num; i++ )
		{
			plot->points[i - 1] = plot->points[i];
		}
	plot->points[plot->point_num - 1] = val;
}
