/*
 * Taken from http://www.andrew.cmu.edu/user/vanhoeve/mdd/ with the notice:
 * "The software can be freely used but comes with no warranty"
 * -----------------------------------------------------------
 * Data structure to store positive sets of integers.

 * Should be used in the cases where one needs to do
 * fast intersections and unions, and does not iterate
 * on all elements too many times.
 * -----------------------------------------------------------
 */

#ifndef INTSET_HPP_
#define INTSET_HPP_

#define NOT_COMPUTED -1     /**< indicates if the size was not computed */

#include <boost/dynamic_bitset.hpp>
#include <cassert>
#include <iostream>
#include <fstream>
#include "util.hpp"


/**
 * Integer Set structure
 */
struct IntSet {

    /** Constructor */
    IntSet(int _min, int _max, bool _filled);

    /** Empty constructor */
    IntSet();

    /** Check if set contains element */
    bool contains(int elem);

    /** Add an element to the set */
    void add(int elem);

    /** Add all possible elements to the set */
    void add_all_elements();

    /** Remove element, if it is contained */
    void remove(int elem);

    /** Get number of elements in the set */
    int get_size();

    /** Get the first element of the set */
    int get_first();

    /** Get next element higher than the one passed as parameter */
    int get_next(int elem);

    /** Get end of the set (beyond last element) */
    const int get_end();

    /** Clear set */
    void clear();

    /** Resize */
    void resize(int _min, int _max, bool _filled);

    /** Take the union with another intset */
    void union_with(IntSet& intset);

    /** Take the intersection with another intset */
    void intersect_with(IntSet& intset);
    
    /** Checks if one intersects with the other intset */
    void does_intersect(IntSet& intset);
    
    /** Assignment operator */
    IntSet& operator=(const IntSet& rhs);
    
    /** Returns if set is a subset of another */
    bool is_subset(const IntSet& other);

    /** Returns if one set equals another */
    bool equals_to(const IntSet& other);


    // parameters

    boost::dynamic_bitset<>     set;            /**< bitvector representing the set */
    const int                   end;            /**< position beyond end of the set */
    int                         size;           /**< number of elements in the set */
    int                         min;            /**< minimum possible element of the set */
    int                         max;            /**< maximum possible element of the set */
    //int                         shift;          /**< shift of element to be added in the set */
};


/**
 * Lexicographic comparator function for IntSet class.
 */
struct IntSetLexLessThan {
    bool operator()(const IntSet* setA, const IntSet* setB) const {
        return setA->set < setB->set;
    }
};



/**
 * -----------------------------------------------
 * Inline implementations
 * -----------------------------------------------
 */

/**
 * Constructor
 */
inline IntSet::IntSet(int _min, int _max, bool _filled) : end((int)set.npos) {
    resize(_min, _max, _filled);
    size = NOT_COMPUTED;
}

/**
 * Empty constructor
 */
inline IntSet::IntSet() : end((int)set.npos) {
}


/**
 * Add an element to the set
 */
inline bool IntSet::contains(int elem) {
    assert( elem >= min && elem <= max );
    return( set.test(elem) );
}


/**
 * Add an element to the set
 */
inline void IntSet::add(int elem) {
    assert( elem >= min && elem <= max );
    set.set(elem, true);
    size = NOT_COMPUTED;
}

/** Remove element, if it is contained */
inline void IntSet::remove(int elem) {
    assert( elem >= min && elem <= max );
    set.set(elem, false);
    size = NOT_COMPUTED;
}

/**
 * Get the first element of the set
 */
inline int IntSet::get_first() {
    return (set.find_first());
}

/**
 * Get next element higher than the one passed as parameter
 */
inline int IntSet::get_next(int elem) {
    assert( elem >= min && elem <= max );
    return (set.find_next(elem));
}

/**
 * Get end of the set (beyond last element)
 */
inline const int IntSet::get_end() {
    return end;
}

/**
 * Clear set
 */
inline void IntSet::clear() {
    set.reset();
    size = 0;
}


/**
 * Assignment operator
 */
inline IntSet& IntSet::operator=(const IntSet& rhs) {
    assert(rhs.max == max && rhs.min == min);
    if (this != &rhs) {
        set = rhs.set;
        size = NOT_COMPUTED;
    }
    return *this;
}


/**
 * Resize
 */
inline void IntSet::resize(int _min, int _max, bool _filled) {

	if( _min != 0 ) {
		exit(1);
	}

    min = _min;
    max = _max;

    set.resize(max - min + 1);

    if( _filled )
        set.set();
    else
        set.reset();

    size = NOT_COMPUTED;
}

/**
 * Take the union with another intset
 */
inline void IntSet::union_with(IntSet& intset) {
    set |= intset.set;
    size = NOT_COMPUTED;
}

/**
 * Take the intersection with another intset
 */
inline void IntSet::intersect_with(IntSet& intset) {
    set &= intset.set;
    size = NOT_COMPUTED;
}

/** Get number of elements in the set */
inline int IntSet::get_size() {
    if( size == NOT_COMPUTED ) {
        size = set.count();
    }
    return size;
}

/**
 * Add all possible elements to the set
 */
inline void IntSet::add_all_elements() {
    set.set();
    size = set.size();
}


/**
 * Stream output function
 */
inline std::ostream& operator<<(std::ostream &os, IntSet &intset) {
    os << "[ ";
    int val = intset.get_first();
    while( val != intset.get_end() ) {
        os << val << " ";
        val = intset.get_next(val);
    }
    os << "]";
    return os;
}


/** 
 * Returns if set is a subset of another 
 */
inline bool IntSet::is_subset(const IntSet& other) {
    return set.is_subset_of(other.set);
}

/**
 * Returns if one is equal to the other
 */
inline bool IntSet::equals_to(const IntSet& other) {
	return (set == other.set);
}



#endif /* INTSET_HPP_ */

