#include <stdbool.h>


typedef struct PerfTest_I PerfTest;

struct PerfTest_I {
   void (*clean)(PerfTest *pt);
   bool (*add_point)(PerfTest *pt, void *data);
   int  (*points_count)(const PerfTest *pt);
   void (*get_points)(const PerfTest *pt, long *buffer, int bufferlen);
};

/**
 * @defgroup PerfTest_Wrappers \
 *           Member Function Wrappers
 * @{ */
void PT_clean(PerfTest *pt)                 { (*pt->clean)(pt); }
bool PT_add_point(PerfTest *pt, void* data) { return (*pt->add_point)(pt, data); }
int PT_points_count(const PerfTest *pt)     { return (*pt->points_count)(pt); }
void PT_get_points(const PerfTest *pt,
                   long *buffer,
                   int bufflen)               { (*pt->get_points)(pt,buffer,bufflen); }

/** @brief Macro to extract block size for allocating memory */
#define PT_BLOCK_SIZE(PT)    sizeof(*(PT).base_link)
/** @} */

typedef void (*PT_clean_t)(PerfTest *pt);
typedef bool (*PT_add_point_t)(PerfTest *pt, void *data);
typedef int (*PT_points_count_t)(const PerfTest *pt);
typedef void (*PT_get_points_t)(const PerfTest *pt, long *buffer, int bufflen);

void PerfTest_init(PerfTest *pt,
                   PT_clean_t cleaner,
                   PT_add_point_t point_adder,
                   PT_points_count_t points_counter,
                   PT_get_points_t points_getter)
{
   pt->clean = cleaner;
   pt->add_point = point_adder;
   pt->points_count = points_counter;
   pt->get_points = points_getter;
}

#ifdef PT_INCLUDE_ALL
#define PT_INCLUDE_TESTS
#endif

#ifdef PT_INCLUDE_TESTS
#define PT_INCLUDE_GENERIC_TEST_REPORT
#define PT_INCLUDE_IMPLEMENTATIONS
#endif

/**
 * @defgroup PerfTest_Domain \
 *           PerfTest Domain
 * @brief Umbrella topic that contains all things pertaining to PerfTest
 * @details
 *    PerfTest collects time-stamps with a minimum of overhead so
 *    the measurements can more accurately represent the time taken
 *    to perform tasks.
 *
 *    The PerfTest_I interface is a collection of methods that
 *    perform the basic utility of PerfTest.  The member methods are
 *    function pointers for which an implementation will provide the
 *    appropriate actual functions.
 *
 *    There is a set of @ref PerfTest_Wrappers that conceal the
 *    complicated syntax need to access the member functions.
 *
 *    There is also a set of @ref PerfTest_Typedefs that help the
 *    compiler check function types to confirm proper initializations
 *    of implementations.
 */

/**
 * @typedef PerfTest
 * @ingroup PerfTest_Domain
 * @brief Typedef to to access PerfTest_I as a single word type
 * @details
 *    It's easier to type and read as a "class" especially
 *    when you start adding *const* or other decorations.
 */

/**************************************
 * PerfTest_I interface documentation *
 *************************************/
/**
 * @interface PerfTest_I
 * @brief The interface through which an application can access
 *        performance timing functions.
 * @ingroup PerfTest_Domain
 *
 * @fn PerfTest::clean
 * @brief  Clean up resources used by the PerfTest instance.
 * @param pt   PerfTest instance to be deactivated
 * @details
 *    This is an implementation-specific function that releases
 *    resources and otherwise cleans up when the PerfTest object
 *    is no longer needed.
 *
 * @fn PerfTest::add_point
 * @brief Add time-stamp record to the PerfTest.
 * @param pt    PerfTest instance to whom a new time-stamp record should be added
 * @param data  Optional pointer (set to NULL if not needed) that
 *              can point to additional data needed by the
 *              implementation.
 * @return True for success, false for failure.  Failure to
 *         allocate memory is probably the most obvious failure,
 *         but implementations can interpret this as they see fit.
 *
 * @fn PerfTest::points_count
 * @brief Get number array elements needed for all time-stamp points.
 * @param pt    PerfTest instance from whom a points list will be requested.
 * @return The number of time-stamp values recorded by the PerfTest
 *
 * @fn PerfTest::get_points
 * @brief Fill array with time-stamp points.
 * @param pt   PerfTest instance to be deactivated
 * @param buffer
 *    address to an array of @p long pointers
 * @param bufferlen
 *    number of @p long elements in @b buffer argument
 */


/*************************************************
 * Typedefs of PerfTest member function pointers *
 ************************************************/

/**
 * @defgroup PerfTest_Typedefs \
 *           Member Function Typedefs
 * @brief Typedefs to simplify defining function argument types, ie PerfTest_inif
 * @ingroup PerfTest_Domain
 * @{
 *
 * @typedef PT_clean_t
 * @brief Typedef mirror to PerfTest::clean
 *
 * @typedef PT_add_point_t
 * @brief Typedef mirror to PerfTest::add_point
 *
 * @typedef PT_points_count_t
 * @brief Typedef mirror to PerfTest::points_count
 *
 * @typedef PT_get_points_t
 * @brief Typedef mirror to PerfTest::get_points
 *
 * @}
 */

/*****************************************
 * Wrappers of PerfTest member functions *
 *****************************************/

/**
 * @addtogroup PerfTest_Wrappers
 * @ingroup PerfTest_Domain
 * @brief Wrapper functions to dereference calls to struct member pointers to functions
 * @{
 *
 * @fn void PT_clean(PerfTest *pt)
 * @brief Wrapper function for PerfTest::clean
 * @param pt
 *
 * @fn bool PT_add_point(PerfTest *pt, void *data)
 * @brief Wrapper function for PerfTest::add_point
 * @param pt
 * @param data
 *
 * @fn int PT_points_count(const PerfTest *pt)
 * @brief Wrapper function for PerfTest::points_count
 * @param pt
 *
 * @fn void PT_get_points(const PerfTest *pt, long *buffer, int bufflen)
 * @brief Wrapper function for PerfTest::get_points
 * @param pt
 * @param buffer
 * @param bufflen
 *
 * @}
 */


/**
 * @fn void PerfTest_init(PerfTest*,
 *                        PT_clean_t,
 *                        PT_add_point_t,
 *                        PT_points_count_t,
 *                        PT_get_points_t)
 * @brief 'Constructor' for base interface
 * @ingroup PerfTest_Domain
 * @param pt
 *        PerfTest derivative to be initialized
 * @param cleaner
 *        name of function that performs PerfTest::clean service
 * @param point_adder
 *        name of function that performs PerfTest::add_point service
 * @param points_counter
 *        name of function that performs PerfTest::points_counter service
 * @param points_getter
 *        name of function that performs PerfTest::get_points service
 */





#ifdef PT_INCLUDE_IMPLEMENTATIONS

// Define to enable `timespec` in time.h
#define _POSIX_C_SOURCE 200809L
#include <time.h>         // time() (PT_Time) and clock_gettime()/timespec (PT_Gettime)
#include <stdlib.h>       // malloc()/free() for building time point chains
                          // and qsort for finding the median
#include <string.h>       // memset() for initializing instances

/** Macro to avoid mistyping the 1 with nine 0s */
#define BILL 1000000000
#define GET_BILLS(TS) ((TS).tv_sec * BILL + (TS).tv_nsec)

/**
 * @defgroup PerfTest_Impl \
 *           Implementations of %PerfTest
 * @ingroup PerfTest_Domain
 * @brief
 *    Examples of PerfTest implementations for use or to serve as programming models
 * @details
 *    There are X implementations:
 *    1.  PT_Gettime  
 *    2.  PT_Gettime_extmem  
 *    3.  PT_Gettime_premem  
 *    4.  PT_Gettime_premem_caller
 */

/**
 * @defgroup PT_Gettime_Impl \
 *           Basic %PerfTest Implementation
 * @ingroup PerfTest_Impl
 * @brief
 *    Definitive implementation of the PerfTest interface
 * @details
 *    This implementation allocates memory as needed and ignores the
 *    @b data argument of the PerfTest_I::add_point member function.
 * @{
 */

/** @brief Typedef of PT_Gettime_Link_s */
typedef struct PT_Gettime_Link_s PT_GTLink;
/** @brief Typedef of PT_Gettime_s */
typedef struct PT_Gettime_s PT_Gettime;


/**
 * @brief Time-point link for PT_Gettime implementation of PerfTime
 */
struct PT_Gettime_Link_s {
   struct PT_Gettime_Link_s *next;   ///< pointer to next link in chain
   struct timespec time_point;       ///< record of instance in time
};

/**
 * @brief Subclass declaration of abstract PerfTime
 */
struct PT_Gettime_s {
   PerfTest  base;          ///< abstract base struct
   int       points_count;  ///< number of time links in the chain
   PT_GTLink *base_link;    ///< pointer to start of chain
   PT_GTLink *last_link;    ///< pointer to end of chain (for efficient additions)
};

/** @brief Implementation of PerfTime::clean */
void PT_Gettime_cleaner(PerfTest *pt)
{
   PT_Gettime *this = (PT_Gettime*)pt;
   PT_GTLink *del, *ptr = this->base_link;
   while (ptr)
   {
      del = ptr;
      ptr = ptr->next;
      free(del);
   }
   this->base_link = this->last_link = NULL;
}

/** @brief Implementation of PerfTime::add_point */
bool PT_Gettime_adder(PerfTest *pt, void *data)
{
   PT_Gettime *this = (PT_Gettime*)pt;
   PT_GTLink *link = (PT_GTLink*)malloc(  PT_BLOCK_SIZE(*this));
   if (link)
   {
      // Get time ASAP
      clock_gettime(CLOCK_MONOTONIC, &link->time_point);
      link->next = NULL;

      // Add link to chain
      if (this->last_link == NULL)
         this->base_link = this->last_link = link;
      else
      {
         this->last_link->next = link;
         this->last_link = link;
      }

      ++this->points_count;

      return true;
   }

   return false;
}

/** @brief Implementation of PerfTime::points_count */
int PT_Gettime_counter(const PerfTest *pt)
{
   PT_Gettime *this = (PT_Gettime*)pt;
   return this->points_count;
}

/** @brief Implementation of PerfTime::get_points */
void PT_Gettime_getter(const PerfTest *pt, long *buff, int bufflen)
{
   PT_Gettime *this = (PT_Gettime*)pt;
   long *lptr = buff;
   long *lend = lptr + bufflen;
   PT_GTLink *gptr = this->base_link;

   long basis_time = GET_BILLS(gptr->time_point);

   while (lptr < lend && gptr != NULL)
   {
      *lptr = GET_BILLS(gptr->time_point);
      *lptr -= basis_time;

      ++lptr;
      gptr = gptr->next;
   }
}

/**
 * @brief Initialize a PT_Gettime instance
 * @param pt  PT_Gettime instance to be initialized
 */
bool PT_Gettime_init(PT_Gettime *pt)
{
   memset(pt, 0, sizeof(PT_Gettime));
   PerfTest_init((PerfTest*)pt,
                 PT_Gettime_cleaner,
                 PT_Gettime_adder,
                 PT_Gettime_counter,
                 PT_Gettime_getter);

   return true;
}



/** @} end of PT_Gettime_Impl */


/**
 * @defgroup PT_Gettime_ExtMem \
             External Memory version of PT_Gettime
 * @ingroup PerfTest_Impl
 * @brief
 *    Implementation of %PerfTest that depends of memory being
 *    supplied externally
 * @details
 *    This implementation uses memory provided to the @b data argument
 *    with each call to PerfTest_I::add_point to track the time-stamps.
 *
 *    It provides a unique implementations of PerfTest_I::add_point
 *    to use caller-provided memory for the PT_GTLink elements.
 *    We continue to supply the standard PerfTest_I::clean function
 *    that deletes all the PT_GTLink elements.  The clean function
 *    should be used if separate `malloc` calls supply the memory for
 *    add_point.  Other memory-providing methods will likely need
 *    unique memory releasing code.  It is the responsibility of the
 *    caller to determine to appropriate clean-up.
 *
 *    This implementation is less safe than the standard implementation
 *    because improper memory management can cause undefined behavour,
 *    but it permits a variety of memory management techniques that may
 *    impact performance and accuracy of the time record.
 * @{
 */

/** @brief Typedef an alias for PT_Gettime_s */
typedef struct PT_Gettime_s PT_Gettime_extmem;

/**
 * @brief Override PT_Gettime_adder
 * @param pt    PT_Gettime instance to be initialized
 * @param data  pointer to PT_GTLink-sized memory block
 */
bool PT_Gettime_extmem_adder(PerfTest *pt, void *data)
{
   PT_Gettime_extmem *this = (PT_Gettime_extmem*)pt;
   if (data)
   {
      PT_GTLink *link = (PT_GTLink*)data;
      // Get time ASAP
      clock_gettime(CLOCK_MONOTONIC, &link->time_point);
      link->next = NULL;

      // Add link to chain
      if (this->last_link == NULL)
         this->base_link = this->last_link = link;
      else
      {
         this->last_link->next = link;
         this->last_link = link;
      }

      ++this->points_count;

      return true;
   }

   return false;
}

/**
 * @brief Initialize a PT_Gettime_extmem instance
 * @param pt  PT_Gettime_extmem instance to be initialized
 */
bool PT_Gettime_extmem_init(PT_Gettime *pt)
{
   memset(pt, 0, sizeof(PT_Gettime_extmem));
   PerfTest_init((PerfTest*)pt,
                 PT_Gettime_cleaner,
                 PT_Gettime_extmem_adder,
                 PT_Gettime_counter,
                 PT_Gettime_getter);
   return true;
}


/** @} end of PT_Gettime_extmem */

/**
 * @defgroup PT_Gettime_premem \
             Gettime Variation
 * @ingroup PerfTest_Impl
 * @brief Gettime variation that pre-allocates memory and distributes it as needed.
 * @{
 */

typedef struct PT_Gettime_premem_s PT_Gettime_premem;

struct PT_Gettime_premem_s {
   PT_Gettime base;
   // Memory pool management
   PT_GTLink *pool;
   PT_GTLink *pool_next;
   PT_GTLink *pool_end;
};

/**
 * @brief implementation of PerfTest::clean
 * @details
 *    This cleaner is necessary to clean up the malloced memory
 *    pool.
 */
void PT_Gettime_premem_cleaner(PerfTest *pt)
{
   PT_Gettime_premem *this = (PT_Gettime_premem*)pt;
   if (this->pool)
   {
      free(this->pool);
      this->pool = this->pool_next = this->pool_end = NULL;
   }
   this->base.base_link = this->base.last_link = NULL;
}

/**
 * @brief implementation of PerfTest::add_point
 * @details
 *    This is a complete implementation, without resorting to
 *    reusing PT_Gettime_extmem because PT_Gettime_extmem
 *    may disappear because it's kind of pointless with the
 *    creation of this PerfTest variation.
 */
bool PT_Gettime_premem_adder(PerfTest *pt, void *data)
{
   bool retval = false;

   PT_Gettime_premem *this = (PT_Gettime_premem*)pt;
#ifdef PREMEM_SAFE
   if (this->pool_next)
   {
#endif
      retval = true;

      PT_GTLink *link = this->pool_next;
      clock_gettime(CLOCK_MONOTONIC, &link->time_point);

      // Install link to the base
      if (this->base.last_link == NULL)
         this->base.base_link = this->base.last_link = link;
      else
      {
         this->base.last_link->next = link;
         this->base.last_link = link;
      }

      ++this->base.points_count;

      // Prepare for next memory pull, disabling
      // the next pull if we're out of memory:
      ++this->pool_next;

#ifdef PREMEM_SAFE
      if (this->pool_next >= this->pool_end)
         this->pool_next = NULL;
   }
#endif

   return retval;
}

/**
 * @brief Initialize with anticipated record count
 * @details
 *    The signature breaks from the standard of previous PT_xx_inits
 *    in order to specify and preallocate memory.
 * @param pt    uninitialized PT_Gettime_premem param
 * @param count number of anticipated time-stamps to be saved
 * @return True if memory could be allocated, False if malloc()
 *         failed.
 */
bool PT_Gettime_premem_init(PT_Gettime_premem *pt, int count)
{
   memset(pt, 0, sizeof(PT_Gettime_premem));

   pt->pool = (PT_GTLink*)malloc(count * sizeof(PT_GTLink));
   if (pt->pool)
   {
      pt->pool_next = pt->pool;
      pt->pool_end = pt->pool + count;

      PerfTest_init((PerfTest*)pt,
                    PT_Gettime_premem_cleaner,
                    PT_Gettime_premem_adder,
                    PT_Gettime_counter,
                    PT_Gettime_getter);
      return true;
   }

   return false;
}

/** @} End of PT_Gettime_premem */

/**
 * @defgroup PT_Gettime_premem_caller
 *           Gettime variation using preallocated memory provided by
 *           its caller
 * @ingroup PerfTest_Impl
 * @brief Implementation that allows the use of a block stack memory
 * @details
 *    After many timing tests, it has become apparent that the most
 *    consistent and nearly always best performance is achieved by
 *    a PT_Gettime_extmem that allocates from a block of stack memory.
 *    Unexpectedly, the PT_Gettime_premem with an internally-allocated
 *    pool of memory was not even as fast at PT_Gettime_extmem with a
 *    pool of heap memory.
 * @{
 */

typedef struct PT_Gettime_premem_s PT_Gettime_premem_caller;


/**
 * @brief implementation of PerfTest::clean
 * @details
 *    This cleaner is necessary to clean up pointer WITHOUT
 *    freeing memory provided by the caller.
 */
void PT_Gettime_premem_caller_cleaner(PerfTest *pt)
{
   PT_Gettime_premem *this = (PT_Gettime_premem*)pt;
   if (this->pool)
   {
      // NOT freeing caller-provided pool PT_Gettime_premem_cleaner

      this->pool = this->pool_next = this->pool_end = NULL;
   }
   this->base.base_link = this->base.last_link = NULL;
}

size_t PT_Gettime_premem_caller_buffer_sizeof(int els)
{
   return sizeof(PT_GTLink) * els;
}

bool PT_Gettime_premem_caller_init(PT_Gettime_premem_caller *pt,
                                   void *buffer,
                                   int byte_len,
                                   int els_len)
{
   bool retval = false;
   if (byte_len >= sizeof(PT_GTLink) * els_len)
   {
      memset(pt, 0, sizeof(PT_Gettime_premem_caller));
      pt->pool = (PT_GTLink*)buffer;
      pt->pool_next = pt->pool;
      pt->pool_end = pt->pool + els_len;

      PerfTest_init((PerfTest*)pt,
                    PT_Gettime_premem_caller_cleaner,
                    PT_Gettime_premem_adder,
                    PT_Gettime_counter,
                    PT_Gettime_getter);
      retval = true;
   }

   return retval;
}

/** @} PT_Gettime_premem_caller */

#endif // PT_INCLUDE_IMPLEMENTATIONS

/**
 * @defgroup PerfTest_Usage \
 *           Using %PerfTest
 * @ingroup PerfTest_Domain
 * @brief
 *    A set of functions that use PerfTest to measure times using
 *    different implementations and memory-allocation methods.
 * @details
 *   Starting with the standard implementation PT_Gettime,
 */

#ifdef PT_INCLUDE_GENERIC_TEST_REPORT

#include <stdio.h>   // for printf in generic_test_report()
#include <alloca.h>  // for alloca() in extmem_stack_test()
#include <math.h>    // for sqrt() to calculate sigma/standard deviation

/**
 * @defgroup SimpleStats \
 *           Functions that calculate simple statistics values
 * @ingroup PerfTest_Usage
 * @brief Some functions used for statistics calculations.
 * @details
 *    Besides ai_qsort_comp that is used for `qsort`, these functions
 *    take an array of long time interval values, sorted in ascending
 *    order, along with the number of elements in the array.
 */

/**
 * @brief QSort comparison function sorting intervals to find median and mode
 * @ingroup SimpleStats
 */
int ai_qsort_comp(const void *left, const void *right)
{
   return *(long*)left - *(long*)right;
}

/**
 * @brief Calculate the mean/average of a set of interval values.
 * @ingroup SimpleStats
 */
double ai_calc_mean(const long *values, int count)
{
   long total = 0;
   const long *ptr = values;
   const long *end = ptr + count;

   while (ptr < end)
      total += *ptr++;

   return (double)total / (double)count;
}

/**
 * @brief Find the median of a set of interval values.  Required sorted interval array.
 * @ingroup SimpleStats
 */
double ai_calc_median(const long *values, int count)
{
   long halfsy = count / 2;
   if (count%2)
      return (double)values[halfsy];
   else
      return (double)(values[halfsy] + values[halfsy+1]) / 2;
}

/**
 * @brief Calculate sigma (or standard deviation) for a set of interval values.
 * @ingroup SimpleStats
 */
double ai_calc_sigma(const long *values, int count)
{
   double mean = ai_calc_mean(values, count);
   const long *ptr = values;
   const long *end = ptr + count;

   double total = 0;
   double diff;
   while (ptr < end)
   {
      diff = (double)*ptr - mean;
      total += (diff * diff);
      ++ptr;
   }

   return sqrt(total / count);
}

/**
 * @brief Produce a very basic report with a set of time-stamps.
 * @param times        array of long time-stamp values
 * @param times_count  number of elements in @b times.
 *
 * @ingroup PerfTest_Usage
 */
void generic_test_report(long *times, int times_count)
{
   long *ptr_times = times;
   long *end_times = ptr_times + times_count;
   long last = *ptr_times;

#ifdef SHOW_LISTS
   printf("Time stamps:\n");
   while (ptr_times < end_times)
   {
      printf("%3ld: %4ld %8ld\n", end_times - ptr_times, *ptr_times - last, *ptr_times);
      last = *ptr_times;
      ++ptr_times;
   }
#endif

   int intervals_count = times_count - 1;
   long *intervals = (long*)malloc(sizeof(long) * intervals_count);
   if (intervals)
   {
      long *ptr_intervals = intervals;

      ptr_times = times;
      last = *ptr_times++;

      while (ptr_times < end_times)
      {
         *ptr_intervals = *ptr_times - last;
         last = *ptr_times;
         ++ptr_times;
         ++ptr_intervals;
      }

#ifdef SHOW_LISTS
      long *end_intervals = ptr_intervals + intervals_count;
      ptr_intervals = intervals;
      int count = 0;
      printf("\nUnsorted time intervals:\n");
      while (ptr_intervals < end_intervals)
      {
         printf("%3d: %ld.\n", ++count, *ptr_intervals);
         ++ptr_intervals;
      }
#endif

      // Sort the intervals
      qsort(intervals, intervals_count, sizeof(long), ai_qsort_comp);

#ifdef SHOW_LISTS
      ptr_intervals = intervals;
      count = 0;
      printf("\nSorted time intervals:\n");
      while (ptr_intervals < end_intervals)
      {
         printf("%3d: %ld.\n", ++count, *ptr_intervals);
         ++ptr_intervals;
      }
#endif

      double mean =   ai_calc_mean(intervals, intervals_count);
      double median = ai_calc_median(intervals, intervals_count);
      double sigma =  ai_calc_sigma(intervals, intervals_count);

      printf("\nFor this sample of intervals:\n");
      printf("  mean                 %f.\n", mean);
      printf("  median               %f.\n", median);
      printf("  standard deviation   %f.\n", sigma);

      free(intervals);
   }
}

void pt_test_report(PerfTest *pt)
{
   int points_count = PT_points_count(pt);
   if (points_count)
   {
      long *buff = (long*)malloc(points_count * sizeof(long));
      if (buff)
      {
         PT_get_points(pt, buff, points_count);
         generic_test_report(buff, points_count);

         free(buff);
      }
   }
}

#endif // PT_INCLUDE_GENERIC_TEST_REPORT

#ifdef PT_INCLUDE_TESTS

/**
 * @brief Test of the definitive PerfTest implementation PT_Gettime_s
 * @param pt          Initialized implementation of adhoc memory version of PerfTest
 * @param iterations  number of points to add to @b pt.
 * @ingroup PerfTest_Usage
 */
void test_base(int iterations)
{
   PT_Gettime ptgt;
   PT_Gettime_init(&ptgt);

   PerfTest *pt = (PerfTest*)&ptgt;

   // Get samples
   PT_add_point(pt, NULL);
   for (int i=0; i<iterations; ++i)
      PT_add_point(pt, NULL);

   pt_test_report(pt);

   PT_clean(pt);
}

/**
 * @brief Test of PT_Gettime_extmem using new stack memory for each add_point
 * @details
 *    Allocates @b stack memory for PT_GTLink for each iteration of the
 *    `for` loop.
 * @param iterations  number of points to add to @b pt.
 * @ingroup PerfTest_Usage
 */
void test_extmem_stack(int iterations)
{
   PT_Gettime_extmem pte;
   PT_Gettime_extmem_init(&pte);

   long link_size = PT_BLOCK_SIZE(pte);

   PerfTest *pt = (PerfTest*)&pte;

   // Get samples
   PT_add_point(pt, alloca(link_size));
   for (int i=0; i<iterations; ++i)
      PT_add_point(pt, alloca(link_size));

   pt_test_report(pt);

   // Don't call clean because the stack-allocated memory
   // will be released when the function returns:
   // PT_clean(pt);
}

/**
 * @brief Test of PT_Gettime_extmem using new heap memory for each add_point
 * @details
 *    Allocates @b heap memory for PT_GTLink for each iteration of the
 *    `for` loop.
 * @param iterations  number of points to add to @b pt.
 * @ingroup PerfTest_Usage
 */
void test_extmem_heap(int iterations)
{
   PT_Gettime_extmem pte;
   PT_Gettime_extmem_init(&pte);

   long link_size = PT_BLOCK_SIZE(pte);

   PerfTest *pt = (PerfTest*)&pte;

   // Get samples
   PT_add_point(pt, malloc(link_size));
   for (int i=0; i<iterations; ++i)
      PT_add_point(pt, malloc(link_size));

   pt_test_report(pt);

   // Use the standard clean up for add_points using whose second
   // argument is an individual malloced block of memory.
   PT_clean(pt);
}

/**
 * @brief Test of PT_Gettime_extmem with part of preallocated heap memory.
 * @details
 *    Creates an instance of PT_Gettime_extmem, then allocates from the
 *    @b heap an array of PT_GTLink in one block sufficient supply
 *    PT_Gettime_extmem instance with memory for the calls to @b add_point.
 * @param iterations  number of points to add to @b pt.
 * @ingroup PerfTest_Usage
 */
void test_extmem_heap_block(int iterations)
{
   PT_Gettime_extmem pte;
   PT_Gettime_extmem_init(&pte);

   // Allocate for +1 to accommodate pre-loop time-stamp
   PT_GTLink *links = (PT_GTLink*)malloc((iterations+1) * PT_BLOCK_SIZE(pte));
   PT_GTLink *lptr = links;

   PerfTest *pt = (PerfTest*)&pte;

   // Get samples
   PT_add_point(pt, lptr++);
   for (int i=0; i<iterations; ++i)
      PT_add_point(pt, lptr++);

   pt_test_report(pt);

   // Free the malloced block of PT_GTLink elements:
   free(links);

   // Don't call "clean" because it would attempt to free
   // already-freed memory.
   // PT_clean(pt);
}

/**
 * @brief Test of PT_Gettime_extmem with part of preallocated stack memory.
 * @details
 *    Creates an instance of PT_Gettime_extmem, then allocates from the
 *    @b stack an array of PT_GTLink in one block sufficient supply
 *    PT_Gettime_extmem instance with memory for the calls to @b add_point.
 * @param iterations  number of points to add to @b pt.
 * @ingroup PerfTest_Usage
 */
 void test_extmem_stack_block(int iterations)
{
   PT_Gettime_extmem pte;
   PT_Gettime_extmem_init(&pte);

   // Allocate for +1 to accommodate pre-loop time-stamp
   PT_GTLink *links = (PT_GTLink*)alloca((iterations+1) * PT_BLOCK_SIZE(pte) );
   PT_GTLink *lptr = links;

   PerfTest *pt = (PerfTest*)&pte;

   // Get samples
   PT_add_point(pt, lptr++);
   for (int i=0; i<iterations; ++i)
      PT_add_point(pt, lptr++);

   pt_test_report(pt);

   // Don't call "clean" because it would attempt to free
   // stack-allocated memory:
   // PT_clean(pt);
}

/**
 * @brief Run test using PT_Gettime_premem
 */
void test_premem(int iterations)
{
   PT_Gettime_premem ptp;

   // Allocate for +1 to accommodate pre-loop time-stamp
   if (PT_Gettime_premem_init(&ptp, iterations+1))
   {
      PerfTest *pt = (PerfTest*)&ptp;

      // Get samples
      PT_add_point(pt, NULL);
      for (int i=0; i<iterations; ++i)
         PT_add_point(pt, NULL);

      pt_test_report(pt);

      // Must call PT_clean to free links memory:
      PT_clean(pt);
   }
}

/**
 * @brief Run test using PT_Gettime_premem_caller with heap memory
 */
void test_premem_caller_heap(int iterations)
{
   PT_Gettime_premem ptp;

   // The initial time-stamp increases the stamp_count need by one:
   int stamp_count = iterations + 1;

   size_t buff_len = PT_Gettime_premem_caller_buffer_sizeof(stamp_count);
   void *buffer = malloc(buff_len);
   if (buffer)
   {
   // Allocate for +1 to accommodate pre-loop time-stamp
      if (PT_Gettime_premem_caller_init(&ptp, buffer, buff_len, stamp_count))
      {
         PerfTest *pt = (PerfTest*)&ptp;

         // Get samples
         PT_add_point(pt, NULL);
         for (int i=0; i<iterations; ++i)
            PT_add_point(pt, NULL);

         pt_test_report(pt);

         // Technically, we don't have to call this because
         // the stack memory will be released with the function
         // return, which will also drop the PT_Gettime_premem_init
         // as it goes out of scope.
         PT_clean(pt);
      }

      free(buffer);
   }
}

/**
 * @brief Run test using PT_Gettime_premem_caller with stack memory
 */
void test_premem_caller(int iterations)
{
   PT_Gettime_premem ptp;

   // The initial time-stamp increases the stamp_count need by one:
   int stamp_count = iterations + 1;

   size_t buff_len = PT_Gettime_premem_caller_buffer_sizeof(stamp_count);
   void *buffer = alloca(buff_len);
   if (buffer)
   {
   // Allocate for +1 to accommodate pre-loop time-stamp
      if (PT_Gettime_premem_caller_init(&ptp, buffer, buff_len, stamp_count))
      {
         PerfTest *pt = (PerfTest*)&ptp;

         // Get samples
         PT_add_point(pt, NULL);
         for (int i=0; i<iterations; ++i)
            PT_add_point(pt, NULL);

         pt_test_report(pt);

         // Technically, we don't have to call this because
         // the stack memory will be released with the function
         // return, which will also drop the PT_Gettime_premem_init
         // as it goes out of scope.
         PT_clean(pt);
      }
   }
}


#endif // PT_INCLUDE_TESTS



#ifdef PERFTEST_MAIN

#include <sys/resource.h>   // for setpriority()
#include <errno.h>          // to print failure status for setpriority

void print_description(const char *implementation,
                       const char *alloc_source,      ///< stack or heap
                       const char *alloc_type,        ///< individual or from pool
                       int iterations,
                       bool pause_after)
{
   printf("Executed \033[32;1m%d\033[39;22m iterations "
          "with implementation \033[32;1m%s\033[39;22m, "
          "link memory from \033[32;1m%s\e[39;22m allocation "
          "using \033[32;1m%s\e[39;22m memory\n",
          iterations,
          implementation,
          alloc_source,
          alloc_type);

   if (pause_after)
      getchar();
}


int main(int argc, const char **argv)
{
   int iterations = 100;
   if (argc>1)
   {
      char *endptr;
      int argiter = (int)strtol(argv[1], &endptr, 10);
      if (endptr > argv[1])
         iterations = argiter;
   }


   bool pause_between = false;

   printf("\033[2J\033[H");

   if (setpriority(PRIO_PROCESS, 0, 19) == -1)
      printf("Failed to set the priority, '%s'\n", strerror(errno));
   else
      printf("Set the highest available priority.\n");

   test_base(iterations);
   print_description("PT_Gettime", "heap", "individual", iterations, pause_between);

   test_extmem_heap(iterations);
   print_description("PT_Gettime_extmem", "heap", "individual", iterations, pause_between);

   test_extmem_stack(iterations);
   print_description("PT_Gettime_extmem", "stack", "individual", iterations, pause_between);

   test_extmem_heap_block(iterations);
   print_description("PT_Gettime_extmem", "heap", "pool", iterations, pause_between);

   test_extmem_stack_block(iterations);
   print_description("PT_Gettime_extmem", "stack", "pool", iterations, pause_between);

   test_premem(iterations);
   print_description("PT_Gettime_premem", "heap", "pool", iterations, pause_between);

   test_premem_caller(iterations);
   print_description("PT_Gettime_premem_caller_heap", "heap", "pool", iterations, pause_between);

   test_premem_caller(iterations);
   print_description("PT_Gettime_premem_caller", "stack", "pool", iterations, pause_between);
   return 0;
}

#endif // PERFTEST_MAIN


/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -DPERFTEST_MAIN               \*/
/*   -DPT_INCLUDE_ALL              \*/
/*   -fsanitize=address            \*/
/*   -lm                           \*/
/*   -o perftest perftest.c"        */
/* End:                             */
