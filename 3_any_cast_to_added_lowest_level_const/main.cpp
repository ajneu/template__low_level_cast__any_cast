#include <iostream>
#include <cassert>
#include <type_traits>
#include <typeinfo>
#include <boost/any.hpp>


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




// function that takes a type_info-ptr as arg   and   returns a type_info-ptr
using ptr_type_lookup = const std::type_info *(*)(const std::type_info *);

struct TypeLookup {
public:
  template <typename TMain>
  static constexpr const std::type_info *lookup_main_type(const std::type_info *linked_type)
  {
    return ((&typeid(TMain) == linked_type)
            ? &typeid(TMain)
            : nullptr);
  }

  template <typename TMain, typename TLink, typename... TLinkMore>
  static constexpr const std::type_info *lookup_main_type(const std::type_info *linked_type)
  {
    static_assert(same_type_and_t1_possib_recurse_lowlev_const<TLink, TMain>(), "error here");
    return ((&typeid(TLink) == linked_type)
            ? &typeid(TMain)
            : lookup_main_type<TMain, TLinkMore...>(linked_type));
  }

};


class TypeLinker {
public:
  // this should actually be in the constructor (how to do it?)
  template <typename TMain, typename... TLinkMore>
  void set_main_and_link_types() {
    lookup_main = &TypeLookup::lookup_main_type<TMain, TLinkMore...>;
  }

  template <typename TPossiblyLinked>
  const std::type_info *main_type_from_linked() const {
    return lookup_main(&typeid(TPossiblyLinked));
  }

private:
  //const ptr_type_lookup lookup_main;
  ptr_type_lookup lookup_main = unint_null;

  static constexpr const std::type_info *unint_null(const std::type_info *){ return nullptr; }
};


using boost::any;
using boost::any_cast;

class MyAny : public any {
public:
  using any::any;
  using any::operator=;

  // template <typename ValueType, typename... TLL>                   // how can one call such a templated constructor???
  // MyAny(ValueType&& value) : any(std::forward<ValueType>(value))
  // {
  //   linked_types.set_main_and_link_types<ValueType, TLL...>();
  // }

  template<typename ValueType, typename... TLL>
  MyAny& operator=(ValueType&& rhs)
  {
    static_cast<any&>(*this).operator=(std::forward<ValueType>(rhs));
    linked_types.set_main_and_link_types<ValueType, TLL...>();
    return *this;
  }
  
  using any::type;
private:
  template<typename T> friend T myany_cast(MyAny &ay);
  TypeLinker linked_types;
};

template<typename T>
T myany_cast(MyAny &ay)
{
  return ((&typeid(T) == &ay.type())
          ? *boost::unsafe_any_cast<T>(static_cast<any*>(&ay))
          : ((&typeid(typename remove_lowest_const<T>::type) == &ay.type())
             ? *boost::unsafe_any_cast<T>(static_cast<any*>(&ay))
             : ((ay.linked_types.main_type_from_linked<T>() != nullptr)
                ? *boost::unsafe_any_cast<T>(static_cast<any*>(&ay))
                : throw boost::bad_any_cast())));
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
    catch(const boost::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    MyAny may;
    may.operator=<char      *, // main type
                  char const*  // linked type (low-level const)
                  >(str);

    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << myany_cast<char const*>(may) << std::endl;
    }
    catch(const boost::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  std::cout << "\n" << std::endl;

  {
    char *x = str;
    MyAny may;
    may.operator=<char      *      *, // main type
                  char const*      *, // linked type (low-level const)
                  char const* const*  // linked type (low-level const)
                  >(&x);
    
    std::cout << "***try myany_cast:" << std::endl;
    try {
      std::cout << *myany_cast<char const*      *>(may) << std::endl;
      std::cout << *myany_cast<char const* const*>(may) << std::endl;
    }
    catch(const boost::bad_any_cast &e) {
      std::cout << "fails" << std::endl;
    }
  }

  return 0;
}
