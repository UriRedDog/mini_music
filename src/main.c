/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_LPCOPEN
#include "board.h"
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here
#include "wavetable.h"
#include "tune_still_alive.h"
#include "tune_still_alive_pitch.h"

// TODO: insert other definitions and declarations here
#define POT 8 // power of two; must match with scale_table values
#define ENVPOT 7
#define OSCILLATOR_COUNT 32 // 16 oscillators: 25% load with -O1 (64: 90%)
#define TICKS_LIMIT 203 // 39062 / ( 4 * 48 )
#define CLIP 1016 // 127 * 8



uint16_t increments_pot[ OSCILLATOR_COUNT ];
uint32_t phase_accu_pot[ OSCILLATOR_COUNT ];
uint32_t envelope_positions_envpot[ OSCILLATOR_COUNT ];
uint8_t next_osc = 0;
uint16_t ticks = 0;
uint16_t time = 0;
uint16_t event_index = 0;
int32_t value = 0;
uint8_t osc;
uint16_t phase_accu;

uint16_t event_count = sizeof( tune_still_alive ) / sizeof( tune_still_alive[ 0 ] );
uint32_t sizeof_wt_pot = ( (uint32_t)sizeof( wt ) << POT );
uint32_t sizeof_wt_sustain_pot = ( (uint32_t)sizeof( wt_sustain ) << POT );
uint32_t sizeof_wt_attack_pot = ( (uint32_t)sizeof( wt_attack ) << POT );
uint32_t sizeof_envelope_table_envpot = ( (uint32_t)sizeof( envelope_table ) << ENVPOT );


int main(void)
{
	uint8_t osc;

#ifdef __USE_LPCOPEN
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();

	// Set up and initialize all required blocks and functions
	// related to the board hardware
	Board_Init();

	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif

	for ( osc = 0; osc < OSCILLATOR_COUNT; ++osc )
	{
		increments_pot[ osc ] = 0;
		phase_accu_pot[ osc ] = 0;
		envelope_positions_envpot[ osc ] = 0;
	}
	while ( 1 )
	{
		Board_LED_Set(0, true); // load visualization pin
		while ( time >= tune_still_alive[ event_index ] ) {
			increments_pot[ next_osc ] = scale_table[ tune_still_alive_pitch[ event_index ] ];
			phase_accu_pot[ next_osc ] = 0;
			envelope_positions_envpot[ next_osc ] = 0;
			++next_osc;
			if ( next_osc >= OSCILLATOR_COUNT ) {
				next_osc = 0;
			}
			++event_index;
			if ( event_index >= event_count ) {
				ticks = 0;
				time = 0;
				event_index = 0;
			}
		}
		++ticks;
		if ( ticks >= TICKS_LIMIT ) {
			ticks = 0;
			time += 1;
		}


		for ( osc = 0; osc < OSCILLATOR_COUNT; ++osc ) {
			phase_accu_pot[ osc ] += increments_pot[ osc ];
			if ( phase_accu_pot[ osc ] >= sizeof_wt_pot ) {
				phase_accu_pot[ osc ] -= sizeof_wt_sustain_pot;
			}
			phase_accu = ( phase_accu_pot[ osc ] >> POT );
			value += envelope_table[ envelope_positions_envpot[ osc ] >> ENVPOT ] * wt[ phase_accu ];
			if ( phase_accu_pot[ osc ] >= sizeof_wt_attack_pot &&
					envelope_positions_envpot[ osc ] < sizeof_envelope_table_envpot - 1 )
			{
				++envelope_positions_envpot[ osc ];
			}
		}
		value >>= 8; // envelope_table resolution
		if ( value > CLIP )
		{
			value = CLIP;
		} else if ( value < -CLIP )
		{
			value = -CLIP;
		}
		Board_LED_Set(0, false); // load visualization pin
//            while ( TMR2 > 10 )
//            {
//            }
//            OC1RS = 1024 + value;
	}


    return 0 ;
}
