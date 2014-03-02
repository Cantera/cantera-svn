function m = FreeFlame(gas, id)
% FREEFLAME - Freely-propagating flat flame
%
% :param gas:
%     Instance of class :mat:func:`Solution`
% :param id:
%     String, ID of the flow
% :return:
%     Domain1D instance representing a freely propagating,
%     adiabatic flame
%

m = Domain1D(1, gas, 2);
if nargin == 1
    setID(m,'flame');
else
    setID(m,id);
end
