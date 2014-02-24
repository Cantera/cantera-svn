function setValveCoeff(f, k)
% Set the valve coefficient, :math:`K`. The mass flow rate [kg/s] is 
% computed from the expression
%
% .. math:: \dot{m} = K(P_{upstream} - P_{downstream})
%
% as long as this produces a positive value.  If this expression is
% negative, zero is returned.
%
% See also: :mat:func:`Valve`
%
% :param f:
%     Instance of class :mat:func:`Valve`
% :param k:
%     Value of the valve coefficient
%

if f.type == 3
    end
    ok = flowdevicemethods(4, f.index, k);
    if ok < 0
        error(geterr);
    end
else
    error('Valve coefficient can only be set for valves')
end
