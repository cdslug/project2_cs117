/*
 * probability.h
 *
 *  Created on: Nov 25, 2013
 *      Author: spencer
 */

#ifndef PROBABILITY_H_
#define PROBABILITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

typedef enum {
    false = 0,
    true = 1
} p_bool; // this can be used for if statements and comparisons in boolean logic

p_bool check_loss(double probability);
p_bool check_corrupt(double probability);
void seed();


#endif /* PROBABILITY_H_ */
