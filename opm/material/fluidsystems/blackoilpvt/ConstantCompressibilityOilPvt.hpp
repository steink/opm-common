// -*- mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
// vi: set et ts=4 sw=4 sts=4:
/*
  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.

  Consult the COPYING file in the top-level source directory of this
  module for the precise wording of the license and the list of
  copyright holders.
*/
/*!
 * \file
 * \copydoc Opm::ConstantCompressibilityOilPvt
 */
#ifndef OPM_CONSTANT_COMPRESSIBILITY_OIL_PVT_HPP
#define OPM_CONSTANT_COMPRESSIBILITY_OIL_PVT_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace Opm {

#if HAVE_ECL_INPUT
class EclipseState;
class Schedule;
#endif

/*!
 * \brief This class represents the Pressure-Volume-Temperature relations of the oil phase
 *        without dissolved gas and constant compressibility/"viscosibility".
 */
template <class Scalar>
class ConstantCompressibilityOilPvt
{
public:
#if HAVE_ECL_INPUT
    /*!
     * \brief Sets the pressure-dependent oil viscosity and density
     *        using the Eclipse PVCDO keyword.
     */
    /*!
     * \brief Initialize the oil parameters via the data specified by the PVTO ECL keyword.
     */
    void initFromState(const EclipseState& eclState, const Schedule&);
#endif

    void setNumRegions(std::size_t numRegions);

    void setVapPars(const Scalar, const Scalar)
    {
    }

    /*!
     * \brief Initialize the reference densities of all fluids for a given PVT region
     */
    void setReferenceDensities(unsigned regionIdx,
                               Scalar rhoRefOil,
                               Scalar /*rhoRefGas*/,
                               Scalar /*rhoRefWater*/)
    { oilReferenceDensity_[regionIdx] = rhoRefOil; }

    /*!
     * \brief Set the viscosity and "viscosibility" of the oil phase.
     */
    void setViscosity(unsigned regionIdx, Scalar muo, Scalar oilViscosibility = 0.0)
    {
        oilViscosity_[regionIdx] = muo;
        oilViscosibility_[regionIdx] = oilViscosibility;
    }

    /*!
     * \brief Set the compressibility of the oil phase.
     */
    void setCompressibility(unsigned regionIdx, Scalar oilCompressibility)
    { oilCompressibility_[regionIdx] = oilCompressibility; }

    /*!
     * \brief Set the oil reference pressure [Pa]
     */
    void setReferencePressure(unsigned regionIdx, Scalar p)
    { oilReferencePressure_[regionIdx] = p; }

    /*!
     * \brief Set the oil reference formation volume factor [-]
     */
    void setReferenceFormationVolumeFactor(unsigned regionIdx, Scalar BoRef)
    { oilReferenceFormationVolumeFactor_[regionIdx] = BoRef; }

    /*!
     * \brief Set the oil "viscosibility" [1/ (Pa s)]
     */
    void setViscosibility(unsigned regionIdx, Scalar muComp)
    { oilViscosibility_[regionIdx] = muComp; }

    /*!
     * \brief Finish initializing the oil phase PVT properties.
     */
    void initEnd()
    { }

    /*!
     * \brief Return the number of PVT regions which are considered by this PVT-object.
     */
    unsigned numRegions() const
    { return oilViscosity_.size(); }

    /*!
     * \brief Returns the specific enthalpy [J/kg] of oil given a set of parameters.
     */
    template <class Evaluation>
    Evaluation internalEnergy(unsigned,
                        const Evaluation&,
                        const Evaluation&,
                        const Evaluation&) const
    {
        throw std::runtime_error("Requested the enthalpy of oil but the thermal "
                                 "option is not enabled");
    }
    Scalar hVap(unsigned) const
    {
        throw std::runtime_error("Requested the hvap of oil but the thermal "
                                 "option is not enabled");
    }
    /*!
     * \brief Returns the dynamic viscosity [Pa s] of gas saturated oil given a pressure
     *        and a phase composition.
     */
    template <class Evaluation>
    Evaluation viscosity(unsigned regionIdx,
                         const Evaluation& temperature,
                         const Evaluation& pressure,
                         const Evaluation& /*Rs*/) const
    { return saturatedViscosity(regionIdx, temperature, pressure); }

    /*!
     * \brief Returns the dynamic viscosity [Pa s] of gas saturated oil given a pressure.
     */
    template <class Evaluation>
    Evaluation saturatedViscosity(unsigned regionIdx,
                                  const Evaluation& temperature,
                                  const Evaluation& pressure) const
    {
        Scalar BoMuoRef = oilViscosity_[regionIdx]*oilReferenceFormationVolumeFactor_[regionIdx];
        const Evaluation& bo = saturatedInverseFormationVolumeFactor(regionIdx, temperature, pressure);

        Scalar pRef = oilReferencePressure_[regionIdx];
        const Evaluation& Y =
            (oilCompressibility_[regionIdx] - oilViscosibility_[regionIdx])
            * (pressure - pRef);
        return BoMuoRef * bo / (1.0 + Y * (1.0 + Y / 2.0));
    }

    /*!
     * \brief Returns the formation volume factor [-] of the fluid phase.
     */
    template <class Evaluation>
    Evaluation inverseFormationVolumeFactor(unsigned regionIdx,
                                            const Evaluation& temperature,
                                            const Evaluation& pressure,
                                            const Evaluation& /*Rs*/) const
    { return saturatedInverseFormationVolumeFactor(regionIdx, temperature, pressure); }

    /*!
     * \brief Returns the formation volume factor [-] of gas saturated oil.
     *
     * Note that constant compressibility oil is a special case of dead oil and dead oil
     * is always gas saturated by by definition.
     */
    template <class Evaluation>
    Evaluation saturatedInverseFormationVolumeFactor(unsigned regionIdx,
                                                     const Evaluation& /*temperature*/,
                                                     const Evaluation& pressure) const
    {
        // cf. ECLiPSE 2011 technical description, p. 116
        Scalar pRef = oilReferencePressure_[regionIdx];
        const Evaluation& X = oilCompressibility_[regionIdx]*(pressure - pRef);

        Scalar BoRef = oilReferenceFormationVolumeFactor_[regionIdx];
        return (1 + X*(1 + X/2))/BoRef;
    }

    /*!
     * \brief Returns the gas dissolution factor \f$R_s\f$ [m^3/m^3] of the oil phase.
     */
    template <class Evaluation>
    Evaluation saturatedGasDissolutionFactor(unsigned /*regionIdx*/,
                                             const Evaluation& /*temperature*/,
                                             const Evaluation& /*pressure*/) const
    { return 0.0; /* this is dead oil! */ }

    /*!
     * \brief Returns the gas dissolution factor \f$R_s\f$ [m^3/m^3] of the oil phase.
     */
    template <class Evaluation>
    Evaluation saturatedGasDissolutionFactor(unsigned /*regionIdx*/,
                                             const Evaluation& /*temperature*/,
                                             const Evaluation& /*pressure*/,
                                             const Evaluation& /*oilSaturation*/,
                                             const Evaluation& /*maxOilSaturation*/) const
    { return 0.0; /* this is dead oil! */ }

    /*!
     * \brief Returns the saturation pressure of the oil phase [Pa]
     *        depending on its mass fraction of the gas component
     *
     * \param Rs The surface volume of gas component dissolved in what
     *           will yield one cubic meter of oil at the surface [-]
     */
    template <class Evaluation>
    Evaluation saturationPressure(unsigned /*regionIdx*/,
                                  const Evaluation& /*temperature*/,
                                  const Evaluation& /*Rs*/) const
    { return 0.0; /* this is dead oil, so there isn't any meaningful saturation pressure! */ }

    template <class Evaluation>
    Evaluation diffusionCoefficient(const Evaluation& /*temperature*/,
                                    const Evaluation& /*pressure*/,
                                    unsigned /*compIdx*/) const
    {
        throw std::runtime_error("Not implemented: The PVT model does not "
                                 "provide a diffusionCoefficient()");
    }

    Scalar oilReferenceDensity(unsigned regionIdx) const
    { return oilReferenceDensity_[regionIdx]; }

    const std::vector<Scalar>& oilReferenceFormationVolumeFactor() const
    { return oilReferenceFormationVolumeFactor_; }

    const std::vector<Scalar>& oilCompressibility() const
    { return oilCompressibility_; }

    const std::vector<Scalar>& oilViscosity() const
    { return oilViscosity_; }

    const std::vector<Scalar>& oilViscosibility() const
    { return oilViscosibility_; }

private:
    std::vector<Scalar> oilReferenceDensity_{};
    std::vector<Scalar> oilReferencePressure_{};
    std::vector<Scalar> oilReferenceFormationVolumeFactor_{};
    std::vector<Scalar> oilCompressibility_{};
    std::vector<Scalar> oilViscosity_{};
    std::vector<Scalar> oilViscosibility_{};
};

} // namespace Opm

#endif
