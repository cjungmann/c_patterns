/** @file perftest_demo.c */

// define macros to enable parts of perftest.c
#define PT_INCLUDE_IMPLEMENTATIONS
#define PT_INCLUDE_RESULTS_REPORT
#include "perftest.c"

#include <math.h>
#include <stdio.h>

/**
 * @brief No-frills Usage of PerfTest Interface
 * @details
 *    Basic use involves these steps:
 *    1. Declare and initialize a PerfTest implementation.
 *    2. Declare a PerfTest interface pointer and assign to it the
 *       address of the instance of the implementation.
 *    3. Use the interface to add points, one just before the loop,
 *       and then add points after each iteration.
 *    4. Use the data (print a report)
 *    5. Call `destructor` to free memory
 *
 *    The basic version allocates memory for each time point.
 *
 *    The other builtin versions differ mainly in how they
 *    manage memory, attempting to reduce execution overhead
 *    for more consistent and accurate timing.
 *
 * @param interations   number tasks to time
 */
void demo_simple_timing(int iterations)
{
   printf("\n\033[33;1mSimple PerfTest Demo\033[39;22m\n");

   PT_Gettime ptgt;
   PT_Gettime_init(&ptgt);

   PerfTest *pt = (PerfTest*)&ptgt;

   // Need a starting point to measure length-of-time:
   PT_add_point(pt, NULL);
   for (int i=0; i<iterations; ++i)
   {
      sqrt((double)i);
      PT_add_point(pt, NULL);
   }

   pt_test_report(pt);

   PT_clean(pt);
}

/**
 * @brief Demonstration of initializer-allocated memory pool implementation
 * @details
 *    This demonstration uses a PerfTest implementation that
 *    pre-allocates a memory pool from which slices will be used to
 *    save the time points.
 *
 * @param interations   number tasks to time
 */
void demo_premem_timing(int iterations)
{
   printf("\n\033[33;1mPooled Memory PerfTest Demo\033[39;22m\n");

   PT_Gettime_premem ptgt_p;
   PT_Gettime_premem_init(&ptgt_p, iterations+1);

   PerfTest *pt = (PerfTest*)&ptgt_p;

   // Need a starting point to measure length-of-time:
   PT_add_point(pt, NULL);
   for (int i=0; i<iterations; ++i)
   {
      sqrt((double)i);
      PT_add_point(pt, NULL);
   }

   pt_test_report(pt);

   PT_clean(pt);
}

/**
 * @brief Demonstration of caller-allocated memory implementation using heap memory
 * @details
 *    This demonstration and @ref demo_caller_stack_timing use the
 *    same implementation that depends on the calling function to
 *    provide the memory needed for saving a time stamp record.
 *
 *    This particular demonstration provides time stamp records from
 *    a pool allocated from **heap** memory using `**malloc**`.  Compare the
 *    post-test report from this implementation against the reports
 *    from the other implementations
 *
 * @param interations   number tasks to time
 */
void demo_caller_heap_timing(int iterations)
{
   printf("\n\033[33;1mCaller-allocated Heap Memory Pool PerfTest Demo\033[39;22m\n");

   PT_GTLink *pool = (PT_GTLink*)malloc((iterations+1) * sizeof(PT_GTLink));
   if (pool)
   {
      PT_GTLink *pool_ptr = pool;

      PT_Gettime_extmem ptgt_p;
      PT_Gettime_extmem_init(&ptgt_p);

      PerfTest *pt = (PerfTest*)&ptgt_p;

      // Need a starting point to measure length-of-time:
      PT_add_point(pt, NULL);
      for (int i=0; i<iterations; ++i)
      {
         sqrt((double)i);
         PT_add_point(pt, pool_ptr++);
      }

      pt_test_report(pt);

      // Skip clean because we're clearing the memory
      // here in the calling function
      free(pool);
   }

}

/**
 * @brief Demonstration of caller-allocated memory implementation using stack memory
 * @details
 *    This demonstration and @ref demo_caller_heap_timing use the
 *    same implementation that depends on the calling function to
 *    provide the memory needed for saving a time stamp record.
 *
 *    This particular demonstration provides time stamp records from
 *    a pool allocated from **stack** memory using `**alloca**`.  Compare the
 *    post-test report from this implementation against the reports
 *    from the other implementations.
 *
 * @param interations   number tasks to time
 */
void demo_caller_stack_timing(int iterations)
{
   printf("\n\033[33;1mCaller-allocated Stack Memory Pool PerfTest Demo\033[39;22m\n");

   PT_GTLink *pool = (PT_GTLink*)alloca((iterations+1) * sizeof(PT_GTLink));
   if (pool)
   {
      PT_GTLink *pool_ptr = pool;

      PT_Gettime_extmem ptgt_p;
      PT_Gettime_extmem_init(&ptgt_p);

      PerfTest *pt = (PerfTest*)&ptgt_p;

      // Need a starting point to measure length-of-time:
      PT_add_point(pt, NULL);
      for (int i=0; i<iterations; ++i)
      {
         sqrt((double)i);
         PT_add_point(pt, pool_ptr++);
      }

      pt_test_report(pt);

      // Skip clean because we provided memory from a
      // pool of stack memory, which will be released
      // when the function returns.
   }

}

/**
 * @defgroup Custom_PerfTest \
 *           Functions for custom PerfTest implementation
 * @brief
 *    Use a custom PerfTest implementation to save values
 *    and square root results within a custom PerfTest Link
 *    data structure.
 * @{
 */

typedef struct PT_Gettime_with_info_link_s PT_GTInfoLink;
typedef struct PT_Gettime_with_info_s      PT_GTInfo;
typedef struct PT_Gettime_with_info_data_s PT_GTInfoData;

/** Data payload for the PerfTest linked list */
struct PT_Gettime_with_info_data_s
{
   int value;
   double root;
};

/** PerfTest data link for this custom PerfTest implementation */
struct PT_Gettime_with_info_link_s
{
   struct PT_Gettime_with_info_link_s *next;
   struct timespec       time_point;

   // Custom part of new timer link
   PT_GTInfoData  data;
};

/** Implementation based on abstract PerfTest interface */
struct PT_Gettime_with_info_s
{
   PerfTest base;
   int      points_count;
   PT_GTInfoLink *base_link;
   PT_GTInfoLink *last_link;
};

/**
 * @brief Calculate mean and standard deviation for `demo_custom_report`.
 * @details
 *    Wanted these values to identify outlier values.  This is
 *    a simpler calculation relative to the ones done in
 *    `pt_test_report` because it doesn't look for the median
 *    which requires sorting the list.
 *
 * @param [in] info     Custom PerfTest instance from which data will
 *                      be used for report
 * @param [out] mean    Address of variable to which the mean
 *                      calculation will be stored
 * @param [out] sigma   Address of variable to which the standard
 *                      deviation calculation will be stored.
 */
void demo_calc_mean_and_sigma(const PT_GTInfo *info, double *mean, double *sigma)
{
   int time_stamps = info->points_count;
   int interval_count = time_stamps - 1;

   *mean = *sigma = 0.0;

   // If there are any intervals measured
   if (interval_count > 0)
   {
      // Calculate the mean
      long time_start = GET_BILLS(info->base_link->time_point);
      long time_end = GET_BILLS(info->last_link->time_point);
      long total_time = time_end - time_start;
      *mean = (double)(total_time) / (double)interval_count;

      // Calculate the standard deviation
      const PT_GTInfoLink *prev = info->base_link;
      const PT_GTInfoLink *current = 0;
      double interval;
      double total = 0;
      double diff;
      while (prev->next)
      {
         current = prev->next;
         time_start = GET_BILLS(prev->time_point);
         time_end = GET_BILLS(current->time_point);

         interval = (double)(time_end - time_start);
         diff = interval - *mean;
         total += (diff * diff);

         prev = prev->next;
      }

      *sigma = sqrt(total / interval_count);
   }
}

/**
 * @brief Having a little fun with a custom report
 *
 * While showing how one might generate a custom report
 * from the data collected by PerfTest, I started adding some
 * color to highlight outliers and perfect squares.
 */
void demo_custom_report(const PT_GTInfo *pt_GTI)
{
   const PT_GTInfoLink *current, *last = pt_GTI->base_link;

   double dmean, dsigma;
   demo_calc_mean_and_sigma(pt_GTI, &dmean, &dsigma);

   printf("Custom Report, mean (\033[34;1m%f\033[39;22m),"
          "and sigma (\033[34;1m%f\033[39;22m).\n", dmean, dsigma);

   long interval;
   long mean = (long)ceil(dmean);
   long sigma = (long)ceil(dsigma);

   bool outlier, perfect;

   while (last->next)
   {
      current = last->next;
      interval = GET_BILLS(current->time_point) - GET_BILLS(last->time_point);

      outlier = fabs(mean-interval) > sigma;
      perfect = fabs(round(current->data.root) - current->data.root) < 0.000001;

      if (outlier)
         printf("\033[38;2;255;64;128m");

      if (perfect)
      {
         printf("\033[48;2;64;128;64;1m");
         printf("%5d: %10ld   (%ld)", current->data.value, (long)round(current->data.root), interval);
      }
      else
         printf("%5d: %10f   (%ld)", current->data.value, current->data.root, interval);

      if (outlier || perfect)
         printf("\033[39;49;22m");

      printf("\n");

      last = current;
   }
}



/**
 * @brief Implementation of cleanup member function for PT_GTInfo
 * @param [in] pt   PerfTest implementation to be cleaned up
 */
void PT_GTInfo_cleaner(PerfTest *pt)
{
   PT_GTInfo *this = (PT_GTInfo*)pt;
   PT_GTInfoLink *del, *ptr = this->base_link;
   while (ptr)
   {
      del = ptr;
      ptr = ptr->next;
      free(del);
   }
   this->base_link = this->last_link = NULL;
}

/**
 * @brief Implementation of time point adding member function for PT_GTInfo
 * @param [in] pt     PerfTest implementation to which a new time stamp is to be added
 * @param [in] data   pointer to struct containing iteration data
 */
bool PT_GTInfo_adder(PerfTest *pt, void *data)
{
   PT_GTInfo *this = (PT_GTInfo*)pt;
   PT_GTInfoLink *link = (PT_GTInfoLink*)malloc(PT_BLOCK_SIZE(*this));
   if (link)
   {
      // Get time ASAP
      clock_gettime(CLOCK_MONOTONIC, &link->time_point);
      link->next = NULL;

      // save the data:
      link->data = *(PT_GTInfoData*)data;

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
 * @brief Initializer for PT_GTInfo
 * @param [in,out] pt   PerfTest implementation to be initialized
 */
bool PT_GTInfo_init(PT_GTInfo *pt)
{
   memset(pt, 0, sizeof(PT_Gettime));
   PerfTest_init((PerfTest*)pt,
                 PT_GTInfo_cleaner,
                 PT_GTInfo_adder,
                 PT_Gettime_counter,
                 PT_Gettime_getter);

   return true;
}

/**
 * @brief Square-root timing alternative to be called from `main`.
 *
 * @param interations   number tasks to time
 */
void demo_custom_perftest(int iterations)
{
   printf("\n\033[33;1mCustom PerfTest Demo\033[39;22m\n");

   PT_GTInfo pt_GTI;
   PT_GTInfo_init(&pt_GTI);

   PerfTest *pt = (PerfTest*)&pt_GTI;
   PT_GTInfoData data = {0};

   PT_add_point(pt, &data);
   for (int i=0; i<iterations; ++i)
   {
      data.value = i;
      data.root = sqrt((double)i);

      PT_add_point(pt,&data);
   }

   pt_test_report(pt);

   demo_custom_report(&pt_GTI);

   PT_clean(pt);
}

/** @}  End of Custom_PerfTest  */




int main(int argc, const char **argv)
{
   int iterations = 1000;
   if (argc>1)
      iterations = atoi(argv[1]);

   demo_simple_timing(iterations);

   printf("Press ENTER for the next test.\n");
   getchar();

   demo_premem_timing(iterations);

   printf("Press ENTER for the next test.\n");
   getchar();

   demo_caller_heap_timing(iterations);

   printf("Press ENTER for the next test.\n");
   getchar();

   demo_caller_stack_timing(iterations);

   printf("Press ENTER for the next test.\n");
   getchar();
   demo_custom_perftest(iterations);
}


/**
 * @page PerfTest_Demo_id PerfTest_Demo: Usage Examples for Comprehension
 *
 * This source file contains several examples of how to use PerfTest.
 *
 * - @ref PerfTest_Demo_Details.
 * - @ref PerfTest_Demo_Compilation
 */


/**
 * @page PerfTest_Demo_Compilation Learn How to Compile the Code
 *
 * @details
 *    There are two methods for building an executable from this
 *    source file:
 *    - In Emacs:  
 *      Run the `compile` command (M-x compile)
 *    - From the command line (without enabled warnings):  
 *      Type:
 *      ```sh
 *      gcc -std=c99 -o perftest_demo perftest_demo.c -lm
 *      ```
 */


/**
 * @page PerfTest_Demo_Details Detailed Description of PerfTest_Demo
 *
 * @details
 *    This source file contains demonstrations of five different
 *    implementations of the PerfTest interface.
 *
 *    With very minor variations in execution, each of the
 *    demonstrations
 *    1. declares and initializes an implementation
 *    2. takes the address of the instance to use with the interface
 *    3. executes the prescribed number of iterations of calling
 *       `sqrt` and using the interface to record the time of
 *       completion.
 *    4. prints a summary report of times to execute, including
 *       the range, mean, median, standard deviation
 *    5. by various methods, according to the needs of the specific
 *       implementation, frees up any allocated memory.
 *
 *    The first four demonstrations use builtin implementations of
 *    the PerfTest interface.  There are slight differences in setup
 *    and usage, especially around memory allocation.
 *
 *    - @ref demo_simple_timing
 *    - @ref demo_premem_timing
 *    - @ref demo_caller_stack_timing
 *    - @ref demo_caller_heap_timing
 *
 *
 *    The fifth PerfTest demonstration features a custom
 *    implementation that extends the base time-stamp data structure
 *    and overrides the clean-up and time-stamp member functions.
 *    The custom interface consists of several functions that are
 *    contained in a Doxygen group named `Custom_PerfTest`.  The
 *    custom interface is use in the following demonstration function:
 *
 *    - @ref demo_custom_perftest
 *
 *
 *    The `Custom_PerfTest` demonstration includes an additional
 *    report that manifests a custom method of extracting and using
 *    the collected data.  After the demonstration shows the builtin
 *    basic summary report, it calls the custom report to print info
 *    on each of the time stamps, highlighting perfect squares and
 *    outlier durations.  This additional report is generated with
 *    the help of:
 *
 *     - @ref demo_custom_report
 *     - @ref demo_calc_mean_and_sigma
 */


/* Local Variables:                 */
/* compile-command: "gcc           \*/
/*   -std=c99 -Wall -Werror -ggdb  \*/
/*   -fsanitize=address            \*/
/*   -lm                           \*/
/*   -o perftest_demo              \*/
/*   perftest_demo.c"               */
/* End:                             */
