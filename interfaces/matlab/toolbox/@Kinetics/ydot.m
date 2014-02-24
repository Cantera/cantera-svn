function v = ydot(a)
% Evaluates the source term :math:`\dot{\omega}_k M_k /\rho`
%
% :param a:
%     Instance of class :mat:func:`Kinetics` (or another
%     object containing an instance of that class)
%     for which the ydots are desired.
% :return:
%     Returns a vector of length nSpecies
%

v = kinetics_get(a.id,24,0);
