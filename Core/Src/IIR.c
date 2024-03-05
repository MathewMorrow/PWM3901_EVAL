/*
 * IIR.c
 *
 *  Created on: Jan. 14, 2023
 *      Author: Mathew
 */


#include "IIR.h"
#include "math.h"



/* PT1 Low Pass Filter -- First Order IIR Low Pass */

float pt1FilterGain(float f_cut, float dT)
{
    float RC = 1 / (2 * M_PI * f_cut);
    return dT / (RC + dT);
}

void pt1FilterInit(pt1Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->k = k;
}

void pt1FilterUpdateCutoff(pt1Filter_t *filter, float k)
{
    filter->k = k;
}

float pt1FilterApply(pt1Filter_t *filter, float input)
{
    filter->state = filter->state + filter->k * (input - filter->state);
    return filter->state;
}

/* PT2 Low Pass Filter -- Second Order IIR Low Pass */

float pt2FilterGain(float f_cut, float dT)
{
    const float order = 2.0f;
    const float orderCutoffCorrection = 1 / sqrtf(powf(2, 1.0f / order) - 1);
    float RC = 1 / (2 * orderCutoffCorrection * M_PI * f_cut);
    // float RC = 1 / (2 * 1.553773974f * M_PIf * f_cut);
    // where 1.553773974 = 1 / sqrt( (2^(1 / order) - 1) ) and order is 2
    return dT / (RC + dT);
}

void pt2FilterInit(pt2Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->state1 = 0.0f;
    filter->k = k;
}

void pt2FilterUpdateCutoff(pt2Filter_t *filter, float k)
{
    filter->k = k;
}

float pt2FilterApply(pt2Filter_t *filter, float input)
{
    filter->state1 = filter->state1 + filter->k * (input - filter->state1);
    filter->state = filter->state + filter->k * (filter->state1 - filter->state);
    return filter->state;
}

/* PT3 Low Pass Filter -- Third Order IIR Low Pass */

float pt3FilterGain(float f_cut, float dT)
{
    const float order = 3.0f;
    const float orderCutoffCorrection = 1 / sqrtf(powf(2, 1.0f / order) - 1);
    float RC = 1 / (2 * orderCutoffCorrection * M_PI * f_cut);
    // float RC = 1 / (2 * 1.961459177f * M_PIf * f_cut);
    // where 1.961459177 = 1 / sqrt( (2^(1 / order) - 1) ) and order is 3
    return dT / (RC + dT);
}

void pt3FilterInit(pt3Filter_t *filter, float k)
{
    filter->state = 0.0f;
    filter->state1 = 0.0f;
    filter->state2 = 0.0f;
    filter->k = k;
}

void pt3FilterUpdateCutoff(pt3Filter_t *filter, float k)
{
    filter->k = k;
}

float pt3FilterApply(pt3Filter_t *filter, float input)
{
    filter->state1 = filter->state1 + filter->k * (input - filter->state1);
    filter->state2 = filter->state2 + filter->k * (filter->state1 - filter->state2);
    filter->state = filter->state + filter->k * (filter->state2 - filter->state);
    return filter->state;
}


/* Dynamic PT3 Low Pass Filter -- Third Order IIR Low Pass with two cutoff frequencies */

void ptd3FilterInit(ptd3Filter_t *filter, float _thresh, float initialization, float _kslow, float _kfast)
{
    filter->state = initialization;
    filter->state1 = initialization;
    filter->state2 = initialization;
    filter->thresh = _thresh;
    filter->kslow = _kslow;
    filter->kfast = _kfast;
}

void ptd3FilterUpdateCutoffs(ptd3Filter_t *filter, float _kslow, float _kfast)
{
    filter->kslow = _kslow;
    filter->kfast = _kfast;
}

float ptd3FilterApply(ptd3Filter_t *filter, float input)
{
	float k = filter->kslow;
	float delta = input - filter->state;

//	if(change > filter->thresh || change < -filter->thresh)
//	{
//		k = filter->kfast;
//	}
//
//    filter->state1 = filter->state1 + k*(input - filter->state1);
//    filter->state2 = filter->state2 + k*(filter->state1 - filter->state2);
//    filter->state = filter->state + k*(filter->state2 - filter->state);
//    return filter->state;


	if(delta > filter->thresh)
	{
		filter->state += 0.1;
		filter->state1 += 0.1;
		filter->state2 += 0.1;
	}
	else if(delta < -filter->thresh)
	{
		filter->state -= 0.05;
		filter->state1 -= 0.05;
		filter->state2 -= 0.05;
	}
	else
	{
	    filter->state1 = filter->state1 + k*(input - filter->state1);
	    filter->state2 = filter->state2 + k*(filter->state1 - filter->state2);
	    filter->state = filter->state + k*(filter->state2 - filter->state);
	}

    return filter->state;
}
