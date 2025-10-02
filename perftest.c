/**
 * @file perftest.c
 */

/**
 * @defgroup PerfTest_I \
 *    PerfTest interface
 * @brief An abstract interface to be used to record and recall time intervals.
 * @{
 */

/** Interface only requires stdbool.h include file */
#include <stdbool.h>      // for bool return type

/** @brief Struct typedef for use as argument type */
typedef struct PerfTest_s PerfTest;

/** @} End of PerfTest_I group */

/**
 * @defgroup PerfTest_Members \
 *    PerfTest Member Functions
 * @ingroup PerfTest_I
 * @{
 */

/**
 * @brief Should free malloced data of the PerfTest instance
 * @param perfTest  PerfTest instance to be cleaned up
 */
typedef void (*PT_cleaner)(PerfTest *perfTest);

/**
 * @brief Add a new time point to the PerfTest instance.
 * @param perfTest  PerfTest instance to be extended with new time point
 * @return True for success, false for failure.  Failure caused by malloc failure.
 */
typedef bool (*PT_add_point)(PerfTest *perfTest, void *data);

/**
 * @brief Reports number of time points for allocating time array
 * @param perfTest  PerfTest instance for which the point count report is needed
 * @return number of time points in the PerfTest instance.
 */
typedef int  (*PT_points_count)(const PerfTest *perfTest);

/**
 * @brief Populates an array with collected time points.
 * @param perfTest  PerfTest instance from which points are to be
 *                  collected
 * @param buff      Array of long values to which time points will
 *                  be copied
 * @param bufflen   Number of array elements to be found in @b buff.
 * @details
 *    Assuming that @b buff and @b bufflen are valid and accurate,
 *    this function will safely copy its collection oftime points
 *    to the array of longs to which @b buff points.
 */
typedef void (*PT_get_points)(const PerfTest *perfTest, long *buff, int bufflen);

/**
 * @brief
 *    Struct definition of the PerfTest interface
 * @details
 *    Every PerfTest-derived interface must have this struct as
 *    its first member to permit generic access to these functions.
 */
struct PerfTest_s {
   PT_cleaner      cleaner;       ///< function pointer
   PT_add_point    add_point;     ///< function pointer
   PT_points_count points_count;  ///< function pointer
   PT_get_points   get_points;    ///< function pointer
};

/** @} End of PerfTest_Members */


/**
 * @defgroup MemberFunctionCallers \
 *    Convenient member function wrappers
 * @ingroup PerfTest_I
 * @brief Access to implementation functions through abstract interface
 * @{
 */
void PerfTest_cleaner(PerfTest *pt)               { (*pt->cleaner)(pt); }
bool PerfTest_add_point(PerfTest *pt, void *data) { return (*pt->add_point)(pt,data); }
int PerfTest_points_count(const PerfTest *pt)     { return (*pt->points_count)(pt); }
void PerfTest_get_points(const PerfTest *pt,
                        long *buff,
                        int bufflen)              { (*pt->get_points)(pt,buff,bufflen); }
/** @} end of MemberFunctionCallers */

/**
 * @ingroup PerfTest_I
 * @brief Setup function for use in constructors of derived implementations
 */
void PerfTest_init(PerfTest *pt,                   ///< pointer to derived class
                   PT_cleaner cleaner,             ///< implementation of class function
                   PT_add_point add_point,         ///< implementation of class function
                   PT_points_count points_count,   ///< implementation of class function
                   PT_get_points get_points)       ///< implementation of class function
{
   pt->cleaner = cleaner;
   pt->add_point = add_point;
   pt->points_count = points_count;
   pt->get_points = get_points;
}

/**
 * @defgroup PerfTestImp \
 *    Implementations of PerfTest
 * @brief Subclasses of PerfTest abstract struct
 */

/*
 * The following includes are needed for the concrete classes
 */

// Define to enable `timespec` in time.h
#define _POSIX_C_SOURCE 200809L
#include <time.h>         // time() (PT_Time) and clock_gettime()/timespec (PT_Gettime)
#include <stdlib.h>       // malloc()/free() for building time point chains
#include <string.h>       // memset() for initializing instances

/**
 * @defgroup PerfTest_Time \
 *    Timer implementation using the low-precision time() function.
 * @ingroup PerfTestImp
 * @brief Use medium-precision timer function `time` for recording times.
 * @{
 */

/** @brief Struct typedef for use as argument type */
typedef struct PT_Time_s PT_Time;
/** Typedef to element of linked list of instance records. */
typedef struct PT_Time_Link_s PT_TLink;

/**
 * @brief Time-point link for PT_Time implementation of PerfTime
 */
struct PT_Time_Link_s {
   struct PT_Time_Link_s *next;   ///< pointer to next link in chain
   time_t time_point;             ///< record of instance in time
};

/**
 * @brief Subclass declaration of abstract PerfTime
 */
struct PT_Time_s {
   PerfTest base;           ///< abstract base struct
   int      link_count;     ///< number of time links in the chain
   PT_TLink *base_link;     ///< pointer to start of chain
   PT_TLink *last_link;     ///< pointer to end of chain (for efficient additions)
};

/** @brief Implementation of PT_cleaner */
void PT_Time_clean(PerfTest *perfTest)
{
   PT_Time *this = (PT_Time*)perfTest;

   PT_TLink *ptr = this->base_link;
   while (ptr)
   {
      PT_TLink *saved = ptr->next;
      free((void*)ptr);
      ptr = saved;
   }
}

/** @brief Implementation of PT_add_point */
bool PT_Time_add_point(PerfTest *perfTest, void *data)
{
   PT_Time *this = (PT_Time*)perfTest;

   PT_TLink *link = (PT_TLink*)malloc(sizeof(PT_TLink));
   if (link)
   {
      // Get time ASAP
      time(&link->time_point);
      link->next = NULL;

      // Add link to chain
      if (this->last_link == NULL)
         this->base_link = this->last_link = link;
      else
      {
         this->last_link->next = link;
         this->last_link = link;
      }

      ++this->link_count;

      return true;
   }

   return false;
}

/** @brief Implementation of PT_points_count */
int PT_Time_points_count(const PerfTest *perfTest)
{
   const PT_Time *this = (PT_Time*)perfTest;
   return this->link_count;
}

/** @brief Implementation of PT_get_points */
void PT_Time_get_points(const PerfTest *perfTest, long *buff, int bufflen)
{
   const PT_Time *this = (PT_Time*)perfTest;

   long *lptr = buff;
   long *lend = lptr + bufflen;

   PT_TLink *gptr = this->base_link;

   long basis_time = gptr->time_point;

   while (lptr < lend && gptr != NULL)
   {
      *lptr =  gptr->time_point - basis_time;

      ++lptr;
      gptr = gptr->next;
   }
}

/**
 * @brief "Constructor" of PT_Time subclass
 * @param pt_time   Uninitialized PT_Time instance
 */
void PT_Time_init(PT_Time *pt_time)
{
   memset(pt_time, 0, sizeof(PT_Time));
   PerfTest *this = (PerfTest*)pt_time;
   PerfTest_init(this,
                 PT_Time_clean,
                 PT_Time_add_point,
                 PT_Time_points_count,
                 PT_Time_get_points);
}

/** @} end of PerfTest_Time */

/**
 * @defgroup PerfTest_Gettime \
 *    Timer implementation using the high-precision clock_gettime() f * @ingroup PerfTestImp
unction.
 * @brief
 *    Using clock_gettime to get time intervals in nano-seconds
 *    (1 billionth of a second).
 * @{
 */

/** Macro to avoid mistyping the number named one-billion */
#define BILL 1000000000

/** @brief Typedef for use as argument type */
typedef struct PT_Gettime_s PT_Gettime;
/** Typedef to element of linked list of instance records. */
typedef struct PT_Gettime_Link_s PT_GTLink;

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
   PerfTest base;           ///< abstract base struct
   int       points_count;  ///< number of time links in the chain
   PT_GTLink *base_link;    ///< pointer to start of chain
   PT_GTLink *last_link;    ///< pointer to end of chain (for efficient additions)
};

/** @brief Implementation of PT_cleaner */
void PT_Gettime_clean(PerfTest *perfTest)
{
   PT_Gettime *this = (PT_Gettime*)perfTest;

   PT_GTLink *ptr = this->base_link;
   while (ptr)
   {
      PT_GTLink *saved = ptr->next;
      free((void*)ptr);
      ptr = saved;
   }
}

/** @brief Implementation of PT_add_point */
bool PT_Gettime_add_point(PerfTest *perfTest, void *data)
{
   PT_Gettime *this = (PT_Gettime*)perfTest;

   PT_GTLink *link = (PT_GTLink*)malloc(sizeof(PT_GTLink));
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

/** @brief Implementation of PT_points_count */
int PT_Gettime_points_count(const PerfTest *perfTest)
{
   const PT_Gettime *this = (PT_Gettime*)perfTest;
   return this->points_count;
}

/** @brief Implementation of PT_get_points */
void PT_Gettime_get_points(const PerfTest *perfTest, long *buff, int bufflen)
{
   const PT_Gettime *this = (PT_Gettime*)perfTest;

   long *lptr = buff;
   long *lend = lptr + bufflen;
   PT_GTLink *gptr = this->base_link;

   long basis_time = gptr->time_point.tv_sec * BILL + gptr->time_point.tv_nsec;

   while (lptr < lend && gptr != NULL)
   {
      *lptr =  gptr->time_point.tv_sec * BILL + gptr->time_point.tv_nsec;
      *lptr -= basis_time;

      ++lptr;
      gptr = gptr->next;
   }
}

/**
 * @brief "Constructor" of PT_Gettime subclass
 * @param pt_time   Uninitialized PT_Gettime instance
 */
void PT_Gettime_init(PT_Gettime *pt_time)
{
   memset(pt_time, 0, sizeof(PT_Gettime));
   PerfTest *this = (PerfTest*)pt_time;
   PerfTest_init(this,
                 PT_Gettime_clean,
                 PT_Gettime_add_point,
                 PT_Gettime_points_count,
                 PT_Gettime_get_points);
}


/** @} End of PerfTest_Gettime */


#ifdef PERFTEST_MAIN


/*
 * The following includes are needed for reporting the results
 */
#include <unistd.h>       // STDOUT_FILENO
#include <limits.h>       // LONG_MAX value
#include <stdio.h>        // printf

/**
 * @brief
 *    Generic report printer that process an array of long time values.
 * @details
 *    Both @ref PerfTest_Time and @ref PerfTest_Gettime time recording
 *    "classes" can prepare a list of long values representing points
 *    in time, though with different accuracies.  This function allows
 *    for a common output format between them.
 */
void Generic_PrintReport(int fd,              ///< file handle to which to print output
                         long *time_points,   ///< array of long time values
                         int points_count     ///< number of elements in @b time_points
   )
{
   long *ptr = time_points;
   long *end = ptr + points_count;

   long starting_time = *ptr;
   long last_value = *ptr;

   while (ptr < end)
   {
      printf("%10ld %10ld\n",
             *ptr - last_value,
             *ptr - starting_time);

      last_value = *ptr;
      ++ptr;
   }
}

void Numbered_PrintReport(int fd,
                         long *time_points,
                          int points_count)
{
   long *ptr = time_points;
   long *end = ptr + points_count;

   long starting_time = *ptr;
   long last_value = *ptr;

   int count = 0;

   while (ptr < end)
   {
      printf("%3d %10ld %10ld\n",
             ++count,
             *ptr - last_value,
             *ptr - starting_time);

      last_value = *ptr;
      ++ptr;
   }
}


/**
 * @brief Run timer test with PerfTest abstraction.
 * @details
 *    This function uses the abstraction so the identical
 *    test can be run with any or all implementation of
 *    PerfTest subclasses.
 *
 * @param pt                Instance of PerfTest subclass
 * @param factor            factorial number to calculate.  Use this
 *                          to set the number of iterations in a
 *                          test.
 * @param units_per_second  to help format PT_Gettime output, which
 *                          tracks billionths of seconds
 */
void run_timer_test(PerfTest *pt, int factor)
{

   double value = 1.0;
   PerfTest_add_point(pt, NULL);
   for (int i=1; i<factor; ++i)
   {
      value *= (double)i;
      PerfTest_add_point(pt, NULL);
   }

   printf("%d factorial is %f.\n", factor, value);

   // The final measurement is of the printf statement.
   PerfTest_add_point(pt, NULL);

   int points_count = PerfTest_points_count(pt);
   if (points_count > 0)
   {
      long *points = (long*)malloc(points_count * sizeof(long));
      if (points)
      {
         PerfTest_get_points(pt, points, points_count);
         // Generic_PrintReport(STDOUT_FILENO, points, points_count);
         Numbered_PrintReport(STDOUT_FILENO, points, points_count);
         free(points);
      }
   }

   PerfTest_cleaner(pt);
}


int main(int argc, const char **argv)
{
   int factor = 10;
   if (argc>1)
   {
      char *endptr;
      int newfactor = (int)strtol(argv[1], &endptr, 10);
      if (endptr > argv[1])
         factor = newfactor;
      else
      {
         printf("perftest optionally takes an integer argument to specify\n");
         printf("the number of iterations to run the test.  The default\n");
         printf("is 10.\n\n");
         return 1;
      }
   }

   PT_Time pt_time;
   PT_Time_init(&pt_time);
   run_timer_test((PerfTest*)&pt_time, factor);

   PT_Gettime gt_time;
   PT_Gettime_init(&gt_time);
   run_timer_test((PerfTest*)&gt_time, factor);

   return 0;
}

#endif // PERFTEST_MAIN

/**
 * @file perftest.c
 * @brief
 *    A generic performance testing interface and implementation
 * @details
 *    This project is an exploration of C-based object oriented
 *    programming.
 *
 *    This source file includes a definition of an abstract
 *    interface in the form of a struct with a set of function
 *    pointer types (PerfTest), two concrete implementations of
 *    this interface, PT_Time and PT_Gettime, and an example of
 *    using both implementations on common data.
 * @note
 *    To build a PerfTest demonstration:  
 *    `gcc -std=c99 -o perftest perftest.c`
 *
 *    In Emacs, use `M-x compile` to compile with warnings,
 *    debugging, and memory-checking enabled.  Refer to the
 *    "compile" command in "Local Variables" section below to
 *    see the additional build flags.
 *
 * @section PerfTest_Discussion Discussion
 *
 * This C-patterns reflection was inspired by a desire to confirm
 * the effectiveness of some new code.  I wanted to collect timings
 * over a multitude of events, but I didn't want to commit to a
 * specific strategy for the collection.
 *
 * I conceived of a simple interface that could sit innocuously in
 * the code with the execution abstracted away.
 */

/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -DPERFTEST_MAIN               \*/
/*   -fsanitize=address            \*/
/*   -o perftest perftest.c"        */
/* End:                             */
 
