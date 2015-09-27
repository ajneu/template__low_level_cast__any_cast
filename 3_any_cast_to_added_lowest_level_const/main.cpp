#include <iostream>
#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <experimental/any>



// remove_lowest_const ... same as in ../2_remove_lowest_level_const/main.cpp

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





using std::experimental::any;
using std::experimental::any_cast;

template<typename T>
T myany_cast(any &ay)
{
  return ((&typeid(T) == &ay.type())
          ? any_cast<T>(ay)
          : ((&typeid(typename remove_lowest_const<T>::type) == &ay.type())
             ? (
#ifndef NDEBUG
                (std::cout << "(removing lowest-level const)" << std::endl),
#endif
                const_cast<T>(any_cast<typename remove_lowest_const<T>::type>(ay)))
             : /* fail */ any_cast<T>(ay)));
}

int main()
{
  char str[] = "asdf";

  {
    any ay{str};

    std::cout << "***try normal any_cast:" << std::endl;
    try {
      std::cout << any_cast<char const*>(ay) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    any may{str};

    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << myany_cast<char const*>(may) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    char *x = str;
    any may{&x};

    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << *myany_cast<char const* *>(may) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    char *x = str;
    any may{&x};

    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << *myany_cast<char const* const*>(may) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "this is no sliver bullet, since if we want to myany_cast to a type with multiple low-level consts, we'd have to compare various permutations (!) of const-removal with the actual typeid (curr_type)" << std::endl;
    }
  }

  return 0;
}
