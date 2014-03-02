function m = OutletRes(id)
% :return:
%     Instance of :mat:func:`Domain1D` representing an outlet
%     reservoir.
%

m = Domain1D(-2);
if nargin == 0
    setID(m,'outletres');
else
    setID(m,id);
end
