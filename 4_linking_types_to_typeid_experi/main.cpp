#include <iostream>
#include <typeinfo>
#include <cassert>

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

using ptr_type_b = const std::type_info *(*)(const std::type_info *);

struct B {
public:
  template <typename T2>
  static constexpr const std::type_info *b(const std::type_info *p)
  {
    return ((&typeid(T2) == p)
            ? &typeid(T2)
            : nullptr);
  }

  template <typename T2, typename T1, typename... T0>
  static constexpr const std::type_info *b(const std::type_info *p)
  {
    static_assert(same_type_and_t1_possib_recurse_lowlev_const<T1, T2>(), "error here");
    return ((&typeid(T1) == p)
            ? &typeid(T2)
            : b<T2, T0...>(p));
  }

};


class mytype_info {
public:
  mytype_info() : ptr_a{nullptr}, ptr_b{nullptr}
  {}


  // this should actually be in the constructor (how to do it?)
  template <typename T, typename... TLL>
  void set_type() {
    ptr_a = &typeid(T);
    ptr_b = &B::b<T, TLL...>;
  }
  
  const std::type_info *operator&() const {
    return ptr_a;
  }

  template <typename Tother>
  const std::type_info *main_type_from_linked() const {
    return ptr_b(&typeid(Tother));
  }

  bool operator==(const mytype_info &rhs) const {
    return (ptr_a == rhs.ptr_a);
  }
  bool operator!=(const mytype_info &rhs) const {
    return !(ptr_a == rhs.ptr_a);
  }
private:
  //const std::type_info * const ptr_a;
  const std::type_info *ptr_a;

  //const ptr_type_b ptr_b;
  ptr_type_b ptr_b;
};


int main()
{
  mytype_info i1;
  i1.set_type</* improvement: should put static linking_checker mechanism here, i.e. same_type_and_t1_possib_recurse_lowlev_const */
              int      *      *      , // main type
              int      *      * const, // linked type
              int      * const*      , // linked type
              int      * const* const, // linked type
              int const*      *      , // linked type
              int const*      * const, // linked type
              int const* const*      , // linked type
              int const* const* const  // linked type
              >();

  mytype_info i2;
  i2.set_type<int * *>();  // only main type
  assert(i1 == i2);        // compare main type
  assert(i1.main_type_from_linked<int * const *>() != nullptr);


  mytype_info c1;
  c1.set_type<char>();
  assert(c1 != i1);

  
  return 0;
}
