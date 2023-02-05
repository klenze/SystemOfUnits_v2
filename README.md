

# SystemOfUnits_v2
A proof of concept on catching dimension errors using the type system to improve on CLHEP's SystemOfUnits.h

## Present state: SystemOfUnits.h

The [CLHEP](https://gitlab.cern.ch/CLHEP/CLHEP) [system of units header](https://gitlab.cern.ch/CLHEP/CLHEP/-/blob/develop/Units/Units/SystemOfUnits.h) originally from [GEANT4](https://geant4.web.cern.ch/) defines a set of units to store physical quantities. 

It does that by defining the base unit as a const double value 1.0:
```
static const HepDouble megaelectronvolt = 1. ;
```
Other SI prefixes are then simply defined relative to that:
```
static const HepDouble     electronvolt = 1.e-6*megaelectronvolt;
static const HepDouble kiloelectronvolt = 1.e-3*megaelectronvolt;
```

This means that -- as long as you agree to use this system of units everywhere -- you can use these si units to store and pass quantities:
```
    obj.SetThreshold(100*kiloelectronvolt); // aka keV, gets converted to a double value of 0.1 (e.g. MeV)
    std::out << "The length is " obj.GetLength()/meter <<"m.\n"; // converts the internal unit (mm) to m. 
```

Note how verbose this is. 

## Failures not caught by SystemOfUnits.h

Units are implemented as const double conversion factors. This means that conversions will never fail, as double multiplications and divisions never fail (unless one activates floating point exceptions). 
This means that the expression
```
double res=std::pow(10*MeV, 20*ns)+log(2*parsec);
```
will be just as valid as any other calculation involving doubles. 

Furthermore, there is no way to automatically check if a some method takes or returns a double in the system of units or perhaps in other units (e.g. the code has hardcoded assumptions of having ROOTs units (cm, GeV).

## Proposed improvement
Using C++ classes, we can track the dimensionality of quantities at compile time. 

As quantities with units such as ``sqrt(MeV)`` can appear, I based used [std::ratio](https://en.cppreference.com/w/cpp/numeric/ratio/ratio) to store the exponents in classes. 

```
template<typename EnergyExponent, typename LengthExponent, typename TimeExponent> // and so on
class physical_quantity
{
  double val;
  ...
};
...
using dimensionless_quantity=physical_quantity<std::ratio<0>, std::ratio<0>, std::ratio<0>>;
using energy_quantity=physical_quantity<std::ratio<1>, std::ratio<0>, std::ratio<0>>;
...
const auto MeV=energy_quantity{1.0e0};
```
A quantity is implicitly convertable to double if and only if it is dimensionless_quantity. 
Otherwise, it's value has to be requested in a specific unit. In a pinch, get_val can be used to explicitly access the value. 
```
  auto en=10.0*MeV;
  double d=en/keV;
  double x=get_val(en); // returns internal representation (MeV)
```
``operator+`` and ``operator-`` use static_assert to make sure that both arguments are quantities of equal dimension. 
``operator*`` and ``operator/`` will figure out the unit of the result. 

## Fundamental limitations

This approach only works if the dimensionality of the quantities stored in the (wrapped) doubles can be determined at compile time and stays constant during the lifetime of the variable. 

As a counterexample, consider the following code which is supposed to calculate the geometric mean of some energies:
```
template<class T>
T geometric_mean(const std::vector<T>& args)
{
    T res{1};
    for(auto & en: args)
       res*=en;
    return std::pow(res, 1/args.size());
}
```
Here, the dimensionality of res changes during its lifetime, so the C++ type system can not be used to track it. 
Explicit conversion to double and converting the result back to T would probably be the preferred way to get this to compile. 

I should stress that in my experience, the above is a rare case in physics calculations.

## Future work

The present state is "proof of concept". 

The following should probably be addressed before any thought can be given to put this system to production use:
* The definition of physical_quantity should be changed. Instead of taking N template parameters, where N is the number of orthogonal units, it should take a variable amount of parameters tagged with the unit. 
```using velocity_quantity = physical_quantity< unit<Length, std::ratio<1>>, unit<Time, std::ratio<-1>> >;``` 
or something, perhaps with Length and Time being enum values. 
This will make calculating a*b and a/b harder, but not impossible. Don't expect super fast compile times, though. 

* Some more operators (assignment, +=, etc) should be implemented. 
* Some functions which can be applied to units (such as std::sqrt) should be overloaded.
* More troublesome: std::pow(q, x) for non-integer x. Options include replacing it with a function ``template<int a, int b=1> pow(quantity q)`` which calculates $$q^\frac{a}{b}$$. Perhaps I'll through in a preprocessor macro as syntactic sugar.  
* Performance tests or asm analysis: does the inlining work everywhere, or is additional code compared to ``-DSO_NO_DIM_ANALYSIS``?
* Support for references.

## Is it all worth it?

* It depends on the project. If your project is very explicit about spelling out what the units are and already uses the CLHEP units everywhere, it might be that jumping through the hoops to make it compilable with unit dimensionality checking is just not worth it. 
* If, on the other hand, your project already uses multiple incompatible systems of units at different places and contains methods like ``SetX(double)``, where the units of X are not immediately obvious, you might benefit from being able to specify (and enforce) ``SetX(length_quantity). (Assuming this code ever goes beyond proof of concept.)
