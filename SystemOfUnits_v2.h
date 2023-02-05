#pragma once

#ifndef SOU_NO_DIM_ANALYSIS

#include <ratio>
#include <type_traits>
#include <string>
#include <sstream>

template<typename ...  Args>
struct pack
{
  template<class T>
  using prepend=pack<T, Args...>;
  template<template <typename...> typename T>
  using instantiate=T<Args...>;
};

template< template<typename, typename> typename op,
	  typename, typename>
struct apply_op;

template< template<typename, typename> typename op,
	  typename A0, typename B0,
	  typename... As, typename... Bs>
struct apply_op<op, pack<A0, As...>, pack<B0, Bs...>>
{
  using res=typename apply_op<op, pack<As...>, pack<Bs...> >::res::prepend<op<A0,B0>>;
};
template< template<typename, typename> typename op>
struct apply_op<op, pack<>, pack<>>
{
  using res=pack<>;
};

template<typename EnergyExponent, typename LengthExponent, typename TimeExponent> // and so on
class physical_quantity
{
  double val;

  template<typename rat>
  std::string mk_unit(std::string base) const
  {
    auto num=rat::num;
    auto den=rat::den;
    std::stringstream res;
    if (num==0)
      return "";
    if (den==1) // whole powers
      {
	if (num>0)
	  {
	    res<<" "<<base;
	  }
	else
	  {
	    res<<"/"<<base;
	    num=-num;
	  }
	if (num!=1)
	  res<<"^"<<num;
	return res.str();
      }
    res<<" "<<base<<"^("<<num<<"/"<<den<<")";
    return res.str();
  }
  
public:
  using unitpack=pack<EnergyExponent, LengthExponent, TimeExponent >;
  inline operator double() //= delete;
  //double blubb()
    {
      // dimensionless values are convertable to double
      // Manually iterate over all exponents to give better user feedback. 
      static_assert(std::is_same<EnergyExponent, std::ratio<0>>::value, "can not convert quantity with dimensions to number.");
      static_assert(std::is_same<LengthExponent, std::ratio<0>>::value, "can not convert quantity with dimensions to number.");
      static_assert(std::is_same<TimeExponent,   std::ratio<0>>::value, "can not convert quantity with dimensions to number.");
      return val; // this line should 
    }
  explicit physical_quantity(double d)
    : val(d)
  {}
  inline double get_val() const
  {
    return val;
  }
  std::string get_unit() const
  {
    std::string res;
    res+=mk_unit<EnergyExponent>("MeV");
    res+=mk_unit<LengthExponent>("mm");
    res+=mk_unit<TimeExponent  >("ns");
    return res;
  }

};

namespace std
{
  template<class... U>
  ostream& operator<<(ostream& lhs, const physical_quantity<U...> rhs)
  {
    return lhs << rhs.get_val() << rhs.get_unit();
  }
};

template<>
class physical_quantity<std::ratio<0>, std::ratio<0>, std::ratio<0>>
{
  double val;
public:
  using unitpack=pack<std::ratio<0>, std::ratio<0>, std::ratio<0>>;
  inline operator double() const
  {
    return val;
  }
  inline double get_val() const
  {
    return val;
  }
  std::string get_unit() const
  {
    return "";
  }
  physical_quantity(double d)
    : val(d)
  {}
};

template<template <typename, typename> typename op, class U, class V>
using unit_combine = typename apply_op<op, U, V>
  ::res::instantiate<physical_quantity>;

using dimensionless_quantity=physical_quantity<std::ratio<0>, std::ratio<0>, std::ratio<0>>;
using energy_quantity=physical_quantity<std::ratio<1>, std::ratio<0>, std::ratio<0>>;
using length_quantity=physical_quantity<std::ratio<0>, std::ratio<1>, std::ratio<0>>;
using time_quantity  =physical_quantity<std::ratio<0>, std::ratio<0>, std::ratio<1>>;

template<typename... U>
inline auto operator*(double p,
		      physical_quantity<U...> q)
{
  return physical_quantity<U...>
    {p * q.get_val()};
}

template<typename... V>
inline auto operator*(physical_quantity<V...> p,
		      double q)
{
  return physical_quantity<V...>
    {p.get_val() * q};
}

template<typename... U, typename... V>
inline auto operator+(physical_quantity<U...> p,
		      physical_quantity<V...> q)
{
  static_assert(std::is_same<physical_quantity<U...>, physical_quantity<V...>>::value, "Can not subtract quantities of different dimensionality!");
  return physical_quantity<U...> {p.get_val()+q.get_val()};
}

template<typename... U, typename... V>
inline auto operator-(physical_quantity<U...> p,
		      physical_quantity<V...> q)
{
  static_assert(std::is_same<physical_quantity<U...>, physical_quantity<V...>>::value, "Can not subtract quantities of different dimensionality!");
  return physical_quantity<U...> {p.get_val()-q.get_val()};
}



template<typename... U, typename... V>
inline auto operator*(physical_quantity<U...> p,
		      physical_quantity<V...> q)
{
  return unit_combine<std::ratio_add, pack<U...>, pack<V...>>
    {p.get_val() * q.get_val()};
}

template<typename... U, typename... V>
inline auto operator/(physical_quantity<U...> p,
		      physical_quantity<V...> q)
{
  return unit_combine<std::ratio_subtract, pack<U...>, pack<V...>>
    {p.get_val() / q.get_val()};
}


// interface usable for all values of SOU_NO_DIM_ANALYSIS
template<typename... U>
inline double get_val(const physical_quantity<U...> p)
{
  return p.get_val();
}

#else // SO_NO_DIM_ANALYSIS is defined
using energy_quantity=double;
using length_quantity=double;
using time_quantity=double;
inline double get_val(double x)
{
  return x;
}
#endif

const auto GeV=energy_quantity{1.0e3};
const auto MeV=energy_quantity{1.0e0};
const auto keV=energy_quantity{1.0e-3};
const auto  eV=energy_quantity{1.0e-6};

const auto km=length_quantity{1.0e6};
const auto  m=length_quantity{1.0e3};
const auto cm=length_quantity{1.0e1};
const auto mm=length_quantity{1.0e0};

const auto  s=time_quantity{1.0e9};
const auto ms=time_quantity{1.0e6};
const auto us=time_quantity{1.0e3};
const auto ns=time_quantity{1.0e0};
const auto ps=time_quantity{1.0e-3};


