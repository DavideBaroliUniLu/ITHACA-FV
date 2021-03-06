/*---------------------------------------------------------------------------*\
     ██╗████████╗██╗  ██╗ █████╗  ██████╗ █████╗       ███████╗██╗   ██╗
     ██║╚══██╔══╝██║  ██║██╔══██╗██╔════╝██╔══██╗      ██╔════╝██║   ██║
     ██║   ██║   ███████║███████║██║     ███████║█████╗█████╗  ██║   ██║
     ██║   ██║   ██╔══██║██╔══██║██║     ██╔══██║╚════╝██╔══╝  ╚██╗ ██╔╝
     ██║   ██║   ██║  ██║██║  ██║╚██████╗██║  ██║      ██║      ╚████╔╝ 
     ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝      ╚═╝       ╚═══╝  
 
 * In real Time Highly Advanced Computational Applications for Finite Volumes 
 * Copyright (C) 2017 by the ITHACA-FV authors
-------------------------------------------------------------------------------

License
    This file is part of ITHACA-FV

    ITHACA-FV is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ITHACA-FV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with ITHACA-FV. If not, see <http://www.gnu.org/licenses/>.

Class
    reducedSteadyNS

Description
    A reduced problem for the stationary NS equations

SourceFiles
    reducedSteadyNS.C

\*---------------------------------------------------------------------------*/

/// \file
/// Header file of the reducedSteadyNS class.
/// \dir
/// Directory containing the header and source files for the reducedSteadyNS class

#ifndef reducedSteadyNS_H
#define reducedSteadyNS_H

#include "fvCFD.H"
#include "IOmanip.H"
#include "reducedProblem.H"
#include "steadyNS.H"
#include "ITHACAutilities.H" 
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
#include <unsupported/Eigen/NumericalDiff>

/// Structure to implement a newton object for a stationary NS problem
struct newton_steadyNS: public newton_argument<double>
{
public:
    newton_steadyNS() {}
    newton_steadyNS(int Nx, int Ny, steadyNS& problem): newton_argument<double>(Nx, Ny),
    Nphi_u(problem.NUmodes + problem.liftfield.size() + problem.NSUPmodes),
    Nphi_p(problem.NPmodes),
    N_BC(problem.inletIndex.rows()),
    B_matrix(problem.B_matrix),
    C_matrix(problem.C_matrix),
    K_matrix(problem.K_matrix),
    P_matrix(problem.P_matrix)
    {}

    int operator()(const Eigen::VectorXd &x, Eigen::VectorXd &fvec) const;
    int df(const Eigen::VectorXd &x,  Eigen::MatrixXd &fjac) const;

    int Nphi_u;
    int Nphi_p;
    int N_BC;
    scalar nu;
    Eigen::MatrixXd B_matrix;
    List <Eigen::MatrixXd> C_matrix; 
    Eigen::MatrixXd K_matrix; 
    Eigen::MatrixXd P_matrix;
    Eigen::VectorXd BC;   
      
};


/*---------------------------------------------------------------------------*\
                        Class reducedProblem Declaration
\*---------------------------------------------------------------------------*/

/// Class where it is implemented a reduced problem for the <b> steady Navier-stokes </b> problem.
/** In this class are implemented the methods to solve a reduced version of the stationary Navier-stokes
equations. This class must be used together with the steadyNS class */
class reducedSteadyNS: public reducedProblem
{
private:

public:
    // Constructors
    /// Construct Null
    reducedSteadyNS();

    /// Construct Null
    ///
    /// @param[in]  problem  a full order steadyNS problem
    /// @param[in]  tipo     Type of pressure stabilisation method you want to use "SUP" for supremizer, "PPE" for pressure Poisson equation.
    /// 
    reducedSteadyNS(steadyNS& problem, word tipo);


    // Specific variable
    /** @name Reduced Matrices
    *
    */
    ///@{

    /// Diffusion Term
    Eigen::MatrixXd B_matrix;

    /// Mass Matrix Term
    Eigen::MatrixXd M_matrix;

    /// Gradient of pressure
    Eigen::MatrixXd K_matrix;

    /// Convective Term
    List <Eigen::MatrixXd> C_matrix;

    /// Divergence of velocity
    Eigen::MatrixXd P_matrix;

    /// Laplacian of pressure
    Eigen::MatrixXd D_matrix;

    /// Divergence of momentum
    List <Eigen::MatrixXd> G_matrix;
    ///@}


    /// Vector to store the previous solution during the Newton procedure
    Eigen::VectorXd y_old;

    /// Vector to store the solution during the Newton procedure
    Eigen::VectorXd y;

    /// Reduced viscosity in case of parametrized viscosity
    scalar nu;

    /// List of Eigen matrices to store the online solution
    List < Eigen::MatrixXd> online_solution;

    /// List of pointers to store the modes for velocity
    PtrList<volVectorField> Umodes;

    /// List of pointers to store the modes for pressure    
    PtrList<volScalarField> Pmodes;

    /// List of pointers to store the snapshots for velocity    
    PtrList<volVectorField> Usnapshots;

    /// List of pointers to store the snapshots for pressure    
    PtrList<volScalarField> Psnapshots;
    
    /// Reconstructed pressure field
    PtrList<volScalarField> PREC;

    /// Recontructed velocity field
    PtrList<volVectorField> UREC;

    /// Newton object used to solve the non linear problem
    newton_steadyNS newton_object;

    /// Number of velocity modes
    int Nphi_u;

    /// Number of pressure modes
    int Nphi_p;

    /// Number of supremizer modes
    int Nphi_sup;

    /// Number of parametrized boundary conditions
    int N_BC;

    /// Counter to count the online solutions
    int count_online_solve = 1;

    // Functions
    
    /// Method to perform an online solve using a PPE stabilisation method
    ///
    /// @param[in]  vel_now  The vector of online velocity. It is defined in 
    /// with an Eigen::MatrixXd and must have one col and as many rows as the number
    /// of parametrized boundary conditions.
    ///
    void solveOnline_PPE(Eigen::MatrixXd vel_now);

    /// Method to perform an online solve using a supremizer stabilisation
    /// method
    ///
    /// @param[in]  vel_now  The vector of online velocity. It is defined in 
    /// with an Eigen::MatrixXd and must have one col and as many rows as the number
    /// of parametrized boundary conditions.
    ///
    void solveOnline_sup(Eigen::MatrixXd vel_now);

    /// Method to reconstruct a solution from an online solve with a PPE stabilisation technique.
    /// stabilisation method
    ///
    /// @param      problem     a steadyNS full order problem.
    /// @param[in]  folder      The folder where you want to store the results (default is "./ITHACAOutput/online_rec").
    /// @param[in]  printevery  Variable to recover only every printevery online solutions default is 1.
    ///
    void reconstruct_PPE(steadyNS& problem, fileName folder = "./ITHACAOutput/online_rec", int printevery = 1);

    /// Method to reconstruct a solution from an online solve with a supremizer stabilisation technique.
    /// stabilisation method
    ///
    /// @param      problem     a steadyNS full order problem.
    /// @param[in]  folder      The folder where you want to store the results (default is "./ITHACAOutput/online_rec").
    /// @param[in]  printevery  Variable to recover only every printevery online solutions default is 1.
    ///
    void reconstruct_sup(steadyNS& problem, fileName folder = "./ITHACAOutput/online_rec", int printevery = 1);

    /// Method to evaluate the online inf-sup constant
    ///
    /// @return     return the reduced version of the inf-sup constant.
    ///
    double inf_sup_constant();
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



#endif






