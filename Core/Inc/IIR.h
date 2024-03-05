/*
 * IIR.h
 *
 *  Created on: Jan. 14, 2023
 *      Author: Mathew
 */

#ifndef INC_IIR_H_
#define INC_IIR_H_



typedef struct pt1Filter_s {
    float state;
    float k;
} pt1Filter_t;

typedef struct pt2Filter_s {
    float state;
    float state1;
    float k;
} pt2Filter_t;

typedef struct pt3Filter_s {
    float state;
    float state1;
    float state2;
    float k;
} pt3Filter_t;

typedef struct ptd3Filter_s {
    float state;
    float state1;
    float state2;
    float thresh;
    float kslow;
    float kfast;
} ptd3Filter_t;


float pt1FilterGain(float f_cut, float dT);
void pt1FilterInit(pt1Filter_t *filter, float k);
void pt1FilterUpdateCutoff(pt1Filter_t *filter, float k);
float pt1FilterApply(pt1Filter_t *filter, float input);

float pt2FilterGain(float f_cut, float dT);
void pt2FilterInit(pt2Filter_t *filter, float k);
void pt2FilterUpdateCutoff(pt2Filter_t *filter, float k);
float pt2FilterApply(pt2Filter_t *filter, float input);

float pt3FilterGain(float f_cut, float dT);
void pt3FilterInit(pt3Filter_t *filter, float k);
void pt3FilterUpdateCutoff(pt3Filter_t *filter, float k);
float pt3FilterApply(pt3Filter_t *filter, float input);

void ptd3FilterInit(ptd3Filter_t *filter, float _thresh, float initialization, float _kslow, float _kfast);
void ptd3FilterUpdateCutoffs(ptd3Filter_t *filter, float _kslow, float _kfast);
float ptd3FilterApply(ptd3Filter_t *filter, float input);


#endif /* INC_IIR_H_ */
