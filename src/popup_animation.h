#pragma once
#include <math.h>

// Use the same curve and speed in both directions, including reversal mid-fade.
static inline float popup_visibility_timed(float current, int closing, float dt,float duration) {
    float next=current+(closing?-1.f:1.f)*fmaxf(dt,0)/fmaxf(.01f,duration);
    return fmaxf(0,fminf(1,next));
}
static inline float popup_visibility(float current,int closing,float dt){return popup_visibility_timed(current,closing,dt,.14f);}
static inline float popup_ease(float value) { return value*value*(3-2*value); }
typedef struct PopupMotion { float value, velocity; } PopupMotion;
// Critically damped motion preserves velocity on repeated taps. About 350 ms
// to cover 95% of a step, with a gentle finish instead of a sudden stop.
static inline void popup_motion_timed(PopupMotion* motion,float target,float dt,float duration) {
    const float omega=4.9f/fmaxf(.05f,duration);
    dt=fmaxf(dt,0);
    float offset=motion->value-target,term=motion->velocity+omega*offset;
    float decay=expf(-omega*dt);
    motion->value=target+(offset+term*dt)*decay;
    motion->velocity=(motion->velocity-omega*term*dt)*decay;
}

static inline void popup_motion(PopupMotion* motion,float target,float dt){popup_motion_timed(motion,target,dt,.35f);}

// Centered cyclic distance with identical geometry on either side of a wrap.
static inline float popup_distance(int index,float phase,int count) {
    if(count<=1)return 0;
    float d=index-phase;return d-floorf((d+count/2.f)/count)*count;
}
// Compact smooth bump: at most two adjacent cards share one focus envelope.
static inline float popup_focus(float distance) {
    float t=fmaxf(0,1-fabsf(distance));return t*t*(3-2*t);
}
