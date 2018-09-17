/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * ------------------------------------
 * General Utilities
 * ------------------------------------
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_


/**
 * -------------------------------------------------------------
 * Macros
 * -------------------------------------------------------------
 */

#define MIN(_a_,_b_)              ((_a_ < _b_) ? _a_ : _b_)
#define MAX(_a_,_b_)              ((_a_ > _b_) ? _a_ : _b_)
#define FLOOR(_a_)                ( (int)_a_ )
#define CEIL(_a_)                 ( ((int)_a_ == _a_) ? _a_ : (((int)_a_)+1) )
//#define ABS(_a_)                    ( _a_ < 0 ? (-1)*_a_ : _a_ )

/**
 * -------------------------------------------------------------
 * Constants
 * -------------------------------------------------------------
 */

const int   INF_WIDTH   = -1;         /**< infinite width */
const int   INF   		= std::numeric_limits<int>::max();

#endif /* UTIL_HPP_ */
