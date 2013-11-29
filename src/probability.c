/*
 * probability.c
 *
 *  Created on: Nov 25, 2013
 *      Author: spencer
 */

#include "probability.h"

void seed(){
	srand(time(NULL));
}

// returns whether or not packet was lost
bool p_check(double probability) {
	double p_val; // the value which stores our virtual "dice roll"

	assert(probability >= 0 && probability <= 1); // valid probability is within the range 0 to 1

	p_val = rand()/(double)RAND_MAX;
	if(p_val > probability){
		return false;
	}
	else
		return true;
}
