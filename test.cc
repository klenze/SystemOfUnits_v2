
#define SOU_NO_DIM_ANALYSIS 1
#include "SystemOfUnits_v2.h"
#include <iostream>

int main(void)
{
  auto en=10.0*MeV;
  //double q=x; //error: static assertion failed: can not convert quantity with dimensions to number.
  double d=en/MeV; // ok: is dimensionless
  // print quantities in their base units
  std::cout << "Energy is "<< en << ".\n";
  // or in explicit units (as in CLHEP)
  std::cout << "Energy is "<< en/GeV << "GeV.\n";
  //auto z=en+20*ns; //error: static assertion failed: Can not add quantities of different dimensionality!
}
