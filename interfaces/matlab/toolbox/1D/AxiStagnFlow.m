function m = AxiStagnFlow(gas)
% Axisymmetric stagnation flow.
%
% :param gas:
%     Instance of class :mat:func:`Solution`
% :return:
%     Domain1D instance representing an axisymmetric
%     stagnation flow.

m = Domain1D(1, gas);
