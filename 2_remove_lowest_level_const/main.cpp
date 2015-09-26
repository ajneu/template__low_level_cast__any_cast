#include <iostream>
#include <cassert>
#include <type_traits>

template<typename T, typename Enable = void>
struct remove_lowest_const
{
  using type = typename std::remove_const<T>::type;
};

template<typename T>
struct remove_lowest_const<T, typename std::enable_if<std::is_pointer<T>::value &&
                                                      std::is_const<  T>::value    >::type>
{
  using type = typename std::add_const<
               typename std::add_pointer<
               typename remove_lowest_const<    typename std::remove_pointer<T>::type   >::type  >::type  >::type;
};

template<typename T>
struct remove_lowest_const<T, typename std::enable_if<std::is_pointer<T>::value &&
                                                    ! std::is_const<  T>::value    >::type>
{
  using type = typename std::add_pointer<
               typename remove_lowest_const<   typename std::remove_pointer<T>::type   >::type  >::type;
};

int main()
{
  assert((std::is_same<remove_lowest_const<char const        >::type, char        >::value));
  assert((std::is_same<remove_lowest_const<char              >::type, char        >::value));
  assert((std::is_same<remove_lowest_const<char      *       >::type, char*       >::value));
  assert((std::is_same<remove_lowest_const<char const*       >::type, char*       >::value));
  assert((std::is_same<remove_lowest_const<char const*      *>::type, char*      *>::value));
  assert((std::is_same<remove_lowest_const<char const* const*>::type, char* const*>::value));
  return 0;
}
