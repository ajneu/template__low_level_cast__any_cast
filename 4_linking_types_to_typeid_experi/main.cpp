#include <iostream>
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

using ptr_type_a = void(*)();
using ptr_type_b = const ptr_type_a(*)(const ptr_type_a);


static_assert(std::is_same<const ptr_type_a, void(* const)()>::value, "const ptr_type_a needs to be same as void(* const)()");


template <typename T>
struct A {
public:
  static constexpr void a()
  {}
};


struct B {
public:
  template <typename T2>
  static constexpr const ptr_type_a b(const ptr_type_a p)
  {
    return ((&A<T2>::a == p)
            ? &A<T2>::a
            : nullptr);
  }

  template <typename T2, typename T1, typename... T0>
  static constexpr const ptr_type_a b(const ptr_type_a p)
  {
    static_assert(same_type_and_t1_possib_recurse_lowlev_const<T1, T2>(), "error here");
    return ((&A<T1>::a == p)
            ? &A<T2>::a
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
    ptr_a = &A<T>::a;
    ptr_b = &B::b<T, TLL...>;
  }
  
  const ptr_type_a operator&() const {
    return ptr_a;
  }

  template <typename Tother>
  const ptr_type_a main_type_from_linked() const {
    return ptr_b(&A<Tother>::a);
  }

  bool operator==(const mytype_info &rhs) const {
    return (ptr_a == rhs.ptr_a);
  }
  bool operator!=(const mytype_info &rhs) const {
    return !(ptr_a == rhs.ptr_a);
  }
private:
  //const ptr_type_a ptr_a;
  ptr_type_a ptr_a;

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
