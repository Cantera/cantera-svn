% Cantera Toolbox.
% Version 2.2a   4-March-2014
%
% Cantera is an open-source, object-oriented software package to aid
% the analysis and simulation of chemically-reacting flows. It
% consists of a computational kernel written largely in C++, along
% with interface libraries for several programming languages,
% including Fortran 90, Python, and MATLAB. More information about
% Cantera is available at http://code.google.com/p/cantera
%
% This toolbox is the Cantera MATLAB interface.
%
%
%   Copyright 2002 California Institute of Technology
%
%   ConstPressureReactor         - Create a constant pressure reactor object.
%   FlowReactor                  - Create a flow reactor object.
%   GRI30                        - Create an object with the GRI-Mech 3.0 reaction mechanism.
%   Hydrogen                     - Return an object representing hydrogen.
%   IdealGasConstPressureReactor - Create a constant pressure reactor with an ideal gas.
%   IdealGasMix                  - Create a mixture of ideal gasses.
%   IdealGasReactor              - Create a reactor with an ideal gas.
%   MassFlowController           - Create a mass flow controller.
%   Methane                      - Return an object representing methane.
%   Nitrogen                     - Return an object representing nitrogen.
%   Oxygen                       - Return an object representing oxygen.
%   Reservoir                    - Create a Reservoir object.
%   Valve                        - Create a valve.
%   Water                        - Return an object representing water.
%   adddir                       - Add a directory to the search path.
%   air                          - Create an object representing air.
%   ck2cti                       - Convert a CHEMKIN input file to Cantera format.
%   cleanup                      - Delete all stored Cantera objects and reclaim memory.
%   conhp                        - Set up ODE system for constant-pressure, adiabatic reactor.
%   constants                    - Get the values of important constants.
%   conuv                        - Set up the ODE system for a constant-volume, adiabatic reactor.
%   gasconstant                  - Get the universal gas constant in J/kmol-K.
%   gaussian                     - Create a Gaussian :mat:func:`Func` instance.
%   geterr                       - Get the error message from a Cantera error.
%   importEdge                   - Import edges between phases.
%   importInterface              - Import an interface between phases.
%   importPhase                  - Import a phase from a CTI file
%   oneatm                       - Get one atmosphere in Pa.
%   polynom                      - Create a polynomial :mat:func:`Func` instance.
%   reactor_ode                  - Set up the ODE sysem for a generic 0-D reactor.
