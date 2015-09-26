#include <iostream>
#include <cassert>
#include <type_traits>

template<typename T1, typename T2>
constexpr bool same_type_and_t1_possib_recurse_lowlev_const()
/*
  T2 SAME as T1 (possibly by adding low-level consts to T2)
 */
{
  return ((!(!std::is_const<T1>::value &&   std::is_const<T2>::value)) &&
          ( std::is_pointer<T1>::value == std::is_pointer<T2>::value)  &&
          ( std::is_pointer<T1>::value
            ? same_type_and_t1_possib_recurse_lowlev_const<typename std::remove_pointer<T1>::type,
                                                           typename std::remove_pointer<T2>::type>()
            : std::is_same<typename std::remove_const<T1>::type,
                           typename std::remove_const<T2>::type>::value
            ));
          
}

#define COMPARE_TYPES(T1, T2)                                                                                        \
  if (same_type_and_t1_possib_recurse_lowlev_const<T1, T2>()) {                                                      \
    std::cout << "using T1 = " #T1 ";\n <=> \nusing T2 = " #T2                                                       \
                 ";\n\t\t-> types SAME, except T1 may possibly have additional low-level consts\n\n" << std::endl;   \
  } else {                                                                                                           \
    std::cout << "using T1 = " #T1 ";\n XXX \nusing T2 = " #T2                                                       \
                 ";\n\t\t-> types DIFFER (not even adding low-level consts to T2, can make the types equal)\n\n" << std::endl; \
  }


int main()
{
  //            T1                  T2
  COMPARE_TYPES(char const*       , char const*       ); // T2 SAME as T1 (possibly by adding low-level consts to T2)
  COMPARE_TYPES(char const*       , char      *       ); // also
  COMPARE_TYPES(char const*      *, char      *      *); // also
  COMPARE_TYPES(char const* const*, char      *      *); // also

  COMPARE_TYPES(char      *       , char const*       ); // DIFFER (not even adding low-level consts to T2, can make the types equal)
  COMPARE_TYPES(char      * const*, char const*      *); // also
  COMPARE_TYPES(char const*      *, char const* const*); // also

  return 0;
}
