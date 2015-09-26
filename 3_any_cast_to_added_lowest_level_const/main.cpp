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

class MyAny : public any {
public:
  MyAny() : any(), curr_type{&typeid(void)}
  {}

  MyAny(const MyAny& other) : any{other}, curr_type{other.curr_type}
  {}

  MyAny(MyAny&& other) : any{std::move(other)}, curr_type{other.curr_type}
  {}

  template <typename ValueType>
  MyAny(ValueType&& value) : any(std::forward<ValueType>(value)), curr_type{&typeid(typename std::decay<ValueType>::type)}
  {}

  MyAny& operator=(const MyAny& rhs) {
    static_cast<any&>(*this).operator=(rhs);
    curr_type = rhs.curr_type;
    return *this;
  }

  MyAny& operator=(MyAny&& rhs) {
    static_cast<any&>(*this).operator=(std::move(rhs));
    curr_type = rhs.curr_type;
    return *this;
  }

  template<typename ValueType>
  MyAny& operator=(ValueType&& rhs)
  {
    static_cast<any&>(*this).operator=(std::forward<ValueType>(rhs));
    curr_type = rhs.curr_type;
    return *this;
  }

private:
  const std::type_info *curr_type;
  template<typename T> friend T myany_cast(MyAny &ay);
};

template<typename T>
T myany_cast(MyAny &ay)
{
  return ((&typeid(T) == ay.curr_type)
          ? any_cast<T>(ay)
          : ((&typeid(typename remove_lowest_const<T>::type) == ay.curr_type)
             ? (
#ifndef NDEBUG
                (std::cout << "(removing lowest-level const)" << std::endl),
#endif
                static_cast<T>(any_cast<typename remove_lowest_const<T>::type>(ay)))
             : /* fail */ any_cast<T>(ay)));
}

int main()
{
  char str[] = "asdf";

  {
    any ay{str};

    std::cout << "***try normal any_cast:" << std::endl;
    try {
      std::cout << any_cast<const char *>(ay) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    MyAny may{str};

    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << myany_cast<const char *>(may) << std::endl;
    }
    catch(const std::experimental::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  return 0;
}
