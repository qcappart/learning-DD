/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * -------------------------------------------------------------------
 * Statistic routines for profiling
 * -------------------------------------------------------------------
 */

#ifndef STATS_HPP_
#define STATS_HPP_

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <map>
#include <vector>

#define UNITIALIZED_STAT    -1    // initial value stat field receives
#define INITIAL_STAT_SIZE  100    // initial number of statistics considered


using namespace std;

/**
 * Struct for C-string comparison
 */
struct ltstr {
    bool operator()(const char* s1, const char* s2) const {
        return( strcmp(s1, s2) < 0 );
    }
};

/**
 * Struct for statistic value
 */
struct data_t {
    int id;
    data_t(): id(UNITIALIZED_STAT) { }
};


/**
 * Class to collect general statistics on the code
 */
class Stats {

public:

    /** Constructor: Just reserve memory */
    Stats() {
        timer_start.reserve(INITIAL_STAT_SIZE);
        value.reserve(INITIAL_STAT_SIZE);
    }

    /** Register a new statistic. The initial value is 0 by default */
    int register_name(const char* name, long int initial_value = 0);

    /** Start timer for a time statistic */
    void start_timer(const char* name);

    /** End timer for a time statistic, accumulating the result (in seconds) */
    void end_timer(const char* name);

    /** Start timer (by id) for a time statistic */
    void start_timer(int id);

    /** End timer (by id) for a time statistic, accumulating the result (in seconds) */
    void end_timer(int id);

    /** Add value for a numerical statistic by name. Default value to add is 1. */
    void add_value(const char* name, long int val=1);

    /** Add value for a numerical statistic by id. Default value to add is 1. */
    void add_value(int id, long int val=1);

    /** Get value for a statistic*/
    long int get_value(const char* name);

    /** Get value for a statistic*/
    long int get_value(int id);

    /** Return value (by name) interpreted as time */
    double get_time(const char* name);

    /** Return value (by id) interpreted as time */
    double get_time(int id);

    /** Return value (by name) interpreted as time, taking current time clock for measure */
    double get_current_time(const char* name);

    /** Return value (by id) interpreted as time, taking current time clock for measure */
    double get_current_time(int id);

    /** Return id of name */
    int get_id(const char* name);

private:

    map<const char*, data_t, ltstr>  name_to_id;     /**< map from name to stat identifier */
    vector<clock_t>                  timer_start;    /**< timer start for statistic */
    vector<long int>                 value;          /**< statistic value */
};


/**
 * -------------------------------------------------------------
 * Inline Implementations
 * -------------------------------------------------------------
 */


/**
 * Register a new statistic
 * */
inline int Stats::register_name(const char* name, long int initial_value) {
    name_to_id[name].id = value.size();
    value.push_back(initial_value);
    timer_start.push_back(clock());
    return value.size()-1;
}


/**
 * Start the timer for a statistic
 */
inline void Stats::start_timer(const char* name) {
    start_timer(get_id(name));
}


/**
 * Start the timer (by id) for a statistic
 */
inline void Stats::start_timer(int id) {
    assert( id >= 0 && id < (int)value.size() );
    timer_start[id] = clock();
}

/**
 * End the timer for a statistic, accumulating the result (in seconds)
 */
inline void Stats::end_timer(const char* name) {
    end_timer(get_id(name));
}

/**
 * End the timer for a statistic (by id), accumulating the result (in seconds)
 */
inline void Stats::end_timer(int id) {
    assert( id >= 0 && id < (int)value.size() );
    value[id] += clock() - timer_start[id];
}

/**
 * Add value for a numerical statistic. Default value to add is 1.
 */
inline void Stats::add_value(const char* name, long int val) {
    add_value(get_id(name), val);
}

/**
 * Add value by id
 */
inline void Stats::add_value(int id, long int val) {
    assert( id >= 0 && id < (int)value.size() );
    value[id] += val;
}


/**
 * Get value for a statistic
 */
inline long int Stats::get_value(const char* name) {
    return( get_value(get_id(name)) );
}


/**
 * Get value for a statistic
 */
inline long int Stats::get_value(int id) {
    assert( id >= 0 && id < (int)value.size() );
    return( value[id] );
}



/**
 * Get value for a statistic interpreted as time
 */
inline double Stats::get_time(const char* name) {
    return get_time(get_id(name));
}


/**
 * Get value for a statistic interpreted as time, using current time as measure
 */
inline double Stats::get_current_time(const char* name) {
    return get_current_time(get_id(name));
}


/**
 * Get value for a statistic interpreted as time, using current time as measure
 */
inline double Stats::get_current_time(int id) {
    assert( id >= 0 && id < (int)value.size() );
    return ((double)(clock() - timer_start[id])) / (double)CLOCKS_PER_SEC;
}


/**
 * Get value for a statistic interpreted as time
 */
inline double Stats::get_time(int id) {
    assert( id >= 0 && id < (int)value.size() );
    return ((double)(value[id])) / (double)CLOCKS_PER_SEC;
}


/**
 * Check and return id
 */
inline int Stats::get_id(const char* name) {
    assert( name_to_id[name].id != UNITIALIZED_STAT );
    return (name_to_id[name].id);
}


#endif /* STATS_HPP_ */
