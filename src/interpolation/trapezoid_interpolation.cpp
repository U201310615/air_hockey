//
// Created by troy on 12/29/16.
//

#include "trapezoid_interpolation.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <math.h>

TrapezoidInterpolation::TrapezoidInterpolation() {
    this->type = "trapezoid interpolation";
}

InterpolationState TrapezoidInterpolation::start(const TimeInS now,
                                                 const double start_position,
                                                 const double start_velocity) {
 //   throw new std::logic_error("not implemented");
    if (this->check() != kIntIdle)
        return kIntIdle;

	//added by 周佩 on 2017/1/2.
	//为简化公式，使用一些替换参数a,b,c,x,t求解二次方程

	double t;
	double x;
	double a;
	double b;
	double c;

	double delta_t1;
	double delta_t2;
	double delta_t3;

	t = target_time - now;
	x = position - start_position;
	a = acceleration;
	b = velocity - start_velocity + a * t;
	c = start_velocity * t - x - (start_velocity - velocity) * (start_velocity - velocity) / (2 * a);

    delta_t1 = (b + sqrt(b*b + 4*a*c)) / (2*a);
	delta_t3 = (start_velocity - velocity) / a + delta_t1;
	delta_t2 = t - delta_t1 - delta_t3;

    state = kAcceleration;
    t0 = now;
    s0.position = start_position;
    s0.velocity = start_velocity;
    s0.acceleration = acceleration;

	t1 = now + delta_t1;
    s1.position = s0.position + s0.velocity * delta_t1 + 0.5 * s0.acceleration * delta_t1 * delta_t1;
    s1.velocity = s0.velocity + s0.acceleration * (t1 - t0);
	s1.acceleration = 0;

    t2 = t1 + delta_t2;
    s2.position = s1.position + s1.velocity * (t2 - t1);
    s2.velocity = s1.velocity;
    s2.acceleration = - acceleration;

    t3 = t2 + delta_t3;
    s3.position = position;
    s3.velocity = velocity;
    s3.acceleration = 0;


    std::cerr << "start @" << std::setiosflags(std::ios::fixed) << t0 << std::endl <<
              "acc until " << t1 << std::endl <<
              "dec @" << t2 << std::endl <<
              "stop @" << t3 << std::endl;
    std::cerr << "switch to state <Acc>" << std::endl;
return kIntIdle;
}

InterpolationState TrapezoidInterpolation::move(const TimeInS now) {
 //   throw new std::logic_error("not implemented");
	switch (this->state) {
        case kAcceleration:
            if (now >= t1) {
                this->position = s1.position;
                this->velocity = s1.velocity;
                this->acceleration = s1.acceleration;
                state = kLinear;
                // No Break; Linear Uniform Moving at once
                std::cerr << "switch to state <Linear>" << std::endl;
            } else {
                this->position = s0.position + 0.5 * this->acceleration * (now - t0) * (now - t0);
                this->velocity = s0.velocity + this->acceleration * (now - t0);
                this->acceleration = s0.acceleration;
                return kIntRunning;
            }
            // Linear Uniform Moving
        case kLinear:
            if (now >= t2) {
                this->position = s2.position;
                this->velocity = s2.velocity;
                this->acceleration = s2.acceleration;
                state = kDeceleration;
                // No Break; Decelerating at once
                std::cerr << "switch to state <Dec>" << std::endl;
            } else {
                this->position = s1.position + s1.velocity * (now - t1);
                this->velocity = s1.velocity;
                this->acceleration = s1.acceleration;
                return kIntRunning;
            }
        case kDeceleration:
            if (now >= t3) {
                this->position = s3.position;
                this->velocity = s3.velocity;
                this->acceleration = s3.acceleration;
                state = kEnd;
                // No Break; Stop at once
                std::cerr << "switch to state <End>" << std::endl;
            } else {
                this->position = s2.position + 0.5 * this->acceleration * (now - t2) * (now - t2);
                this->velocity = s2.velocity + this->acceleration * (now - t2);
                this->acceleration = s2.acceleration;
                return kIntRunning;
            }
        case kEnd:
            return kIntDone;
        default:
            std::cerr << "WTF" << std::endl;
            return kIntError;
	}
}

TrapezoidInterpolation::~TrapezoidInterpolation() {
}

