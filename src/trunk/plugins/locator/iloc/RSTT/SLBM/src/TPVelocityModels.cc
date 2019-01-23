//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract 
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains 
//- certain rights in this software.
//-
//- BSD Open Source License.
//- All rights reserved.
//- 
//- Redistribution and use in source and binary forms, with or without 
//- modification, are permitted provided that the following conditions are met:
//-
//-    * Redistributions of source code must retain the above copyright notice, 
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright 
//-      notice, this list of conditions and the following disclaimer in the 
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its 
//-      contributors may be used to endorse or promote products derived from  
//-      this software without specific prior written permission.
//-
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
//- POSSIBILITY OF SUCH DAMAGE.
//-

//- ****************************************************************************
//-
//- Program:       SNL TauP Location Library (TauPLoc)
//- Module:        $RCSfile: TPVelocityModels.cc,v $
//- Creator:       Jim Hipp
//- Creation Date: April 17, 2007
//- Revision:      $Revision: 1.6 $
//- Last Modified: $Date: 2011/10/07 13:15:00 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************

// **** _SYSTEM INCLUDES_ ******************************************************

#include <cmath>
#include <iomanip>

// **** _LOCAL INCLUDES_ *******************************************************

#include "TPVelocityModels.h"
#include "Brents.h"
#include "Brents.cc"
#include "IntegrateFunction.h"
#include "IntegrateFunction.cc"

// **** _BEGIN TAUP NAMESPACE_ *************************************************

template class util::Brents<taup::VZero<taup::VelocityCubic> >;
template class util::IntegrateFunction<taup::TPdDistdr<taup::VelocityConst> >;
template class util::IntegrateFunction<taup::TPdTaudr<taup::VelocityConst> >;
template class util::IntegrateFunction<taup::TPdDistdr<taup::VelocityPower> >;
template class util::IntegrateFunction<taup::TPdTaudr<taup::VelocityPower> >;
template class util::IntegrateFunction<taup::TPdDistdr<taup::VelocityLinear> >;
template class util::IntegrateFunction<taup::TPdTaudr<taup::VelocityLinear> >;
template class util::IntegrateFunction<taup::TPdDistdr<taup::VelocityQuadratic> >;
template class util::IntegrateFunction<taup::TPdTaudr<taup::VelocityQuadratic> >;
template class util::IntegrateFunction<taup::TPdDistdr<taup::VelocityCubic> >;
template class util::IntegrateFunction<taup::TPdTaudr<taup::VelocityCubic> >;

namespace taup {

// **** _STATIC INITIALIZATIONS_************************************************

template class VZero<VelocityCubic>;
template class TPdDistdr<VelocityConst>;
template class TPdTaudr<VelocityConst>;
template class TPdDistdr<VelocityPower>;
template class TPdTaudr<VelocityPower>;
template class TPdDistdr<VelocityLinear>;
template class TPdTaudr<VelocityLinear>;
template class TPdDistdr<VelocityQuadratic>;
template class TPdTaudr<VelocityQuadratic>;
template class TPdDistdr<VelocityCubic>;
template class TPdTaudr<VelocityCubic>;

//template class util::Brents<VZero<VelocityCubic> >;
//template class util::IntegrateFunction<TPdDistdr<VelocityConst> >;
//template class util::IntegrateFunction<TPdTaudr<VelocityConst> >;
//template class util::IntegrateFunction<TPdDistdr<VelocityPower> >;
//template class util::IntegrateFunction<TPdTaudr<VelocityPower> >;
//template class util::IntegrateFunction<TPdDistdr<VelocityLinear> >;
//template class util::IntegrateFunction<TPdTaudr<VelocityLinear> >;
//template class util::IntegrateFunction<TPdDistdr<VelocityQuadratic> >;
//template class util::IntegrateFunction<TPdTaudr<VelocityQuadratic> >;
//template class util::IntegrateFunction<TPdDistdr<VelocityCubic> >;
//template class util::IntegrateFunction<TPdTaudr<VelocityCubic> >;

template<class V>
const double   VelocityIntegrate<V>::vmIntegTol = 1.0e-6;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief TPVelocityLayer factory. Returns a new TPVelocityLayer object copied
//! from the input object \em tpvl
//
// *****************************************************************************
TPVelocityLayer* TPVelocityLayer::newModelCopy(TPVelocityLayer* tpvl)
{
  if (tpvl->isVelocityConstant())
    return new VelocityConst(*dynamic_cast<VelocityConst*>(tpvl));
  else if (tpvl->isVelocityPowerLaw())
    return new VelocityPower(*dynamic_cast<VelocityPower*>(tpvl));
  else if (tpvl->isVelocityLinear())
    return new VelocityLinear(*dynamic_cast<VelocityLinear*>(tpvl));
  else if (tpvl->isVelocityQuadratic())
    return new VelocityQuadratic(*dynamic_cast<VelocityQuadratic*>(tpvl));
  else if (tpvl->isVelocityCubic())
    return new VelocityCubic(*dynamic_cast<VelocityCubic*>(tpvl));
  else
    return (TPVelocityLayer*) NULL;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief TPVelocityLayer factory. Returns a new TPVelocityLayer object
//! of type \em cnam whose data is contained in the input DataBuffer
//! \em buffer.
//
// *****************************************************************************
TPVelocityLayer* TPVelocityLayer::newModelCopy(const string& cnam,
                                               DataBuffer& buffer)
{
  if (cnam == "VelocityConst")
    return new VelocityConst(buffer);
  else if (cnam == "VelocityPower")
    return new VelocityPower(buffer);
  else if (cnam == "VelocityLinear")
    return new VelocityLinear(buffer);
  else if (cnam == "VelocityQuadratic")
    return new VelocityQuadratic(buffer);
  else if (cnam == "VelocityCubic")
    return new VelocityCubic(buffer);
  else
    return (TPVelocityLayer*) NULL;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to string.
//
// *****************************************************************************
string TPVelocityLayer::toString() const
{
  ostringstream os;
  toStream(os, "  ");
  return os.str();
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void TPVelocityLayer::toStream(ostream& os, string indent) const
{
  int mns;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(TPVelocityLayer) << endl << endl;

  // print out definition data

  os << indent << "Tau-P Layer Velocity Model" << endl << endl;

  os << indent << "  Layer Name                   = "
     << vlLayerName << endl;
  if (vlPhase != "")
    os << indent << "  Phase Name                   = "
       << vlPhase << endl;
  if (vlPhaseUpper != "")
    os << indent << "  Upper Branch Phase Name      = "
       << vlPhaseUpper << endl;
  if (vlPhaseLower != "")
    os << indent << "  Lower Branch Phase Name      = "
       << vlPhaseLower << endl;
  if (vlPhaseIUpper != "")
    os << indent << "  Top Layer Interface Phase Name = "
       << vlPhaseIUpper << endl;
  if (vlPhaseILower != "")
    os << indent << "  Bot Layer Interface Phase Name = "
       << vlPhaseILower << endl;

  os << endl << endl;

  //              "  Layer Data             Top        Bottom"
  //              "  Name (unit)      xxxxxxxxxxxx  xxxxxxxxxxxx"
  os << indent << "  Layer Data             Top        Bottom"
     << endl << endl;
  os << indent << "  Radius (km)      "
     << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right << vlRt
     << std::setprecision(4) << std::setw(12) << vlRb << endl;
  os << indent << "  Velocity (km/s)  "
     << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right << vlVt
     << std::setprecision(4) << std::setw(12) << vlVb << endl;
  os << indent << "  Slowness (s/deg) "
     << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right << DEG_TO_RAD * vlPt
     << std::setprecision(4) << std::setw(12) << DEG_TO_RAD * vlPb << endl;
  os << indent << "  Distance (deg)   "
     << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right << 2.0 * RAD_TO_DEG * vlDistT
     << std::setprecision(4) << std::setw(12) << 2.0 * RAD_TO_DEG * vlDistB
     << endl;
  os << indent << "  dDistdP (deg^2/s)"
     << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right
     << RAD_TO_DEG * RAD_TO_DEG * vldDistdP_T
     << std::setprecision(4) << std::setw(12)
     << RAD_TO_DEG * RAD_TO_DEG * vldDistdP_B << endl << endl;

  os << indent << "  Layer Type: ";
  if (vlLayerType == 0)
    os << "Turning" << endl;
  else if (vlLayerType == 1)
    os << "Bottom Turning (" << DEG_TO_RAD * vlPmin << "->"
       <<  DEG_TO_RAD * vlPb << ")" << endl;
  else if (vlLayerType == 2)
    os << "Top Turning (" << DEG_TO_RAD * vlPt << "->"
       <<  DEG_TO_RAD * vlPmin << ")" << endl;
  else if (vlLayerType == 3)
    os << "Shadow" << endl;

  os << endl;

  if (vlSplitLayer)
  {
    os << indent << "  Retrograde Layer:" << endl
       << indent << "    " << "Critical Slowness(s/`deg) / Distance(km) = "
       << DEG_TO_RAD * vlPCrit << " / "
       << 2.0 * RAD_TO_DEG * vlDistCrit << endl << endl;
  }
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void VelocityConst::toStream(ostream& os, string indent) const
{
  int mns;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(VelocityConst) << endl;

  // print out definition data

  os << indent << "Velocity Definition          = ";
  writeVelocity(os);
  os << endl;

  // Print parent control object information

  TPVelocityLayer::toStream(os, indent); 
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void VelocityPower::toStream(ostream& os, string indent) const
{
  int mns;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(VelocityPower) << endl;

  // print out definition data

  os << indent << "Velocity Definition          = ";
  writeVelocity(os);
  os << endl;

  // Print parent control object information

  TPVelocityLayer::toStream(os, indent); 
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void VelocityLinear::toStream(ostream& os, string indent) const
{
  int mns;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(VelocityLinear) << endl;

  // print out definition data

  os << indent << "Velocity Definition          = ";
  writeVelocity(os);
  os << endl;

  // Print parent control object information

  TPVelocityLayer::toStream(os, indent); 
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void VelocityQuadratic::toStream(ostream& os, string indent) const
{
  int mns;

  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(VelocityQuadratic) << endl;

  // print out definition data

  os << indent << "Velocity Definition          = ";
  writeVelocity(os);
  os << endl;

  // Print parent control object information

  TPVelocityLayer::toStream(os, indent); 
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Print object data to input stream \em os.
//
// *****************************************************************************
void VelocityCubic::toStream(ostream& os, string indent) const
{
  int mns;


  // print class name and increment indent

  mns = indent.size() + class_name().size() + 10;
  os << indent << class_name() << " (" << this << ") "
     << string(79 - mns, '*') << endl << endl;
  indent.append(2, ' ');
  
  // print out run-time data

  os << indent << "Object Size (bytes)          = "
     << sizeof(VelocityCubic) << endl;

  // print out definition data

  os << indent << "Velocity Definition          = ";
  writeVelocity(os);
  os << endl;

  // Print parent control object information

  TPVelocityLayer::toStream(os, indent); 
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! Standard function that returns the radius as function of the turning
//! ray parameter \em p. The turning ray parameter is defined as
//! \em p = r / v(r) where v(r) is the radial velocity. For Cubic velocity
//! the result is found numerically by zeroing in using the function object
//! VZero templated on the cubic velocity.
//
// *****************************************************************************
double VelocityCubic::rAtP(double p)
{
  VZero<VelocityCubic> vz(p, *this);
  util::Brents<VZero<VelocityCubic> > zb(vz, 1.0e-8);

  return zb.zeroF(vlRt, vlRb);
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the velocity definition to the input stream \em os.
//
// *****************************************************************************
void VelocityConst::writeVelocity(ostream& os) const
{
  os << std::fixed << std::showpoint
     << std::setprecision(4) << std::setw(12) << std::right
     << vc << std::setprecision(4) << std::setw(12) << std::right
     << "Constant" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the velocity definition to the input stream \em os.
//
// *****************************************************************************
void VelocityPower::writeVelocity(ostream& os) const
{
  os << std::fixed << std::showpoint
     << std::setprecision(4) << std::setw(12) << std::right
     << vp0 << std::setprecision(4) << std::setw(12) << std::right
     << vp1 << std::setprecision(4) << std::setw(12) << std::right
     << "Power" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the normalized radius definition to input stream \em os.
//
// *****************************************************************************
void VelocityLinear::writeNormRadius(ostream& os) const
{
  if (vNormRadius == 1.0)
    os << "    NormalizedRadius = False" << endl;
  else
    os << "    NormalizedRadius = True" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the velocity definition to the input stream \em os.
//
// *****************************************************************************
void VelocityLinear::writeVelocity(ostream& os) const
{
  os << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right
     << va0 << std::setprecision(4) << std::setw(12) << std::right
     << va1 << std::setprecision(4) << std::setw(12) << std::right
     << "Linear" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the normalized radius definition to input stream \em os.
//
// *****************************************************************************
void VelocityQuadratic::writeNormRadius(ostream& os) const
{
  if (vNormRadius == 1.0)
    os << "    NormalizedRadius = False" << endl;
  else
    os << "    NormalizedRadius = True" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the velocity definition to the input stream \em os.
//
// *****************************************************************************
void VelocityQuadratic::writeVelocity(ostream& os) const
{
  os << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right
     << va0 << std::setprecision(4) << std::setw(12) << std::right
     << va1 << std::setprecision(4) << std::setw(12) << std::right
     << va2 << std::setprecision(4) << std::setw(12) << std::right
     << "Quadratic" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the normalized radius definition to input stream \em os.
//
// *****************************************************************************
void VelocityCubic::writeNormRadius(ostream& os) const
{
  if (vNormRadius == 1.0)
    os << "    NormalizedRadius = False" << endl;
  else
    os << "    NormalizedRadius = True" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Writes the velocity definition to the input stream \em os.
//
// *****************************************************************************
void VelocityCubic::writeVelocity(ostream& os) const
{
  os << std::setprecision(4) << std::fixed << std::showpoint
     << std::setw(12) << std::right
     << va0 << std::setprecision(4) << std::setw(12) << std::right
     << va1 << std::setprecision(4) << std::setw(12) << std::right
     << va2 << std::setprecision(4) << std::setw(12) << std::right
     << va3 << std::setprecision(4) << std::setw(12) << std::right
     << "Cubic" << endl;
}

// **** _FUNCTION DESCRIPTION_ *************************************************
//
//! \brief Used to create the distance and tau integrand functions vmDist,
//! and vmTau, and their associated integration objects vmDistNI and
//! vmTauNI.
//
// *****************************************************************************
template<class V>
void VelocityIntegrate<V>::createNumericObjects(V& v)
{
  vmDist   = new TPdDistdr<V>(v);
  vmDistNI = new util::IntegrateFunction<TPdDistdr<V> >(*vmDist, vmIntegTol);
  vmTau    = new TPdTaudr<V>(v);
  vmTauNI  = new util::IntegrateFunction<TPdTaudr<V> >(*vmTau, vmIntegTol);
}

} // end namespace taup
