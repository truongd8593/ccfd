/** \file
 *
 * \brief Contains the functions for initializing and evaluating the source term
 *
 * \author hhh
 * \date Wed 01 Apr 2020 12:25:11 PM CEST
 */

#include <math.h>

#include "main.h"
#include "equation.h"
#include "mesh.h"

/**
 * \brief Evaluate the source term
 * \param[in] iSource The source control integer
 * \param[in] x The coordinates at which to evaluate the source term
 * \param[in] time The computation time at which the evaluate the source term
 * \param[out] source The source term contribution
 */
void evalSource(int iSource, double x[NDIM], double time, double source[NVAR])
{
	switch (iSource) {
	case 1: {
		double freq = 1.0;
		double amp = 0.1;
		double om = pi * freq;
		double a = 2.0 * pi;

		double tmp1 = cos(om * (x[X] + x[Y]) - a * time);
		double tmp2 = sin(2.0 * (om * (x[X] + x[Y]) - a * time));

		#ifdef navierstokes
		double tmp3 = sin(om * (x[X] + x[Y]) - a * time);
		#endif

		source[RHO] = (- a + 2.0 * om) * tmp1;
		source[VX]  = (- a + om * (gam * 3.0 - 1.0)) * tmp1
			+ amp * om * gam1 * tmp2;
		source[VY]  = source[VX];

		#ifdef navierstokes
		source[E]   = ((2.0 + gam * 6.0) * om - 4.0 * a) * tmp1
			+ amp * (2.0 * om * gam - a) * tmp2
			+ 2.0 * mu * gam * om * om / Pr * tmp3;
		#else
		source[E]   = ((2.0 + gam * 6.0) * om - 4.0 * a) * tmp1
			+ amp * (2.0 * om * gam - a) * tmp2;
		#endif

		source[RHO] *= amp;
		source[VX]  *= amp;
		source[VY]  *= amp;
		source[E]   *= amp;

		break;
	}
	}
}

/**
 * \brief Calculate the contribution of the source terms
 * \param[in] time The computation time at which the evaluate the source term
 */
void calcSource(double time)
{
	#pragma omp parallel for
	for (long iElem = 0; iElem < nElems; ++iElem) {
		elem_t *aElem = elem[iElem];
		aElem->source[RHO] = 0.0;
		aElem->source[VX]  = 0.0;
		aElem->source[VY]  = 0.0;
		aElem->source[E]   = 0.0;

		double source[NVAR] = {0.0};
		for (int iGP = 0; iGP < aElem->nGP; ++iGP) {
			evalSource(sourceFunc, aElem->xGP[iGP], time, source);
			aElem->source[RHO] += source[RHO] * aElem->wGP[iGP];
			aElem->source[VX]  += source[VX]  * aElem->wGP[iGP];
			aElem->source[VY]  += source[VY]  * aElem->wGP[iGP];
			aElem->source[E]   += source[E]   * aElem->wGP[iGP];
		}
	}
}
